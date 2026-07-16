# 🔌 UEF - Universal Embedded Framework

**Write Once. Run on Any MCU.**

UEF (Universal Embedded Framework) is a hardware-abstraction layer that provides a consistent API across different microcontrollers. Write your application once using UEF drivers, and port it to any supported MCU by changing just **one file**.

## ✨ Key Features

- **Single API for all MCUs** - Learn once, use everywhere
- **Zero code changes for application** - Same main.c works on STM32, NXP, ESP32
- **One-file porting** - Only `ChipLink.h` changes per MCU
- **Hardware-optimized** - Compiles to direct register access (no overhead)
- **Built-in services** - Logger, RingBuffer, Firmware Update
- **RTOS-ready** - Designed for both bare-metal and RTOS environments

## 🏗️ Architecture
```
┌─────────────────────────────────────────────┐
│ Application (main.c) - NEVER CHANGES        │
└─────────────────────────────────────────────┘
                      │
┌─────────────────────────────────────────────┐
│ UEF Drivers - NEVER CHANGES                 │
│ UART, SPI, I2C, GPIO, ADC, PWM, Timer       │
└─────────────────────────────────────────────┘
                      │
┌─────────────────────────────────────────────┐
│ ChipLink.h - ONLY FILE THAT CHANGES         │
│ ~200 lines of MCU-specific mappings         │
└─────────────────────────────────────────────┘
                      │
┌─────────────────────────────────────────────┐
│ Vendor CMSIS - Provided by MCU vendor       │
└─────────────────────────────────────────────┘
```

## 📦 Supported Peripherals
```

| Driver   | Status       | Interrupt | DMA |
|----------|--------======|-----------|-----|
| GPIO     | ✅ Complete |     ✅    | -   |
| UART     | ✅ Complete |     ✅    | ✅ |
| SPI      | ✅ Complete |     ✅    | ✅ |
| I2C      | ✅ Complete |     ✅    | -   |
| Timer    | ✅ Complete |     ✅    | -   |
| PWM      | ✅ Complete |     ✅    | -   |
| ADC      | ✅ Complete |     ✅    | ✅ |
| RTC      | ✅ Complete |     -      | -   |
| Watchdog | ✅ Complete |     -      | -   |
| DMA      | 🚧 In Progress |   -     | -   |
```
## 🛠️ Built-in Services

- **Logger** - Configurable logging with levels (ERROR, WARN, INFO, DEBUG, TRACE)
- **RingBuffer** - Thread-safe circular buffer
- **Firmware Update** - Over-the-air or serial update system
- **Scheduler** - Cooperative task scheduler (coming soon)

## 🚀 Quick Start

### 1. Write your application
```c
#include "UEF_uart.h"
#include "UEF_gpio.h"

int main(void) {
    UEF_UART_Handle_t uart = { .instance_num = 1 };
    UEF_UART_Init(&uart);
    
    UEF_GPIO_Handle_t led;
    UEF_GPIO_Config_t cfg = { .port = GPIOA, .pin = 5, .mode = UEF_GPIO_MODE_OUTPUT_PP };
    UEF_GPIO_Init(&led, &cfg);
    
    while(1) {
        UEF_UART_SendString(&uart, "Hello World!\r\n", 1000);
        UEF_GPIO_Toggle(&led);
        UEF_DelayMs(500);
    }
}
```
### 2 Port to mew MCU
```
# Copy ChipLink template
cp -r ChipLink_Template/ UEF/Ports/NewMCU/

# Edit ONE file (ChipLink.h) to map hardware
vim UEF/Ports/NewMCU/ChipLink.h

# Done! Application code works immediately
```
🤝 Why UEF?
-----------

Problem: Every MCU vendor has different APIs. Switching from STM32 to NXP means rewriting everything.

Solution: UEF provides one API that works on all MCUs. Write your application once, deploy anywhere.

How it works: The `ChipLink.h` file maps UEF API calls to vendor-specific register operations. Only this one file changes per MCU.

🏆 Use Cases
------------

-   Product lines - Use the same code across multiple hardware variants

-   IoT devices - Support multiple MCUs with one codebase

-   Rapid prototyping - Switch MCUs without rewriting code

-   Legacy migration - Move from one MCU to another seamlessly

-   Education - Learn one API, apply to any MCU
