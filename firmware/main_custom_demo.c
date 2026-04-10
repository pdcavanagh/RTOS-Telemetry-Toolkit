#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "telemetry/telemetry.h"

int master_fd = -1;

char *slave_name = NULL;
QueueHandle_t xTcQueue;

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

// TODO: Make master_fd non-blocking or add timeout to avoid blocking the scheduler in vReceiveTelecommandTask.
static void vReceiveTelecommandTask(void *pvParameters) {
    for (;;) {
        uint8_t buf[64];
        ssize_t n = read(master_fd, buf, sizeof(buf));
        if (n > 0) {
            printf("Telecommand RX (%zd bytes): ", n);
            for (ssize_t i = 0; i < n; ++i) {
                printf("%02X ", buf[i]);
            }
            printf("\n");
            fflush(stdout);

            xQueueSend(xTcQueue, buf, portMAX_DELAY);
        }
    }
}

/* todo */
static void vProcessTelecommand(void *pvParameters) {
    uint8_t tc_buf[64];
    while (xQueueReceive(xTcQueue, &tc_buf, portMAX_DELAY) == pdPASS) {
        /* Print telecommand as hex bytes */
        printf("Processing telecommand: ");
        for (int i = 0; i < 64; ++i) {
            printf("%02X ", tc_buf[i]);
        }
        printf("\n");
        fflush(stdout);
    }
}

int main_custom_demo(void) {
    xTcQueue = xQueueCreate(10, sizeof(uint8_t) * 64);
    if (xTcQueue == NULL) {
        printf("Failed to create telecommand queue\n");
        return -1;
    }

    printf("Starting custom demo: CCSDS-like packet emitter\n");
    setup_virtual_serial_port();
 
    /* Create emitter task */
    xTaskCreate(vEmitterTask, "Emitter", configMINIMAL_STACK_SIZE + 256, NULL, tskIDLE_PRIORITY + 1, NULL);

    /* Create telecommand task */
    // xTaskCreate(vReceiveTelecommandTask, "Telecommand", configMINIMAL_STACK_SIZE + 256, NULL, tskIDLE_PRIORITY + 2, NULL);

    // Processing task for telecommands (not fully implemented)
    xTaskCreate(vProcessTelecommand, "ProcessTC", configMINIMAL_STACK_SIZE + 256, NULL, tskIDLE_PRIORITY + 3, NULL);

    /* Start scheduler (POSIX port provides pthread-based scheduler) */
    vTaskStartScheduler();

    /* Should never reach here */
    return 0;
}
