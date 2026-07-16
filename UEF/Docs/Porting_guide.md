# ChipLink Porting Guide

```markdown

# ChipLink Porting Guide

## Table of Contents

1\. [Introduction](#introduction)

2\. [What is ChipLink?](#what-is-chiplink)

3\. [Porting Overview](#porting-overview)

4\. [Step-by-Step Porting Process](#step-by-step-porting-process)

5\. [Detailed Section Guide](#detailed-section-guide)

6\. [Testing Your Port](#testing-your-port)

7\. [Common Pitfalls](#common-pitfalls)

8\. [Porting Checklist](#porting-checklist)

9\. [Example: Porting to a New MCU](#example-porting-to-a-new-mcu)

10\. [Troubleshooting](#troubleshooting)

---

## Introduction

This guide explains how to port the UEF (Unified Embedded Framework) to a new MCU by creating a new ChipLink file.

**Key Concept:** The UEF framework is designed so that **only one file** needs to be modified when porting to a new MCU - the `ChipLink.h` file. All UEF_Drivers and UEF_Services remain **100% identical** across all MCUs.

---

## What is ChipLink?

ChipLink is the hardware abstraction layer that sits between the UEF drivers and the vendor-provided CMSIS/SDK headers. It:

- Maps vendor register types to UEF types

- Provides MCU-specific implementations of clock, GPIO, UART, SPI, I2C, etc.

- Contains all vendor-specific code in one file

- Is the **only** file that knows about vendor register names and bit definitions

```

┌─────────────────────────────────────────────────────────────┐

│                 Application (main.c)                        │

└──────────────────────────┬──────────────────────────────────┘

                           │

┌──────────────────────────▼──────────────────────────────────┐

│              UEF_Drivers (Portable)                         │

│  - UEF_uart.c    - UEF_spi.c    - UEF_i2c.c               │

│  - UEF_gpio.c    - UEF_adc.c    - UEF_pwm.c               │

│  - UEF_clock.c   - UEF_timer.c  - UEF_dma.c               │

└──────────────────────────┬──────────────────────────────────┘

                           │

┌──────────────────────────▼──────────────────────────────────┐

│            ChipLink.h (MCU-Specific - ONLY THIS CHANGES!)  │

│  - Vendor type mappings                                     │

│  - Register access functions                                │

│  - Pin mappings                                             │

│  - Clock configuration                                      │

└──────────────────────────┬──────────────────────────────────┘

                           │

┌──────────────────────────▼──────────────────────────────────┐

│         Vendor CMSIS/SDK Headers (Provided by vendor)       │

│  - stm32f411xe.h     - core_cm4.h     - system_stm32f4xx.h │

└─────────────────────────────────────────────────────────────┘

```

---

## Porting Overview

### Time Estimate

- **Simple MCU** (similar to STM32F4): 2-4 hours

- **Different ARM MCU** (NXP, NRF, etc.): 4-8 hours

- **Non-ARM MCU** (ESP32, RISC-V, etc.): 8-16 hours

### Prerequisites

1\. Vendor CMSIS/SDK headers for your target MCU

2\. Datasheet and reference manual for your target MCU

3\. Basic understanding of the MCU's peripherals

4\. A working development environment for the target MCU

### What You Need to Know

- The MCU's clock tree and how to configure it

- GPIO register layout and pin configuration

- UART/SPI/I2C register maps and configuration

- Interrupt numbers and NVIC configuration

- Flash memory organization (if using firmware update)

---

## Step-by-Step Porting Process

### Step 1: Create the Port Directory

```

UEF/Ports/[VENDOR]/[MCU_NAME]/

├── ChipLink/

│   └── ChipLink.h          ← YOUR NEW CHIPLINK FILE

└── vendor/                 ← VENDOR CMSIS/SDK HEADERS

    ├── [mcu_name].h

    ├── system_[mcu_name].h

    ├── core_cmX.h

    ├── cmsis_compiler.h

    └── cmsis_version.h

```

### Step 2: Copy the ChipLink Template

Start with a working ChipLink.h from a similar MCU (e.g., STM32F411RE) and rename it.

### Step 3: Update Vendor Includes

Replace the vendor includes at the top of ChipLink.h:

```c

// OLD (STM32F411RE)

#include "vendor/stm32f411xe.h"

#include "vendor/system_stm32f4xx.h"

// NEW (Your MCU)

#include "vendor/[your_mcu_header].h"

#include "vendor/system_[your_mcu].h"

```

### Step 4: Fill in Each Section

Follow the detailed guide below for each section.

---

## Detailed Section Guide

### SECTION 0: CHIP IDENTITY

Update this section with your MCU information:

```c

#define CHIPLINK_VENDOR        "YourVendorName"  // e.g., "NXP", "Microchip"

