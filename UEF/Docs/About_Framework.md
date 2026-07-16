Universal Embedded Framework (UEF)

```txt
┌─────────────────────────────────────────────────────────────────────┐
│                         APPLICATION                                │
│                                                                      │
│  main.c (NEVER CHANGES)                                            │
│  ├── Uses UEF_GPIO_Init()                                          │
│  ├── Uses UEF_UART_Send()                                          │
│  └── Uses UEF_SPI_Transfer()                                       │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                     UEF SERVICES (Optional)                         │
│                                                                      │
│  UEF_Logger.c (NEVER CHANGES)                                      │
│  UEF_RingBuffer.c (NEVER CHANGES)                                  │
│  UEF_FirmwareUpdate.c (NEVER CHANGES)                              │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      UEF DRIVERS (Core)                             │
│                                                                      │
│  UEF_uart.c    (NEVER CHANGES)                                     │
│  UEF_spi.c     (NEVER CHANGES)                                     │
│  UEF_i2c.c     (NEVER CHANGES)                                     │
│  UEF_gpio.c    (NEVER CHANGES)                                     │
│  UEF_adc.c     (NEVER CHANGES)                                     │
│                                                                      │
│  All hardware access through ChipLink macros!                      │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      ChipLink.h (THE BRIDGE)                        │
│                                                                      │
│  // STEP 1: Type Mapping                                            │
│  typedef USART_TypeDef UEF_UART_Reg_t;                             │
│  typedef SPI_TypeDef UEF_SPI_Reg_t;                                │
│                                                                      │
│  // STEP 2: Instance Mapping                                        │
│  #define UEF_SPI_GET(n) ((n==1)?SPI1:SPI2)                         │
│                                                                      │
│  // STEP 3: Clock Control                                           │
│  #define UEF_SPI_CLK_ENABLE(n) RCC->APB2ENR |= (1<<12)             │
│                                                                      │
│  // STEP 4: Register Access                                         │
│  #define UEF_SPI_WRITE_CR1(spi,val) (spi)->CR1 = (val)             │
│                                                                      │
│  // STEP 5: Pin Mapping                                             │
│  #define UEF_PIN_SPI1_SCK  { GPIOA, 5, AF5 }                       │
│                                                                      │
│  🔧 ONLY FILE THAT CHANGES PER MCU                                │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    VENDOR CMSIS HEADERS                             │
│                                                                      │
│  stm32f411xe.h (Provided by ST)                                    │
│  ├── Peripheral register definitions                               │
│  ├── Bit definitions                                               │
│  └── Memory maps                                                   │
│                                                                      │
│  📦 Provided by MCU Vendor                                         │
│  (Different for ST, NXP, ESP32, etc.)                              │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                         HARDWARE                                    │
│                                                                      │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐                        │
│  │  SPI1   │    │  UART2  │    │  GPIOA  │                        │
│  │ Registers│    │Registers│    │Registers│                        │
│  └─────────┘    └─────────┘    └─────────┘                        │
│                                                                      │
│  Physical pins on the MCU                                          │
└─────────────────────────────────────────────────────────────────────┘

```

                 APPLICATION
                      |
                      |
              Framework APIs
                      |
        +-------------+-------------+
        |                           |
     Drivers                     Services
        |                           |
        +-------------+-------------+
                      |
                Port Layer
                      |
             Vendor SDK/CMSIS
                      |
                 Hardware

Mental Model:
```
              Application (NEVER CHANGES)
                        |
                        v
              UEF Drivers (NEVER CHANGES)
                        |
                        v
                 Middleware Services
                        |
                        v
            MCU Port Layer (CHANGES PER MCU)
                        |
                        v
      STM32F411 Vendor Definitions (COPIED, NEVER MODIFIED)
                        |
                        v
              STM32F411 Hardware Pins (MCU)
```

Structure:
```
Universal_Driver/
│
├── core/                           # LAYER 3: NEVER CHANGES!
│   ├── universal_uart.h           # Public API
│   ├── universal_uart.c           # Protocol logic
│   ├── universal_spi.h
│   ├── universal_spi.c
│   ├── universal_i2c.h
│   └── universal_i2c.c
│
├── middleware/                     # LAYER 2: RARELY CHANGES
│   ├── ring_buffer.c              # Buffer management
│   ├── dma_manager.c              # DMA logic
│   ├── interrupt_manager.c        # Interrupt handling
│   └── error_handler.c            # Error handling
│
├── port/                          # LAYER 1: CHANGES PER MCU!
│   ├── mcu_stm32f4/
│   │   ├── ChipLink_config.h         # Register addresses
│   │   ├── ChipLink_pins.h           # Pin mappings
│   │   ├── ChipLink_clocks.h         # Clock definitions
│   │   ├── ChipLink_interrupts.h     # IRQ numbers
│   │   └── ChipLink_dma.h            # DMA definitions
│   │
│   ├── mcu_stm32f7/
│   │   └── (similar structure)
│   │
│   ├── mcu_nxp_lpc/
│   │   └── (similar structure)
│   │
│   └── mcu_ti_cc13xx/
│       └── (similar structure)
│
├── examples/
│   ├── stm32f4_example.c
│   ├── stm32f7_example.c
│   └── ...
│
└── docs/

    ├── porting_guide.md           # How to port to new MCU

    └── api_reference.md

```

PART 1 --- DRIVERS
================

Drivers are hardware abstraction.

Think about it like this:

They answer:
>"What do I want?""


Example:

```
Send UART data
Read sensor
Control motor
```

Middleware
----------

"How do I manage complex software behavior?"

Example:

```
Buffer the data
Queue commands
Handle errors
Schedule tasks
Manage memory
```


Port
----

"How does THIS specific MCU physically do it?"

Example:

STM32:

```
USART2->DR
```

ESP32:

```
UART_FIFO_REG
```

NXP:

```
FLEXCOMM->TXDAT
```