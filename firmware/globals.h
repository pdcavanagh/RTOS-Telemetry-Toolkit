#ifndef GLOBALS_H
#define GLOBALS_H

#include <FreeRTOS.h>
#include <queue.h>

extern int master_fd;
extern QueueHandle_t xTcQueue;

#endif // GLOBALS_H