#define CHIPLINK_MCU           "YourMCUName"     // e.g., "LPC1768", "ATSAM4S"

#define CHIPLINK_CORE          "Cortex-MX"       // e.g., "Cortex-M3", "Cortex-M4F"

```

### SECTION 0.5: CONFIGURATION VALIDATION

Add a guard to prevent using the wrong ChipLink:

```c

// Replace STM32F411xE with your MCU's define

#if !defined(YOUR_MCU_DEFINE)

#error "ChipLink.h is configured for YOUR_MCU, but a different MCU is selected!"

#endif

```

### SECTION 0.6: PERIPHERAL CAPABILITIES

Update these counts based on your MCU:

```c

#define CHIPLINK_UART_COUNT     3   // Number of UART peripherals

#define CHIPLINK_SPI_COUNT      5   // Number of SPI peripherals  

#define CHIPLINK_I2C_COUNT      3   // Number of I2C peripherals

#define CHIPLINK_ADC_COUNT      1   // Number of ADC peripherals

#define CHIPLINK_TIMER_COUNT    8   // Number of timers

// Features present on your MCU (1 = present, 0 = absent)

#define CHIPLINK_FEATURE_DMA    1

#define CHIPLINK_FEATURE_USB    1

#define CHIPLINK_FEATURE_CAN    0

#define CHIPLINK_FEATURE_SDIO   1

#define CHIPLINK_FEATURE_HW_FLOW  0  // Hardware flow control

```

### SECTION 1: REGISTER TYPE MAPPING

Map vendor types to UEF types:

```c

// Replace these with your MCU's peripheral types

typedef USART_TypeDef      UEF_UART_Reg_t;

typedef GPIO_TypeDef       UEF_GPIO_Reg_t;

typedef SPI_TypeDef        UEF_SPI_Reg_t;

typedef I2C_TypeDef        UEF_I2C_Reg_t;

typedef ADC_TypeDef        UEF_ADC_Reg_t;

typedef TIM_TypeDef        UEF_TIM_Reg_t;

typedef DMA_Stream_TypeDef UEF_DMA_Stream_t;

typedef IRQn_Type          UEF_IRQ_t;

```

**Common vendor type names:**

- STM32: `USART_TypeDef`, `GPIO_TypeDef`, `SPI_TypeDef`

- NXP LPC: `LPC_USART_T`, `LPC_GPIO_T`, `LPC_SSP_T`

- NRF: `NRF_UARTE_Type`, `NRF_GPIO_Type`, `NRF_SPIM_Type`

- Microchip SAM: `Usart`, `Port`, `Spi`

### SECTION 2: COMMON / TICK

Implement the system tick function:

```c

// External declaration - implemented by UEF_clock.c

extern uint32_t UEF_Clock_GetTick(void);

// Weak attribute allows override by application if needed

__attribute__((weak)) 

static inline uint32_t UEF_GetTick(void) { 

    return UEF_Clock_GetTick(); 

}

```

### SECTION 3: GPIO

#### 3.1 GPIO Mode Values

Replace these with your MCU's mode values:

```c

// Replace with actual values from your MCU

#define UEF_GPIO_MODE_INPUT_VAL     0x00UL  // Input mode

#define UEF_GPIO_MODE_OUTPUT_VAL    0x01UL  // Output mode

#define UEF_GPIO_MODE_AF_VAL        0x02UL  // Alternate function

#define UEF_GPIO_MODE_ANALOG_VAL    0x03UL  // Analog mode

```

#### 3.2 GPIO Clock Enable

Implement GPIO clock enable/disable:

```c

