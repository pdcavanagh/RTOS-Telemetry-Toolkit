#ifndef TELECOMMANDS_H
#define TELECOMMANDS_H

#include <FreeRTOS.h>
#include "globals.h"

extern void vReceiveTelecommandTask(void *pvParameters);
extern void vProcessTelecommand(void *pvParameters);

#endif // TELECOMMANDS_H