#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>

/* Configuration */
#define EMIT_PERIOD_MS      1000
#define PACKET_ID           0x1001  /* example APID (11 bits typically) */
#define MAX_PAYLOAD_SIZE    32

/* Simple CCSDS-like header (not full spec): 
   Primary header (fixed 6 bytes):
     - [0..1] Packet ID (16 bits)
     - [2..3] Packet Sequence Control (16 bits) -> 14-bit seq, 2-bit flags
     - [4..5] Packet Length (16 bits) -> length of payload-1
*/
static uint16_t seq_counter = 0;
int master_fd;
char *slave_name = NULL;

/* Setup Serial Port for communication */
static void setup_virtual_serial_port(void) {
    printf("Setting up virtual serial port...\n");
    master_fd = posix_openpt( O_RDWR | O_NOCTTY );
    if (master_fd < 0) {
        perror("posix_openpt");
        exit(EXIT_FAILURE);
    }
    if (grantpt(master_fd) != 0) {
        perror("grantpt");
        exit(EXIT_FAILURE);
    }
    if (unlockpt(master_fd) != 0) {
        perror("unlockpt");
        exit(EXIT_FAILURE);
    }
    slave_name = ptsname(master_fd);
    if (!slave_name) {
        perror("ptsname");
        exit(EXIT_FAILURE);
    }
    printf("Virtual serial port created: %s\n", slave_name);   
}

/* Helper: write 16-bit big-endian */
static void write_u16_be(uint8_t *buf, uint16_t v) {
    buf[0] = (v >> 8) & 0xFF;
    buf[1] = v & 0xFF;
}

/* Build payload: synthetic sensors (float32 big-endian) + timestamp (uint32) */
static size_t build_payload(uint8_t *payload, uint16_t seq) {
    /* Example synthetic values */
    float temperature = 20.0f + (seq % 50) * 0.1f; /* cycles */
    float pressure = 1013.25f + (seq % 20) * 0.5f;
    float battery = 3.7f + (seq % 10) * 0.01f;
    uint32_t ts = (uint32_t)xTaskGetTickCount(); /* tick count as simple timestamp */

    /* pack floats big-endian */
    union { float f; uint32_t u; } fu;
    size_t idx = 0;

    /* Pack temperature */
    fu.f = temperature;
    write_u16_be(&payload[idx], (fu.u >> 16) & 0xFFFF); idx += 2;
    write_u16_be(&payload[idx], fu.u & 0xFFFF); idx += 2;

    /* Pack pressure */
    fu.f = pressure;
    write_u16_be(&payload[idx], (fu.u >> 16) & 0xFFFF); idx += 2;
    write_u16_be(&payload[idx], fu.u & 0xFFFF); idx += 2;

    /* Pack battery */
    fu.f = battery;
    write_u16_be(&payload[idx], (fu.u >> 16) & 0xFFFF); idx += 2;
    write_u16_be(&payload[idx], fu.u & 0xFFFF); idx += 2;

    /* timestamp as big-endian U32 */
    payload[idx++] = (ts >> 24) & 0xFF;
    payload[idx++] = (ts >> 16) & 0xFF;
    payload[idx++] = (ts >> 8) & 0xFF;
    payload[idx++] = ts & 0xFF;

    return idx;
}

/* Emit one CCSDS-like packet to stdout */
static void emit_packet(uint16_t seq) {
    uint8_t packet[6 + MAX_PAYLOAD_SIZE];
    uint8_t payload[MAX_PAYLOAD_SIZE];
    memset(packet, 0, sizeof(packet));
    memset(payload, 0, sizeof(payload));

    size_t payload_len = build_payload(payload, seq);

    /* Build primary header */
    /* Packet ID: use 16-bit value (in real CCSDS, APID + type/version bits) */
    write_u16_be(&packet[0], PACKET_ID & 0xFFFF);

    /* Packet Sequence Control: 2 bits flags = 3 (standalone), 14-bit seq */
    uint16_t seq_field = (3u << 14) | (seq & 0x3FFF);
    write_u16_be(&packet[2], seq_field);

    /* Packet Length = (payload_len - 1) per CCSDS if using packet length field.
       Here we set to payload_len - 1 (but ensure non-negative). */
    uint16_t pkt_len_field = (payload_len > 0) ? (uint16_t)(payload_len - 1) : 0;
    write_u16_be(&packet[4], pkt_len_field);

    /* Copy payload */
    memcpy(&packet[6], payload, payload_len);

    size_t total_len = 6 + payload_len;

    /* Transmit packet to the virtual serial port */
    const char *test_msg = "Serial port test message\n";
    ssize_t n = write(master_fd, packet, total_len);

    /* Print hex bytes */
    for (size_t i = 0; i < total_len; ++i) {
        printf("%02X", packet[i]);
        // if (i + 1 < total_len) putchar('');
    }
    putchar('\n');

    /* Also print human-readable line */
    printf("SEQ=%u T=%.2f P=%.2f V=%.3f\n\n",
           seq,
           20.0f + (seq % 50) * 0.1f,
           1013.25f + (seq % 20) * 0.5f,
           3.7f + (seq % 10) * 0.01f);
    fflush(stdout);
}

/* FreeRTOS task */
static void vEmitterTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(EMIT_PERIOD_MS);

    for (;;) {
        /* emit packet */
        emit_packet(seq_counter);

        /* increment 14-bit sequence counter wrap at 16383 */
        seq_counter = (seq_counter + 1) & 0x3FFF;

        /* wait until next period */
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

int main_custom_demo(void) {
    printf("Starting custom demo: CCSDS-like packet emitter\n");
    setup_virtual_serial_port();
 
    /* Create emitter task */
    xTaskCreate(vEmitterTask, "Emitter", configMINIMAL_STACK_SIZE + 256, NULL, tskIDLE_PRIORITY + 1, NULL);

    /* Start scheduler (POSIX port provides pthread-based scheduler) */
    vTaskStartScheduler();

    /* Should never reach here */
    return 0;
}