static inline void UEF_GPIO_CLK_ENABLE(void* port) {

    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;

    // Enable the bus clock for this GPIO port

    // Example for STM32:

    if (gpio == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // ... add all ports

}

static inline void UEF_GPIO_CLK_DISABLE(void* port) {

    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;

    // Disable the bus clock for this GPIO port

}

```

**Tips:**

- Check your MCU's clock enable registers

- Some MCUs use separate `SET` and `CLR` registers

- Some MCUs have different bus domains (APB1, APB2, AHB1, etc.)

#### 3.3 Pin Mapping

Define useful pin mappings for your board:

```c

// Replace with your board's pins

#define UEF_PIN_LED1        GPIOA, 5    // Onboard LED

#define UEF_PIN_LED2        GPIOA, 6    // Second LED

#define UEF_PIN_BUTTON      GPIOC, 13   // User button

#define UEF_PIN_BUZZER      GPIOB, 8    // Buzzer output

```

### SECTION 4: UART

#### 4.1 UART Instance Mapping

Map instance numbers to hardware pointers:

```c

static inline USART_TypeDef* UEF_UART_GET(uint8_t instance) {

    switch (instance) {

        case 1: return USART1;

        case 2: return USART2;

        case 3: return USART3;  // If available

        // ... add all UART instances

        default: return NULL;

    }

}

```

#### 4.2 UART Flags

Define UART status flags:

```c

#define UEF_UART_FLAG_TXE      USART_SR_TXE   // Transmit buffer empty

#define UEF_UART_FLAG_TC       USART_SR_TC    // Transmission complete

#define UEF_UART_FLAG_RXNE     USART_SR_RXNE  // Receive buffer not empty

#define UEF_UART_FLAG_ORE      USART_SR_ORE   // Overrun error

#define UEF_UART_FLAG_FE       USART_SR_FE    // Framing error

#define UEF_UART_FLAG_PE       USART_SR_PE    // Parity error

```

**Note:** Flag names vary by MCU. Check your datasheet.

#### 4.3 UART Control Register Bits

Define control register bits:

```c

#define UEF_UART_CR1_UE        USART_CR1_UE   // UART enable

#define UEF_UART_CR1_TE        USART_CR1_TE   // Transmitter enable

#define UEF_UART_CR1_RE        USART_CR1_RE   // Receiver enable

#define UEF_UART_CR1_TXEIE     USART_CR1_TXEIE // TX interrupt enable

#define UEF_UART_CR1_RXNEIE    USART_CR1_RXNEIE // RX interrupt enable

#define UEF_UART_CR1_TCIE      USART_CR1_TCIE // TC interrupt enable

#define UEF_UART_CR3_EIE       USART_CR3_EIE  // Error interrupt enable

```

#### 4.4 UART Clock Enable

```c

static inline void UEF_UART_CLK_ENABLE(uint8_t instance) {

    // Enable the bus clock for this UART instance

    if (instance == 1) RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // ... add all instances

}

```

#### 4.5 UART Interrupts

```c

static inline UEF_IRQ_t UEF_UART_GET_IRQ(uint8_t instance) {

    switch (instance) {

        case 1: return USART1_IRQn;

        // ... add all instances

        default: return (UEF_IRQ_t)-1;

    }

}

static inline void UEF_UART_ENABLE_IRQ(uint8_t instance) {

    UEF_IRQ_t irq = UEF_UART_GET_IRQ(instance);

    if (irq != (UEF_IRQ_t)-1) NVIC_EnableIRQ(irq);

}

```

#### 4.6 UART Clock Frequency

```c

// Get the bus frequency for this UART instance

static inline uint32_t UEF_UART_GetClockFreq(uint8_t instance) {

    // Return APB frequency based on which bus the UART is on

    if (instance == 2) return UEF_Clock_GetAPB1Freq();

    return UEF_Clock_GetAPB2Freq();

}

```

#### 4.7 UART Baud Rate Calculation

```c

// Calculate the baud rate register value

static inline uint32_t UEF_UART_CalcBRR(uint32_t pclk, uint32_t baud) {

    // Most ARM MCUs use this formula:

    return (pclk + (baud / 2U)) / baud;

    // Some MCUs (like STM32) use oversampling:

    // return ((pclk * 8U + baud * 4U) / (baud * 8U)) * 2U;

}

```

#### 4.8 UART Pin Mapping

Define pin mappings for each UART instance:

```c

typedef struct {

    GPIO_TypeDef* port;

    uint16_t      pin;

    uint8_t       af;

    uint8_t       is_tx;

} UEF_UART_PinMap_t;

// Define the alternate function numbers

#define GPIO_AF7_USART1   7U

#define GPIO_AF7_USART2   7U

#define GPIO_AF7_USART3   7U

// Get the pin map for a UART instance

static inline const UEF_UART_PinMap_t* UEF_UART_GetPinMap(uint8_t instance, uint8_t is_tx) {

    static const UEF_UART_PinMap_t usart1_tx = { GPIOA, 9,  GPIO_AF7_USART1, 1 };

    static const UEF_UART_PinMap_t usart1_rx = { GPIOA, 10, GPIO_AF7_USART1, 0 };

    // ... define all TX/RX pairs

    switch (instance) {

        case 1: return is_tx ? &usart1_tx : &usart1_rx;

        // ... add all instances

        default: return NULL;

    }

}

```

#### 4.9 UART Pin Configuration

```c

static inline int UEF_UART_ConfigurePin(const UEF_UART_PinMap_t* p) {

    if (!p || !p->port) return -1;

    GPIO_TypeDef* gpio = p->port;

    uint32_t pin = p->pin;

    // 1. Enable GPIO clock

    UEF_GPIO_CLK_ENABLE(gpio);

    // 2. Set to alternate function mode

    gpio->MODER = (gpio->MODER & ~(0x3UL << (pin * 2))) | 

                  (UEF_GPIO_MODE_AF_VAL << (pin * 2));

    // 3. Set speed (if applicable)

    gpio->OSPEEDR = (gpio->OSPEEDR & ~(0x3UL << (pin * 2))) | 

                    (0x2UL << (pin * 2));

    // 4. Set pull-up/pull-down (pull-up on RX)

    gpio->PUPDR = (gpio->PUPDR & ~(0x3UL << (pin * 2)));

    if (!p->is_tx) {

        gpio->PUPDR |= (0x1UL << (pin * 2));  // Pull-up on RX

    }

    // 5. Set alternate function number

    if (pin < 8) {

        gpio->AFR[0] = (gpio->AFR[0] & ~(0xFUL << (pin * 4))) | 

                       ((uint32_t)p->af << (pin * 4));

    } else {

        gpio->AFR[1] = (gpio->AFR[1] & ~(0xFUL << ((pin - 8) * 4))) | 

                       ((uint32_t)p->af << ((pin - 8) * 4));

    }

    return 0;

}

```

### SECTION 5: SPI

#### 5.1 SPI Instance Mapping

```c

static inline SPI_TypeDef* UEF_SPI_GET(uint8_t instance) {

    switch (instance) {

        case 1: return SPI1;

        case 2: return SPI2;

        // ... add all instances

        default: return NULL;

    }

}

```

#### 5.2 SPI Control Bits

```c

#define UEF_SPI_CR1_MSTR       SPI_CR1_MSTR     // Master mode

#define UEF_SPI_CR1_CPOL       SPI_CR1_CPOL     // Clock polarity

#define UEF_SPI_CR1_CPHA       SPI_CR1_CPHA     // Clock phase

#define UEF_SPI_CR1_LSBFIRST   SPI_CR1_LSBFIRST // LSB first

#define UEF_SPI_CR1_DFF        SPI_CR1_DFF      // 16-bit data

#define UEF_SPI_CR1_SSM        SPI_CR1_SSM      // Software SS

#define UEF_SPI_CR1_SPE        SPI_CR1_SPE      // SPI enable

#define UEF_SPI_CR2_TXEIE      SPI_CR2_TXEIE    // TX interrupt

#define UEF_SPI_CR2_RXNEIE     SPI_CR2_RXNEIE   // RX interrupt

#define UEF_SPI_CR2_ERRIE      SPI_CR2_ERRIE    // Error interrupt

#define UEF_SPI_FLAG_TXE       SPI_SR_TXE       // TX buffer empty

#define UEF_SPI_FLAG_RXNE      SPI_SR_RXNE      // RX buffer not empty

#define UEF_SPI_FLAG_BSY       SPI_SR_BSY       // Busy

```

#### 5.3 SPI Register Helpers

These functions wrap register access so the driver never touches `SPI_TypeDef` directly:

```c

static inline bool UEF_SPI_GET_FLAG(UEF_SPI_Reg_t* spi, uint32_t flag) {

    return (spi->SR & flag) != 0;

}

static inline bool UEF_SPI_IS_BUSY(UEF_SPI_Reg_t* spi) {

    return (spi->SR & SPI_SR_BSY) != 0;

}

static inline void UEF_SPI_ENABLE(UEF_SPI_Reg_t* spi) {

    spi->CR1 |= SPI_CR1_SPE;

}

static inline void UEF_SPI_DISABLE(UEF_SPI_Reg_t* spi) {

    spi->CR1 &= ~SPI_CR1_SPE;

}

static inline void UEF_SPI_WRITE_BYTE(UEF_SPI_Reg_t* spi, uint8_t b) {

    *(volatile uint8_t*)&spi->DR = b;

}

static inline uint8_t UEF_SPI_READ_BYTE(UEF_SPI_Reg_t* spi) {

    return *(volatile uint8_t*)&spi->DR;

}

```

#### 5.4 SPI Baud Rate Mapping

```c

// Map baud rate enum values to hardware bits

static inline uint32_t UEF_SPI_GetBaudDiv(uint8_t baud_rate_enum) {

    // On STM32, the enum matches the hardware bits

    return baud_rate_enum;

    // On other MCUs, you might need a lookup table:

    // static const uint32_t div_table[] = {2, 4, 8, 16, 32, 64, 128, 256};

    // return div_table[baud_rate_enum];

}

```

### SECTION 6: I2C

#### 6.1 I2C Instance Mapping

```c

static inline I2C_TypeDef* UEF_I2C_GET(uint8_t instance) {

    switch (instance) {

        case 1: return I2C1;

        case 2: return I2C2;

        case 3: return I2C3;

        default: return NULL;

    }

}

```

#### 6.2 I2C Clock Enable

```c

static inline void UEF_I2C_CLK_ENABLE(uint8_t instance) {

    switch (instance) {

        case 1: RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; break;

        case 2: RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; break;

        case 3: RCC->APB1ENR |= RCC_APB1ENR_I2C3EN; break;

        default: break;

    }

}

```

#### 6.3 I2C Pin Configuration

```c

typedef struct {

    GPIO_TypeDef* port;

    uint16_t      scl_pin;

    uint16_t      sda_pin;

    uint8_t       af;

} UEF_I2C_PinMap_t;

static inline int UEF_I2C_ConfigurePins(uint8_t instance) {

    // Define pin mappings for each I2C instance

    switch (instance) {

        case 1: {

            GPIO_TypeDef* gpio = GPIOB;

            UEF_GPIO_CLK_ENABLE(gpio);

            // Configure SCL (PB6) and SDA (PB7)

            // Both as AF, open-drain, with pull-ups

            // ... setup pins

            break;

        }

        // ... add all instances

        default: return -1;

    }

    return 0;

}

```

#### 6.4 I2C Clock Calculation

```c

static inline uint32_t UEF_I2C_CalcCCR(uint32_t pclk_hz, uint32_t speed_hz) {

    uint32_t ccr;

    if (speed_hz <= 100000U) {

        // Standard mode (100 kHz)

        ccr = pclk_hz / (speed_hz * 2U);

        if (ccr < 4U) ccr = 4U;

    } else {

        // Fast mode (400 kHz)

        ccr = pclk_hz / (speed_hz * 3U);

        ccr |= I2C_CCR_DUTY;  // Fast mode duty cycle

        if ((ccr & ~I2C_CCR_DUTY) < 1U) ccr |= 1U;

    }

    return ccr;

}

```

#### 6.5 I2C Error Handling

```c

static inline void UEF_I2C_ClearErrors(UEF_I2C_Reg_t* i2c) {

    uint32_t sr1 = i2c->SR1;

    (void)i2c->SR2;  // Read SR2 to clear some flags

    if (sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR)) {

        i2c->SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR);

    }

}

