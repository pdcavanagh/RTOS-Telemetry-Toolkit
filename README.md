# RTOS Telemetry Toolkit

FreeRTOS POSIX implementation to demonstrate CCSDS packet creation. Python application to consume and decode CCSDS packets.

## Overview
Sandbox for running FreeRTOS locally and exchanging telemetry/telecommands with a Python client over a virtual serial/PTTY (coming soon).

## Contents
- `firmware/` — FreeRTOS app or POSIX-port source (C/C++)
- `host/` — Python client for telemetry/telecommands

## Quickstart

**Prerequisites**
- Linux or macOS (for PTY support)
- gcc/clang, make, or the toolchain used for the FreeRTOS POSIX port
- Python 3.8+ and pip

**Steps**
1. Build/run FreeRTOS (POSIX port)
   ```bash
   cd firmware
   make CC=gcc USER_DEMO=CUSTOM_DEMO 
   ```
2. Start Python client
   ```bash
   cd host 
   python telemetry_decoder.py
   ```

## License
Specify license (e.g., MIT). Include a `LICENSE` file.

## Contact
Maintainer: Patrick Cavanagh — pdcavanagh@yahoo.com
