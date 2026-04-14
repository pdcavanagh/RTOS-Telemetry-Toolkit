#include <unistd.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include "telecommands.h"
#include "globals.h"

// TODO: Make master_fd non-blocking or add timeout to avoid blocking the scheduler in vReceiveTelecommandTask.
extern void vReceiveTelecommandTask(void *pvParameters) {
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
extern void vProcessTelecommand(void *pvParameters) {
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