```

### SECTION 7: ADC

#### 7.1 ADC Instance Mapping

```c

static inline ADC_TypeDef* UEF_ADC_GET(uint8_t instance) {

    if (instance == 1) return ADC1;

    return NULL;

}

```

#### 7.2 ADC Clock Enable

```c

static inline void UEF_ADC_CLK_ENABLE(uint8_t instance) {

    (void)instance;

    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

}

static inline void UEF_ADC_CLK_DISABLE(uint8_t instance) {

    (void)instance;

    RCC->APB2ENR &= ~RCC_APB2ENR_ADC1EN;

}

```

#### 7.3 ADC Pin Configuration

```c

static inline void UEF_ADC_ConfigurePin(void* port, uint16_t pin) {

    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;

    UEF_GPIO_CLK_ENABLE(gpio);

    uint32_t shift2 = pin * 2U;

    gpio->MODER |= (0x3UL << shift2);   // Analog mode

    gpio->PUPDR &= ~(0x3UL << shift2);  // No pull

}

```

### SECTION 8: CLOCK TREE

#### 8.1 HSI Initialization

```c

static inline int UEF_ChipLink_Clock_InitHSI(void) {

    // Enable the internal RC oscillator

    // Wait for it to stabilize

    // Switch the system clock to it

    // Configure flash latency for the new frequency

    return 0;  // Return 0 on success, -1 on failure

}

