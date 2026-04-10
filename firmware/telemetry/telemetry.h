#ifndef TELEMETRY_H
#define TELEMETRY_H

// #include <stdint.h>
// #include <stdbool.h>
#include <FreeRTOS.h>

// possible future functions for a more complete telemetry system:
// // Initializes the telemetry system
// void telemetry_init(void);

// // Sends telemetry data
// bool telemetry_send(const uint8_t *data, uint16_t length);

// // Receives telemetry data
// int telemetry_receive(uint8_t *buffer, uint16_t buffer_size);

// // Processes incoming telemetry messages
// void telemetry_process(void);

// // Shuts down the telemetry system
// void telemetry_shutdown(void);

extern size_t build_payload(uint8_t *payload, uint16_t seq);
extern void emit_packet(uint16_t seq);
extern void vEmitterTask(void *pvParameters);
static void write_u16_be(uint8_t *buf, uint16_t v);

#endif // TELEMETRY_H