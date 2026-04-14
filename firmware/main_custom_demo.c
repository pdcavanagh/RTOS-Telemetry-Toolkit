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
#include "telecommands/telecommands.h"

int master_fd = -1;
char *slave_name = NULL;
QueueHandle_t xTcQueue = NULL;

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

int main_custom_demo(void) {
    xTcQueue = xQueueCreate(10, sizeof(uint8_t) * 64);
    if (xTcQueue == NULL) {
        printf("Failed to create telecommand queue\n");
        return -1;
    }

    printf("Starting custom demo: CCSDS-like packet emitter\n");
    setup_virtual_serial_port();
 
    /* Create emitter task */
    // xTaskCreate(vEmitterTask, "Emitter", configMINIMAL_STACK_SIZE + 256, NULL, tskIDLE_PRIORITY + 1, NULL);

    /* Create telecommand task */
    xTaskCreate(vReceiveTelecommandTask, "Telecommand", configMINIMAL_STACK_SIZE + 256, NULL, tskIDLE_PRIORITY + 2, NULL);

    // Processing task for telecommands (not fully implemented)
    xTaskCreate(vProcessTelecommand, "ProcessTC", configMINIMAL_STACK_SIZE + 256, NULL, tskIDLE_PRIORITY + 3, NULL);

    /* Start scheduler (POSIX port provides pthread-based scheduler) */
    vTaskStartScheduler();

    /* Should never reach here */
    return 0;
}