```

#### 8.2 HSE Initialization

```c

static inline int UEF_ChipLink_Clock_InitHSE(void) {

    // Enable the external crystal oscillator

    // Wait for it to stabilize

    // Switch the system clock to it

    // Configure flash latency

    return 0;

}

```

#### 8.3 PLL Initialization

```c

static inline int UEF_ChipLink_Clock_InitPLL(uint32_t hse_freq) {

    // Configure the PLL for the desired frequency

    // Enable PLL

    // Wait for PLL to lock

    // Switch system clock to PLL

    // Configure flash latency

    return 0;

}

```

#### 8.4 Clock Frequency Queries

```c

static inline uint32_t UEF_ChipLink_Clock_GetSystemFreq(void) {

    // Return current system clock frequency in Hz

    return 16000000U;  // Example: 16 MHz

}

static inline uint32_t UEF_ChipLink_Clock_GetHCLKFreq(void) {

    // Return AHB bus frequency in Hz

    return UEF_ChipLink_Clock_GetSystemFreq();

}

static inline uint32_t UEF_ChipLink_Clock_GetAPB1Freq(void) {

    // Return APB1 bus frequency in Hz

    return UEF_ChipLink_Clock_GetHCLKFreq() / 4;  // Example: divided by 4

}

static inline uint32_t UEF_ChipLink_Clock_GetAPB2Freq(void) {

    // Return APB2 bus frequency in Hz

    return UEF_ChipLink_Clock_GetHCLKFreq() / 2;  // Example: divided by 2

}

```

#### 8.5 SysTick Configuration

```c

static inline int UEF_ChipLink_Clock_ConfigSysTick(uint32_t sysclk, uint32_t ticks_per_second) {

    uint32_t reload = sysclk / ticks_per_second - 1U;

    if (reload > 0xFFFFFFUL) return -1;  // Reload value too large

    SysTick->CTRL = 0;

    SysTick->LOAD = reload;

    SysTick->VAL = 0;

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

    NVIC_SetPriority(SysTick_IRQn, 0);

    return 0;

}

```

#### 8.6 Simple Delay

```c

static inline void UEF_ChipLink_Delay_Cycles(uint32_t cycles) {

    for (volatile uint32_t i = 0; i < cycles; i++) {

        __NOP();  // No operation - consumes one cycle

    }

}

```

---

## Testing Your Port

### Test Order (from simplest to most complex)

1\. **Compilation Test**

   ```bash

   make clean && make

   ```

   Ensure everything compiles without errors.

2\. **Clock Test**

   ```c

   // In main.c

   UEF_Clock_ConfigHSI();

   // Test with simple delay

   UEF_DelayMs(1000);  // Should delay ~1 second

   ```

3\. **GPIO Test**

   ```c

   // Configure an LED pin

   UEF_GPIO_Config_t led = {

       .port = GPIOA,

       .pin = 5,

       .mode = UEF_GPIO_MODE_OUTPUT_PP,

       .pull = UEF_GPIO_PULL_NONE,

       .speed = UEF_GPIO_SPEED_LOW

   };

   UEF_GPIO_Handle_t led_handle;

   UEF_GPIO_Init(&led_handle, &led);

   // Blink the LED

   while(1) {

       UEF_GPIO_Toggle(&led_handle);

       UEF_DelayMs(500);

   }

   ```

4\. **UART Test**

   ```c

   // Send "Hello World" over UART

   UEF_UART_Handle_t uart_handle;

   uart_handle.instance = CHIPLINK_UART2;

   uart_handle.instance_num = 2;

   UEF_UART_Init(&uart_handle);

   UEF_UART_SendString(&uart_handle, "Hello World!\r\n", 1000);

   ```

5\. **SPI Test** (if you have a device)

6\. **I2C Test** (if you have a device)

7\. **ADC Test** (if you have an analog source)

---

## Common Pitfalls

### 1. Wrong Bus Frequencies

**Problem:** Baud rates are incorrect (e.g., 115200 baud sends garbage)

**Solution:** Check which APB bus your UART is on. `UEF_UART_GetClockFreq()` must return the correct frequency.

### 2. Incorrect GPIO Mode Values

**Problem:** GPIO pins don't work as expected

**Solution:** Verify the mode values in your MCU's reference manual:

- Input: 0b00

- Output: 0b01

- AF: 0b10

- Analog: 0b11

### 3. Missing GPIO Clock Enable

**Problem:** GPIO doesn't work even though configuration looks correct

**Solution:** Many MCUs require you to enable the clock for each GPIO port you use.

### 4. Wrong Alternate Function Numbers

**Problem:** UART/SPI/I2C pins don't work

**Solution:** Check your MCU's datasheet for the correct AF number for each peripheral.

### 5. Flash Latency

**Problem:** System crashes or behaves erratically when running at high frequency

**Solution:** Configure flash wait states properly for your system clock frequency.

### 6. Interrupt Priorities

**Problem:** Interrupts don't fire or work correctly

**Solution:** Ensure NVIC priorities are set correctly and interrupts are enabled.

---

## Porting Checklist

Use this checklist to track your progress:

### Pre-Porting

- [ ] Read the MCU datasheet and reference manual

- [ ] Set up the development environment for the MCU

- [ ] Verify you can compile and flash a simple program

### Vendor CMSIS/SDK Headers

- [ ] Copy vendor headers to `./vendor/`

- [ ] Update includes in `ChipLink.h`

### SECTION 0: CHIP IDENTITY

- [ ] Update `CHIPLINK_VENDOR`

- [ ] Update `CHIPLINK_MCU`

- [ ] Update `CHIPLINK_CORE`

### SECTION 0.5: CONFIGURATION VALIDATION

- [ ] Add the MCU define guard

### SECTION 0.6: PERIPHERAL CAPABILITIES

- [ ] Update peripheral counts

- [ ] Update feature flags

### SECTION 1: REGISTER TYPE MAPPING

- [ ] Map UART type

- [ ] Map GPIO type

- [ ] Map SPI type

- [ ] Map I2C type

- [ ] Map ADC type

- [ ] Map TIMER type

- [ ] Map DMA type

- [ ] Map IRQ type

### SECTION 2: COMMON / TICK

- [ ] Define `UEF_GetTick()` function

### SECTION 3: GPIO

- [ ] Define GPIO mode values

- [ ] Implement `UEF_GPIO_CLK_ENABLE()`

- [ ] Implement `UEF_GPIO_CLK_DISABLE()`

- [ ] Define board-specific pin mappings

### SECTION 4: UART

- [ ] Map UART instances

- [ ] Define UART flags

- [ ] Define UART control bits

- [ ] Implement `UEF_UART_CLK_ENABLE()`

- [ ] Implement `UEF_UART_GET_IRQ()`

- [ ] Implement `UEF_UART_ENABLE_IRQ()`

- [ ] Implement `UEF_UART_GetClockFreq()`

- [ ] Implement `UEF_UART_CalcBRR()`

- [ ] Define UART pin maps

- [ ] Implement `UEF_UART_ConfigurePin()`

- [ ] Implement `UEF_UART_ConfigurePins()`

### SECTION 5: SPI

- [ ] Map SPI instances

- [ ] Define SPI control bits

- [ ] Define SPI flags

- [ ] Implement `UEF_SPI_CLK_ENABLE()`

- [ ] Implement `UEF_SPI_GET_IRQ()`

- [ ] Implement all SPI register helpers

- [ ] Implement `UEF_SPI_GetBaudDiv()`

- [ ] Define SPI pin maps

- [ ] Implement `UEF_SPI_ConfigurePins()`

### SECTION 6: I2C

- [ ] Map I2C instances

- [ ] Implement `UEF_I2C_CLK_ENABLE()`

- [ ] Define I2C pin maps

- [ ] Implement `UEF_I2C_ConfigurePins()`

- [ ] Implement `UEF_I2C_CalcCCR()`

- [ ] Implement `UEF_I2C_ClearErrors()`

### SECTION 7: ADC

- [ ] Map ADC instances

- [ ] Implement `UEF_ADC_CLK_ENABLE()`

- [ ] Implement `UEF_ADC_ConfigurePin()`

### SECTION 8: CLOCK TREE

- [ ] Implement `UEF_ChipLink_Clock_InitHSI()`

- [ ] Implement `UEF_ChipLink_Clock_InitHSE()`

- [ ] Implement `UEF_ChipLink_Clock_InitPLL()`

- [ ] Implement `UEF_ChipLink_Clock_GetSystemFreq()`

- [ ] Implement `UEF_ChipLink_Clock_GetHCLKFreq()`

- [ ] Implement `UEF_ChipLink_Clock_GetAPB1Freq()`

- [ ] Implement `UEF_ChipLink_Clock_GetAPB2Freq()`

- [ ] Implement `UEF_ChipLink_Clock_ConfigSysTick()`

- [ ] Implement `UEF_ChipLink_Delay_Cycles()`

### Testing

- [ ] Compilation test passes

- [ ] Clock test works

- [ ] GPIO test works (LED blinking)

- [ ] UART test works (terminal output)

- [ ] SPI test works (if applicable)

- [ ] I2C test works (if applicable)

- [ ] ADC test works (if applicable)

---

## Example: Porting to a New MCU

### Target: NXP LPC1768 (Cortex-M3)

#### 1. Directory Structure

```

UEF/Ports/NXP/LPC1768/

├── ChipLink/

│   └── ChipLink.h

└── vendor/

    ├── LPC17xx.h

    ├── system_LPC17xx.h

    ├── core_cm3.h

    └── cmsis_compiler.h

```

#### 2. Key Differences in ChipLink.h

**SECTION 0:**

```c

#define CHIPLINK_VENDOR        "NXP"

#define CHIPLINK_MCU           "LPC1768"

#define CHIPLINK_CORE          "Cortex-M3"

```

**SECTION 1: (LPC uses different type names)**

```c

typedef LPC_USART_T      UEF_UART_Reg_t;

typedef LPC_GPIO_T       UEF_GPIO_Reg_t;

typedef LPC_SSP_T        UEF_SPI_Reg_t;   // SSP = SPI

typedef LPC_I2C_T        UEF_I2C_Reg_t;

typedef LPC_ADC_T        UEF_ADC_Reg_t;

typedef LPC_TIM_T        UEF_TIM_Reg_t;

typedef IRQn_Type        UEF_IRQ_t;

```

**SECTION 3: (LPC has different GPIO registers)**

```c

#define UEF_GPIO_MODE_INPUT_VAL     0x00UL

#define UEF_GPIO_MODE_OUTPUT_VAL    0x01UL

#define UEF_GPIO_MODE_AF_VAL        0x00UL  // LPC doesn't have AF mode

#define UEF_GPIO_MODE_ANALOG_VAL    0x02UL

static inline void UEF_GPIO_CLK_ENABLE(void* port) {

    // LPC has a single PCONP register for all peripherals

    LPC_SC->PCONP |= (1UL << 15);  // GPIO

}

```

**SECTION 4: (LPC has different UART registers)**

```c

#define UEF_UART_FLAG_TXE      (1UL << 5)   // THRE

#define UEF_UART_FLAG_TC       (1UL << 6)   // TEMT

#define UEF_UART_FLAG_RXNE     (1UL << 0)   // RDR

static inline uint32_t UEF_UART_CalcBRR(uint32_t pclk, uint32_t baud) {

    // LPC uses a different baud rate calculation

    return (pclk / (16 * baud));

}

```

#### 3. Testing the Port

```c

// Simple test main.c

#include "UEF_clock.h"

#include "UEF_gpio.h"

#include "UEF_uart.h"

int main(void) {

    // Init clock (HSI equivalent on LPC)

    UEF_Clock_ConfigHSI();

    // Blink LED (GPIO)

    UEF_GPIO_Config_t led = {

        .port = (void*)LPC_GPIO1,

        .pin = 18,  // LED on LPC1768

        .mode = UEF_GPIO_MODE_OUTPUT_PP,

        .pull = UEF_GPIO_PULL_NONE,

        .speed = UEF_GPIO_SPEED_LOW

    };

    UEF_GPIO_Handle_t led_handle;

    UEF_GPIO_Init(&led_handle, &led);

    // Test UART

    UEF_UART_Handle_t uart;

    uart.instance = (void*)LPC_UART0;

    uart.instance_num = 0;

    UEF_UART_Init(&uart);

    UEF_UART_SendString(&uart, "Hello from LPC1768!\r\n", 1000);

    while(1) {

        UEF_GPIO_Toggle(&led_handle);

        UEF_DelayMs(500);

    }

}

```

---

## Troubleshooting

### Common Issues and Solutions

| Symptom | Likely Cause | Solution |

|---------|--------------|----------|

| Compilation errors | Wrong vendor include | Update includes in ChipLink.h |

| ChipLink not found | Wrong include path | Update Makefile include paths |

| No GPIO output | Clock not enabled | Check `UEF_GPIO_CLK_ENABLE()` |

| Wrong baud rate | Wrong clock frequency | Check `UEF_UART_GetClockFreq()` |

| UART sends garbage | Wrong BRR calculation | Check `UEF_UART_CalcBRR()` |

| SPI not working | Wrong pin AF | Check `UEF_SPI_ConfigurePins()` |

| I2C hangs | Missing pull-ups | Add external pull-up resistors |

| ADC reads zero | Pin not analog | Check `UEF_ADC_ConfigurePin()` |

| System freezes | Flash latency wrong | Configure flash wait states |

| Interrupts not firing | NVIC not enabled | Check `UEF_UART_ENABLE_IRQ()` |

### Debugging Tips

1\. **Use a logic analyzer** to verify pin signals

2\. **Use a terminal program** to test UART output

3\. **Add debug prints** to verify execution flow

4\. **Check register values** with a debugger

5\. **Start simple** - test one thing at a time

6\. **Compare with vendor examples** to verify register values

---

## Additional Resources

### For Specific MCU Families

| MCU Family | Where to Find Information |

|------------|---------------------------|

| STM32F4 | Reference Manual RM0368, Datasheet DS8626 |

| STM32F1 | Reference Manual RM0008, Datasheet DS5319 |

| NXP LPC17xx | User Manual UM10360, Datasheet |

| Nordic NRF52 | Product Specification v1.3 |

| Microchip SAM | Datasheet DS60001507 |

### Online Resources

- [CMSIS Documentation](https://arm-software.github.io/CMSIS_5/General/html/index.html)

- [MCU Reference Manuals](https://www.st.com/en/microcontrollers-microprocessors/stm32f4-series.html)

- [UEF Framework Documentation](https://github.com/your-repo/uef)

---

## Version History

| Version | Date | Changes |

|---------|------|---------|

| 1.0.0 | 2024-01-01 | Initial release |

---

**End of Porting Guide**

```

---

This comprehensive guide covers everything needed to port ChipLink to a new MCU. It includes:

1\. **Clear explanation** of what ChipLink is and why it matters

2\. **Step-by-step instructions** for each section

3\. **Detailed examples** for each peripheral type

4\. **Common pitfalls** to avoid

5\. **Complete checklist** for tracking progress

6\. **Real-world example** of porting to a different MCU

7\. **Troubleshooting guide** for common issues

The guide is designed to be self-contained so that anyone with basic embedded knowledge can port the framework to a new MCU.