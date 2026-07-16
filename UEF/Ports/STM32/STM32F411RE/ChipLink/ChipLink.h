// ============================================================
// File: UEF/Ports/STM32/STM32F411RE/ChipLink.h
// ============================================================
// PURPOSE:
//   This is the ChipLink file for the STM32F411RE.
//   It is the ONLY file that is allowed to know about vendor
//   (CMSIS) register names, bit names, and pin numbers.
//
//   Every UEF_Driver (.c file) is written ONCE and must be
//   100% identical on every MCU. The only thing that changes
//   when you move to a new chip is this file.
//
//   HOW TO PORT TO A NEW MCU:
//     1. Copy this whole folder, rename it (e.g. "ESP32").
//     2. Drop the new vendor's CMSIS/SDK headers in ./vendor.
//     3. Rewrite the sections below to match the new chip.
//     4. Point the build system's include path at the new folder.
//        Nothing in UEF_Drivers/ or App/ changes.
// ============================================================

#ifndef CHIPLINK_H
#define CHIPLINK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <vendor/stm32f411xe.h>
#include <vendor/system_stm32f4xx.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// SECTION 0: CHIP IDENTITY
// ============================================================

#define CHIPLINK_VENDOR        "STMicroelectronics"
#define CHIPLINK_MCU           "STM32F411RE"
#define CHIPLINK_CORE          "Cortex-M4F"

// ============================================================
// SECTION 1: REGISTER TYPE MAPPING
// ============================================================

typedef USART_TypeDef      UEF_UART_Reg_t;
typedef GPIO_TypeDef       UEF_GPIO_Reg_t;
typedef SPI_TypeDef        UEF_SPI_Reg_t;
typedef I2C_TypeDef        UEF_I2C_Reg_t;
typedef ADC_TypeDef        UEF_ADC_Reg_t;
typedef DMA_Stream_TypeDef UEF_DMA_Stream_t;
typedef TIM_TypeDef        UEF_TIMER_Reg_t;
typedef IRQn_Type          UEF_IRQ_t;

// ============================================================
// SECTION 2: COMMON / TICK
// ============================================================


extern uint32_t UEF_Clock_GetTick(void);
static inline uint32_t UEF_GetTick(void) { return UEF_Clock_GetTick(); }

// ADD THESE FORWARD DECLARATIONS:
extern uint32_t UEF_Clock_GetAPB1Freq(void);
extern uint32_t UEF_Clock_GetAPB2Freq(void);
// ============================================================
// SECTION 3: GPIO - COMPLETE
// ============================================================

// These are the raw hardware values
#define UEF_GPIO_MODE_INPUT_VAL     0x00UL
#define UEF_GPIO_MODE_OUTPUT_VAL    0x01UL
#define UEF_GPIO_MODE_AF_VAL        0x02UL
#define UEF_GPIO_MODE_ANALOG_VAL    0x03UL

// GPIO Pull-up/Pull-down values
#define UEF_GPIO_PULL_NONE_VAL      0x00UL
#define UEF_GPIO_PULL_UP_VAL        0x01UL
#define UEF_GPIO_PULL_DOWN_VAL      0x02UL

// GPIO Speed values
#define UEF_GPIO_SPEED_LOW_VAL      0x00UL
#define UEF_GPIO_SPEED_MEDIUM_VAL   0x01UL
#define UEF_GPIO_SPEED_HIGH_VAL     0x02UL
#define UEF_GPIO_SPEED_VERY_HIGH_VAL 0x03UL


// Set GPIO pin speed
static inline void UEF_GPIO_SetSpeed(void* port, uint16_t pin, uint32_t speed) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    uint32_t shift = pin * 2;
    gpio->OSPEEDR = (gpio->OSPEEDR & ~(0x3UL << shift)) | (speed << shift);
}


// Clock control
static inline void UEF_GPIO_CLK_ENABLE(void* port) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    if (gpio == GPIOA)      RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (gpio == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (gpio == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    else if (gpio == GPIOD) RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    else if (gpio == GPIOE) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    else if (gpio == GPIOH) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
}

static inline void UEF_GPIO_CLK_DISABLE(void* port) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    if (gpio == GPIOA)      RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN;
    else if (gpio == GPIOB) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;
    else if (gpio == GPIOC) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOCEN;
    else if (gpio == GPIOD) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIODEN;
    else if (gpio == GPIOE) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOEEN;
    else if (gpio == GPIOH) RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOHEN;
}

// GPIO configuration
static inline void UEF_GPIO_SET_MODE(void* port, uint16_t pin, uint32_t mode) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    uint32_t shift = pin * 2;
    gpio->MODER = (gpio->MODER & ~(0x3UL << shift)) | (mode << shift);
}

static inline void UEF_GPIO_SET_PULL(void* port, uint16_t pin, uint32_t pull) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    uint32_t shift = pin * 2;
    gpio->PUPDR = (gpio->PUPDR & ~(0x3UL << shift)) | (pull << shift);
}

static inline void UEF_GPIO_SET_SPEED(void* port, uint16_t pin, uint32_t speed) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    uint32_t shift = pin * 2;
    gpio->OSPEEDR = (gpio->OSPEEDR & ~(0x3UL << shift)) | (speed << shift);
}

static inline void UEF_GPIO_SET_OTYPE(void* port, uint16_t pin, bool open_drain) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    if (open_drain) {
        gpio->OTYPER |= (1UL << pin);
    } else {
        gpio->OTYPER &= ~(1UL << pin);
    }
}

static inline void UEF_GPIO_SET_AF(void* port, uint16_t pin, uint8_t af) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    if (pin < 8) {
        gpio->AFR[0] = (gpio->AFR[0] & ~(0xFUL << (pin * 4))) | ((uint32_t)af << (pin * 4));
    } else {
        gpio->AFR[1] = (gpio->AFR[1] & ~(0xFUL << ((pin - 8) * 4))) | ((uint32_t)af << ((pin - 8) * 4));
    }
}

// GPIO read/write
static inline void UEF_GPIO_WRITE(void* port, uint16_t pin, bool state) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    if (state) {
        gpio->BSRR = (1UL << pin);
    } else {
        gpio->BSRR = (1UL << (pin + 16));
    }
}

static inline bool UEF_GPIO_READ(void* port, uint16_t pin) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    return (gpio->IDR & (1UL << pin)) != 0;
}

static inline void UEF_GPIO_TOGGLE(void* port, uint16_t pin) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    gpio->ODR ^= (1UL << pin);
}

static inline void UEF_GPIO_WRITE_PORT(void* port, uint32_t value) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    gpio->ODR = value;
}

static inline uint32_t UEF_GPIO_READ_PORT(void* port) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    return gpio->IDR;
}

static inline uint32_t UEF_GPIO_GetPortIndex(void* port) {
    GPIO_TypeDef* gpio = (GPIO_TypeDef*)port;
    if (gpio == GPIOA) return 0U;
    if (gpio == GPIOB) return 1U;
    if (gpio == GPIOC) return 2U;
    if (gpio == GPIOD) return 3U;
    if (gpio == GPIOE) return 4U;
    if (gpio == GPIOH) return 7U;
    return 0U;
}

// Named pins
#define UEF_PIN_LED1        GPIOA, 5
#define UEF_PIN_BUTTON      GPIOC, 13

// ============================================================
// SECTION 4: UART - COMPLETE
// ============================================================

// FLAG DEFINITIONS
// UART Status Flags - These map to hardware-specific register bits
#define UEF_UART_FLAG_TXE      USART_SR_TXE   // Transmit data register empty
#define UEF_UART_FLAG_TC       USART_SR_TC    // Transmission complete
#define UEF_UART_FLAG_RXNE     USART_SR_RXNE  // Receive data register not empty
#define UEF_UART_FLAG_IDLE     USART_SR_IDLE  // Idle line detected
#define UEF_UART_FLAG_ORE      USART_SR_ORE   // Overrun error
#define UEF_UART_FLAG_NE       USART_SR_NE    // Noise error
#define UEF_UART_FLAG_FE       USART_SR_FE    // Framing error
#define UEF_UART_FLAG_PE       USART_SR_PE    // Parity error

// UART Control Register Bits
#define UEF_UART_CR1_UE        USART_CR1_UE   // UART enable
#define UEF_UART_CR1_TE        USART_CR1_TE   // Transmitter enable
#define UEF_UART_CR1_RE        USART_CR1_RE   // Receiver enable
#define UEF_UART_CR1_TXEIE     USART_CR1_TXEIE // TX interrupt enable
#define UEF_UART_CR1_TCIE      USART_CR1_TCIE  // TC interrupt enable
#define UEF_UART_CR1_RXNEIE    USART_CR1_RXNEIE // RX interrupt enable
#define UEF_UART_CR1_PEIE      USART_CR1_PEIE   // Parity error interrupt enable
#define UEF_UART_CR1_IDLEIE    USART_CR1_IDLEIE // IDLE interrupt enable
#define UEF_UART_CR3_EIE       USART_CR3_EIE    // Error interrupt enable


// Hardware instance
static inline USART_TypeDef* UEF_UART_GET(uint8_t instance) {
    switch (instance) {
        case 1: return USART1;
        case 2: return USART2;
        case 6: return USART6;
        default: return NULL;
    }
}

// Clock control
static inline void UEF_UART_CLK_ENABLE(uint8_t instance) {
    if (instance == 1)      RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    else if (instance == 2) RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    else if (instance == 6) RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
}

static inline void UEF_UART_CLK_DISABLE(uint8_t instance) {
    if (instance == 1)      RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;
    else if (instance == 2) RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
    else if (instance == 6) RCC->APB2ENR &= ~RCC_APB2ENR_USART6EN;
}

// Interrupt control
static inline UEF_IRQ_t UEF_UART_GET_IRQ(uint8_t instance) {
    switch (instance) {
        case 1: return USART1_IRQn;
        case 2: return USART2_IRQn;
        case 6: return USART6_IRQn;
        default: return (UEF_IRQ_t)-1;
    }
}

static inline void UEF_UART_ENABLE_IRQ(uint8_t instance) {
    UEF_IRQ_t irq = UEF_UART_GET_IRQ(instance);
    if (irq != (UEF_IRQ_t)-1) NVIC_EnableIRQ(irq);
}

static inline void UEF_UART_DISABLE_IRQ(uint8_t instance) {
    UEF_IRQ_t irq = UEF_UART_GET_IRQ(instance);
    if (irq != (UEF_IRQ_t)-1) NVIC_DisableIRQ(irq);
}

// Clock frequency
static inline uint32_t UEF_UART_GetClockFreq(uint8_t instance) {
    if (instance == 2) return UEF_Clock_GetAPB1Freq();
    return UEF_Clock_GetAPB2Freq();
}

// Baud rate calculation
static inline uint32_t UEF_UART_CalcBRR(uint32_t pclk, uint32_t baud) {
    uint32_t div = (pclk * 16) / baud;
    return (div >> 4) | ((div & 0xF) << 0);
}

// ---- UART register access functions (NEW) ----

// Enable/disable
static inline void UEF_UART_ENABLE(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 |= USART_CR1_UE;
}

static inline void UEF_UART_DISABLE(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 &= ~USART_CR1_UE;
}

// Configuration
static inline void UEF_UART_SET_BAUD(void* uart, uint32_t brr) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->BRR = brr;
}

static inline void UEF_UART_SET_DATA_BITS(void* uart, uint8_t bits) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 &= ~USART_CR1_M;
    if (bits == 9) {
        u->CR1 |= USART_CR1_M;
    }
}

static inline void UEF_UART_SET_PARITY(void* uart, uint8_t parity) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 &= ~(USART_CR1_PCE | USART_CR1_PS);
    if (parity != 0) {  // Not NONE
        u->CR1 |= USART_CR1_PCE;
        if (parity == 2) {  // ODD
            u->CR1 |= USART_CR1_PS;
        }
    }
}

static inline void UEF_UART_SET_STOP_BITS(void* uart, uint8_t stop_bits) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR2 &= ~USART_CR2_STOP;
    if (stop_bits == 1) {  // 2 stop bits
        u->CR2 |= USART_CR2_STOP_1;
    }
}

static inline void UEF_UART_ENABLE_TX(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 |= USART_CR1_TE;
}

static inline void UEF_UART_DISABLE_TX(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 &= ~USART_CR1_TE;
}

static inline void UEF_UART_ENABLE_RX(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 |= USART_CR1_RE;
}

static inline void UEF_UART_DISABLE_RX(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 &= ~USART_CR1_RE;
}

// Data transfer
static inline void UEF_UART_SEND_BYTE(void* uart, uint8_t byte) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->DR = byte;
}

static inline uint8_t UEF_UART_RECV_BYTE(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    return (uint8_t)u->DR;
}

// Status flags
static inline bool UEF_UART_GET_FLAG(void* uart, uint32_t flag) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    return (u->SR & flag) != 0;
}

static inline uint32_t UEF_UART_GET_STATUS(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    return u->SR;
}

static inline uint32_t UEF_UART_GET_ERRORS(uint32_t status) {
    return status & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE);
}

static inline void UEF_UART_CLEAR_ERRORS(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    (void)u->SR;  // Read to clear
}

static inline bool UEF_UART_TX_FLAG_SET(uint32_t status) {
    return (status & USART_SR_TXE) != 0;
}

static inline bool UEF_UART_RX_FLAG_SET(uint32_t status) {
    return (status & USART_SR_RXNE) != 0;
}

static inline bool UEF_UART_TC_FLAG_SET(uint32_t status) {
    return (status & USART_SR_TC) != 0;
}

// Interrupt control
static inline void UEF_UART_ENABLE_TX_INTERRUPT(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 |= USART_CR1_TXEIE;
}

static inline void UEF_UART_DISABLE_TX_INTERRUPT(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 &= ~USART_CR1_TXEIE;
}

static inline void UEF_UART_ENABLE_RX_INTERRUPT(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 |= USART_CR1_RXNEIE;
}

static inline void UEF_UART_DISABLE_RX_INTERRUPT(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR1 &= ~USART_CR1_RXNEIE;
}

static inline void UEF_UART_ENABLE_ERROR_INTERRUPT(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR3 |= USART_CR3_EIE;
}

static inline void UEF_UART_DISABLE_ERROR_INTERRUPT(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    u->CR3 &= ~USART_CR3_EIE;
}

static inline bool UEF_UART_TX_INTERRUPT_ENABLED(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    return (u->CR1 & USART_CR1_TXEIE) != 0;
}

static inline bool UEF_UART_RX_INTERRUPT_ENABLED(void* uart) {
    USART_TypeDef* u = (USART_TypeDef*)uart;
    return (u->CR1 & USART_CR1_RXNEIE) != 0;
}

// ---- UART pin mapping ----
#define GPIO_AF7_USART1   7U
#define GPIO_AF7_USART2   7U
#define GPIO_AF8_USART6   8U

typedef struct {
    GPIO_TypeDef* port;
    uint16_t      pin;
    uint8_t       af;
    uint8_t       is_tx;
} UEF_UART_PinMap_t;

static inline const UEF_UART_PinMap_t* UEF_UART_GetPinMap(uint8_t instance, uint8_t is_tx) {
    static const UEF_UART_PinMap_t usart1_tx = { GPIOA, 9,  GPIO_AF7_USART1, 1 };
    static const UEF_UART_PinMap_t usart1_rx = { GPIOA, 10, GPIO_AF7_USART1, 0 };
    static const UEF_UART_PinMap_t usart2_tx = { GPIOA, 2,  GPIO_AF7_USART2, 1 };
    static const UEF_UART_PinMap_t usart2_rx = { GPIOA, 3,  GPIO_AF7_USART2, 0 };
    static const UEF_UART_PinMap_t usart6_tx = { GPIOC, 6,  GPIO_AF8_USART6, 1 };
    static const UEF_UART_PinMap_t usart6_rx = { GPIOC, 7,  GPIO_AF8_USART6, 0 };

    switch (instance) {
        case 1: return is_tx ? &usart1_tx : &usart1_rx;
        case 2: return is_tx ? &usart2_tx : &usart2_rx;
        case 6: return is_tx ? &usart6_tx : &usart6_rx;
        default: return NULL;
    }
}

static inline int UEF_UART_ConfigurePin(const UEF_UART_PinMap_t* p) {
    if (!p || !p->port) return -1;
    UEF_GPIO_CLK_ENABLE(p->port);
    UEF_GPIO_SET_MODE(p->port, p->pin, UEF_GPIO_MODE_AF_VAL);
    UEF_GPIO_SET_OTYPE(p->port, p->pin, false);
    UEF_GPIO_SET_SPEED(p->port, p->pin, 2);
    if (!p->is_tx) {
        UEF_GPIO_SET_PULL(p->port, p->pin, 1);  // Pull-up on RX
    } else {
        UEF_GPIO_SET_PULL(p->port, p->pin, 0);  // No pull on TX
    }
    UEF_GPIO_SET_AF(p->port, p->pin, p->af);
    return 0;
}

static inline int UEF_UART_ConfigurePins(uint8_t instance) {
    const UEF_UART_PinMap_t* tx = UEF_UART_GetPinMap(instance, 1);
    const UEF_UART_PinMap_t* rx = UEF_UART_GetPinMap(instance, 0);
    if (!tx || !rx) return -1;
    if (UEF_UART_ConfigurePin(tx) != 0) return -1;
    if (UEF_UART_ConfigurePin(rx) != 0) return -1;
    return 0;
}

// ============================================================
// SECTION 5: SPI - COMPLETE
// ============================================================

// FLAG DEFINITIONS

// SPI Status Flags - These map to hardware-specific register bits
#define UEF_SPI_FLAG_TXE      SPI_SR_TXE      // Transmit buffer empty
#define UEF_SPI_FLAG_RXNE     SPI_SR_RXNE     // Receive buffer not empty
#define UEF_SPI_FLAG_BSY      SPI_SR_BSY      // Busy flag
#define UEF_SPI_FLAG_OVR      SPI_SR_OVR      // Overrun flag
#define UEF_SPI_FLAG_MODF     SPI_SR_MODF     // Mode fault
#define UEF_SPI_FLAG_CRCERR   SPI_SR_CRCERR   // CRC error
#define UEF_SPI_FLAG_FRE      SPI_SR_FRE      // Frame format error

// SPI Control Register 1 Bits
#define UEF_SPI_CR1_MSTR      SPI_CR1_MSTR    // Master selection
#define UEF_SPI_CR1_CPOL      SPI_CR1_CPOL    // Clock polarity
#define UEF_SPI_CR1_CPHA      SPI_CR1_CPHA    // Clock phase
#define UEF_SPI_CR1_LSBFIRST  SPI_CR1_LSBFIRST // LSB first
#define UEF_SPI_CR1_DFF       SPI_CR1_DFF     // Data frame format (16-bit)
#define UEF_SPI_CR1_SSM       SPI_CR1_SSM     // Software slave management
#define UEF_SPI_CR1_SSI       SPI_CR1_SSI     // Internal slave select
#define UEF_SPI_CR1_SPE       SPI_CR1_SPE     // SPI enable
#define UEF_SPI_CR1_BR        SPI_CR1_BR      // Baud rate control
#define UEF_SPI_CR1_BR_Pos    SPI_CR1_BR_Pos  // Baud rate bit position

// SPI Control Register 2 Bits
#define UEF_SPI_CR2_TXEIE     SPI_CR2_TXEIE   // TX interrupt enable
#define UEF_SPI_CR2_RXNEIE    SPI_CR2_RXNEIE  // RX interrupt enable
#define UEF_SPI_CR2_ERRIE     SPI_CR2_ERRIE   // Error interrupt enable


// Hardware instance
static inline SPI_TypeDef* UEF_SPI_GET(uint8_t instance) {
    switch (instance) {
        case 1: return SPI1;
        case 2: return SPI2;
        case 3: return SPI3;
        case 4: return SPI4;
        case 5: return SPI5;
        default: return NULL;
    }
}

// Clock control
static inline void UEF_SPI_CLK_ENABLE(uint8_t instance) {
    switch (instance) {
        case 1: RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; break;
        case 2: RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; break;
        case 3: RCC->APB1ENR |= RCC_APB1ENR_SPI3EN; break;
        case 4: RCC->APB2ENR |= RCC_APB2ENR_SPI4EN; break;
        case 5: RCC->APB2ENR |= RCC_APB2ENR_SPI5EN; break;
        default: break;
    }
}

static inline void UEF_SPI_CLK_DISABLE(uint8_t instance) {
    switch (instance) {
        case 1: RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN; break;
        case 2: RCC->APB1ENR &= ~RCC_APB1ENR_SPI2EN; break;
        case 3: RCC->APB1ENR &= ~RCC_APB1ENR_SPI3EN; break;
        case 4: RCC->APB2ENR &= ~RCC_APB2ENR_SPI4EN; break;
        case 5: RCC->APB2ENR &= ~RCC_APB2ENR_SPI5EN; break;
        default: break;
    }
}

// Interrupt control
static inline UEF_IRQ_t UEF_SPI_GET_IRQ(uint8_t instance) {
    switch (instance) {
        case 1: return SPI1_IRQn;
        case 2: return SPI2_IRQn;
        case 3: return SPI3_IRQn;
        case 4: return SPI4_IRQn;
        case 5: return SPI5_IRQn;
        default: return (UEF_IRQ_t)-1;
    }
}

static inline void UEF_SPI_ENABLE_IRQ(uint8_t instance) {
    UEF_IRQ_t i = UEF_SPI_GET_IRQ(instance);
    if (i != (UEF_IRQ_t)-1) NVIC_EnableIRQ(i);
}

static inline void UEF_SPI_DISABLE_IRQ(uint8_t instance) {
    UEF_IRQ_t i = UEF_SPI_GET_IRQ(instance);
    if (i != (UEF_IRQ_t)-1) NVIC_DisableIRQ(i);
}

// ---- SPI register access functions (NEW) ----

// Enable/disable
static inline void UEF_SPI_ENABLE(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR1 |= SPI_CR1_SPE;
}

static inline void UEF_SPI_DISABLE(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR1 &= ~SPI_CR1_SPE;
}

// Configuration
static inline void UEF_SPI_SET_MASTER(void* spi, bool master) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    if (master) {
        s->CR1 |= SPI_CR1_MSTR;
        s->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;
    } else {
        s->CR1 &= ~SPI_CR1_MSTR;
    }
}

static inline void UEF_SPI_SET_MODE(void* spi, uint8_t mode) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA);
    if (mode == 1 || mode == 3) s->CR1 |= SPI_CR1_CPHA;
    if (mode == 2 || mode == 3) s->CR1 |= SPI_CR1_CPOL;
}

static inline void UEF_SPI_SET_BAUD_DIV(void* spi, uint32_t div) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR1 = (s->CR1 & ~SPI_CR1_BR) | (div << SPI_CR1_BR_Pos);
}

static inline void UEF_SPI_SET_DATA_WIDTH(void* spi, bool is_16bit) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    if (is_16bit) {
        s->CR1 |= SPI_CR1_DFF;
    } else {
        s->CR1 &= ~SPI_CR1_DFF;
    }
}

static inline void UEF_SPI_SET_LSB_FIRST(void* spi, bool lsb_first) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    if (lsb_first) {
        s->CR1 |= SPI_CR1_LSBFIRST;
    } else {
        s->CR1 &= ~SPI_CR1_LSBFIRST;
    }
}

// Data transfer
static inline void UEF_SPI_WRITE_BYTE(void* spi, uint8_t byte) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    *(volatile uint8_t*)&s->DR = byte;
}

static inline uint8_t UEF_SPI_READ_BYTE(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    return *(volatile uint8_t*)&s->DR;
}

static inline void UEF_SPI_WRITE_WORD(void* spi, uint16_t word) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->DR = word;
}

static inline uint16_t UEF_SPI_READ_WORD(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    return (uint16_t)s->DR;
}

// Status flags
static inline bool UEF_SPI_GET_FLAG(void* spi, uint32_t flag) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    return (s->SR & flag) != 0;
}

static inline bool UEF_SPI_IS_BUSY(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    return (s->SR & SPI_SR_BSY) != 0;
}

static inline uint32_t UEF_SPI_GET_STATUS(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    return s->SR;
}

static inline uint32_t UEF_SPI_GET_ERRORS(uint32_t status) {
    return status & (SPI_SR_OVR | SPI_SR_MODF | SPI_SR_CRCERR);
}

static inline void UEF_SPI_CLEAR_ERRORS(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    (void)s->SR;
    (void)s->DR;
}

static inline bool UEF_SPI_TX_FLAG_SET(uint32_t status) {
    return (status & SPI_SR_TXE) != 0;
}

static inline bool UEF_SPI_RX_FLAG_SET(uint32_t status) {
    return (status & SPI_SR_RXNE) != 0;
}

// Interrupt control
static inline void UEF_SPI_ENABLE_TX_INTERRUPT(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR2 |= SPI_CR2_TXEIE;
}

static inline void UEF_SPI_DISABLE_TX_INTERRUPT(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR2 &= ~SPI_CR2_TXEIE;
}

static inline void UEF_SPI_ENABLE_RX_INTERRUPT(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR2 |= SPI_CR2_RXNEIE;
}

static inline void UEF_SPI_DISABLE_RX_INTERRUPT(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR2 &= ~SPI_CR2_RXNEIE;
}

static inline void UEF_SPI_ENABLE_ERROR_INTERRUPT(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR2 |= SPI_CR2_ERRIE;
}

static inline void UEF_SPI_DISABLE_ERROR_INTERRUPT(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR2 &= ~SPI_CR2_ERRIE;
}

static inline bool UEF_SPI_TX_INTERRUPT_ENABLED(uint32_t cr2) {
    return (cr2 & SPI_CR2_TXEIE) != 0;
}

static inline bool UEF_SPI_RX_INTERRUPT_ENABLED(uint32_t cr2) {
    return (cr2 & SPI_CR2_RXNEIE) != 0;
}

static inline bool UEF_SPI_ERROR_INTERRUPT_ENABLED(uint32_t cr2) {
    return (cr2 & SPI_CR2_ERRIE) != 0;
}

static inline void UEF_SPI_WRITE_CR2(void* spi, uint32_t value) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR2 = value;
}

static inline uint32_t UEF_SPI_READ_CR2(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    return s->CR2;
}

static inline void UEF_SPI_ENABLE_INTERRUPTS(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR2 |= (SPI_CR2_TXEIE | SPI_CR2_RXNEIE | SPI_CR2_ERRIE);
}

static inline void UEF_SPI_DISABLE_INTERRUPTS(void* spi) {
    SPI_TypeDef* s = (SPI_TypeDef*)spi;
    s->CR2 &= ~(SPI_CR2_TXEIE | SPI_CR2_RXNEIE | SPI_CR2_ERRIE);
}


// SPI pin mapping
#define GPIO_AF5_SPI1 5U

typedef struct {
    GPIO_TypeDef* port;
    uint16_t      pin;
    uint8_t       af;
} UEF_SPI_PinMap_t;

static inline int UEF_SPI_ConfigurePins(uint8_t instance) {
    static const UEF_SPI_PinMap_t spi1_pins[3] = {
        { GPIOA, 5, GPIO_AF5_SPI1 },
        { GPIOA, 6, GPIO_AF5_SPI1 },
        { GPIOA, 7, GPIO_AF5_SPI1 },
    };

    const UEF_SPI_PinMap_t* pins;
    size_t count;

    switch (instance) {
        case 1: pins = spi1_pins; count = 3; break;
        default: return -1;
    }

    for (size_t i = 0; i < count; i++) {
        UEF_GPIO_CLK_ENABLE(pins[i].port);
        UEF_GPIO_SET_MODE(pins[i].port, pins[i].pin, UEF_GPIO_MODE_AF_VAL);
        UEF_GPIO_SET_OTYPE(pins[i].port, pins[i].pin, false);
        UEF_GPIO_SET_SPEED(pins[i].port, pins[i].pin, 3);
        UEF_GPIO_SET_PULL(pins[i].port, pins[i].pin, 0);
        UEF_GPIO_SET_AF(pins[i].port, pins[i].pin, pins[i].af);
    }
    return 0;
}

// ============================================================
// SECTION 6: I2C - COMPLETE
// ============================================================

//Flag definitions

// I2C Status Flags - These map to hardware-specific register bits
#define UEF_I2C_FLAG_SB       I2C_SR1_SB      // Start bit sent (master)
#define UEF_I2C_FLAG_ADDR     I2C_SR1_ADDR    // Address sent/matched
#define UEF_I2C_FLAG_BTF      I2C_SR1_BTF     // Byte transfer finished
#define UEF_I2C_FLAG_TXE      I2C_SR1_TXE     // Transmit data register empty
#define UEF_I2C_FLAG_RXNE     I2C_SR1_RXNE    // Receive data register not empty
#define UEF_I2C_FLAG_STOPF    I2C_SR1_STOPF   // Stop condition detected (slave)
#define UEF_I2C_FLAG_BERR     I2C_SR1_BERR    // Bus error
#define UEF_I2C_FLAG_ARLO     I2C_SR1_ARLO    // Arbitration lost
#define UEF_I2C_FLAG_AF       I2C_SR1_AF      // Acknowledge failure
#define UEF_I2C_FLAG_OVR      I2C_SR1_OVR     // Overrun/Underrun
#define UEF_I2C_FLAG_PECERR   I2C_SR1_PECERR  // PEC error
#define UEF_I2C_FLAG_TIMEOUT  I2C_SR1_TIMEOUT // Timeout error
#define UEF_I2C_FLAG_SMBALERT I2C_SR1_SMBALERT // SMBus alert

static inline I2C_TypeDef* UEF_I2C_GET(uint8_t instance) {
    switch (instance) {
        case 1: return I2C1;
        case 2: return I2C2;
        case 3: return I2C3;
        default: return NULL;
    }
}

static inline void UEF_I2C_CLK_ENABLE(uint8_t instance) {
    switch (instance) {
        case 1: RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; break;
        case 2: RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; break;
        case 3: RCC->APB1ENR |= RCC_APB1ENR_I2C3EN; break;
        default: break;
    }
}

static inline void UEF_I2C_CLK_DISABLE(uint8_t instance) {
    switch (instance) {
        case 1: RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN; break;
        case 2: RCC->APB1ENR &= ~RCC_APB1ENR_I2C2EN; break;
        case 3: RCC->APB1ENR &= ~RCC_APB1ENR_I2C3EN; break;
        default: break;
    }
}

// ---- I2C register access functions ----

// Enable/disable
static inline void UEF_I2C_ENABLE(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CR1 |= I2C_CR1_PE;
}

static inline void UEF_I2C_DISABLE(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CR1 &= ~I2C_CR1_PE;
}

// Configuration
static inline void UEF_I2C_SET_FREQ(void* i2c, uint32_t freq_mhz) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CR2 = (i->CR2 & ~I2C_CR2_FREQ) | (freq_mhz & 0x3F);
}

// ONLY ONE definition of UEF_I2C_CalcCCR
static inline uint32_t UEF_I2C_CalcCCR(uint32_t pclk_hz, uint32_t speed_hz) {
    uint32_t ccr;
    if (speed_hz <= 100000U) {
        ccr = pclk_hz / (speed_hz * 2U);
        if (ccr < 4U) ccr = 4U;
    } else {
        ccr = pclk_hz / (speed_hz * 3U);
        ccr |= I2C_CCR_DUTY;
        if ((ccr & ~I2C_CCR_DUTY) < 1U) ccr |= 1U;
    }
    return ccr;
}

// ONLY ONE definition of UEF_I2C_SET_SPEED
static inline void UEF_I2C_SET_SPEED(void* i2c, uint32_t pclk, uint32_t speed_hz) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CCR = UEF_I2C_CalcCCR(pclk, speed_hz);
}

static inline void UEF_I2C_SET_RISE_TIME(void* i2c, uint32_t rise_time) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->TRISE = rise_time;
}

static inline void UEF_I2C_SET_OWN_ADDRESS(void* i2c, uint8_t address) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->OAR1 = (1UL << 14) | ((uint32_t)address << 1);
}

// Bus operations
static inline void UEF_I2C_GENERATE_START(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CR1 |= I2C_CR1_START;
}

static inline void UEF_I2C_GENERATE_STOP(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CR1 |= I2C_CR1_STOP;
}

static inline void UEF_I2C_ENABLE_ACK(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CR1 |= I2C_CR1_ACK;
}

static inline void UEF_I2C_DISABLE_ACK(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CR1 &= ~I2C_CR1_ACK;
}

// Data transfer
static inline void UEF_I2C_SEND_BYTE(void* i2c, uint8_t byte) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->DR = byte;
}

static inline uint8_t UEF_I2C_RECV_BYTE(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    return (uint8_t)i->DR;
}

// Status flags
static inline bool UEF_I2C_GET_FLAG(void* i2c, uint32_t flag) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    return (i->SR1 & flag) != 0;
}

static inline uint32_t UEF_I2C_GET_STATUS(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    return i->SR1;
}

static inline uint32_t UEF_I2C_GET_ERRORS(uint32_t status) {
    return status & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR);
}

static inline void UEF_I2C_CLEAR_ERRORS(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    uint32_t sr1 = i->SR1;
    (void)i->SR2;
    if (sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR)) {
        i->SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR);
    }
}

static inline void UEF_I2C_CLEAR_ADDR(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    (void)i->SR1;
    (void)i->SR2;
}

static inline bool UEF_I2C_TX_FLAG_SET(uint32_t status) {
    return (status & I2C_SR1_TXE) != 0;
}

static inline bool UEF_I2C_RX_FLAG_SET(uint32_t status) {
    return (status & I2C_SR1_RXNE) != 0;
}

// Hardware-specific read handling (STM32 errata)
static inline void UEF_I2C_SETUP_SINGLE_BYTE_READ(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CR1 &= ~I2C_CR1_ACK;
    (void)i->SR1;
    (void)i->SR2;
    i->CR1 |= I2C_CR1_STOP;
}

static inline void UEF_I2C_CLEAR_ADDR_WITH_ACK(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CR1 |= I2C_CR1_ACK;
    (void)i->SR1;
    (void)i->SR2;
}

static inline void UEF_I2C_SETUP_LAST_BYTE_READ(void* i2c) {
    I2C_TypeDef* i = (I2C_TypeDef*)i2c;
    i->CR1 |= I2C_CR1_STOP;
}

// I2C pin mapping - KEEP ONLY ONE COPY
typedef struct {
    GPIO_TypeDef* port;
    uint16_t      scl_pin;
    uint16_t      sda_pin;
    uint8_t       af;
} UEF_I2C_PinMap_t;

// KEEP ONLY ONE COPY of UEF_I2C_ConfigurePins
static inline int UEF_I2C_ConfigurePins(uint8_t instance) {
    GPIO_TypeDef* gpio;
    uint16_t scl, sda;
    uint8_t af = 4;

    switch (instance) {
        case 1: gpio = GPIOB; scl = 6;  sda = 7;  break;
        case 2: gpio = GPIOB; scl = 10; sda = 11; break;
        case 3:
            UEF_GPIO_CLK_ENABLE(GPIOA);
            UEF_GPIO_CLK_ENABLE(GPIOC);
            UEF_GPIO_SET_MODE(GPIOA, 8, UEF_GPIO_MODE_AF_VAL);
            UEF_GPIO_SET_OTYPE(GPIOA, 8, true);
            UEF_GPIO_SET_SPEED(GPIOA, 8, 2);
            UEF_GPIO_SET_PULL(GPIOA, 8, 1);
            UEF_GPIO_SET_AF(GPIOA, 8, af);
            UEF_GPIO_SET_MODE(GPIOC, 9, UEF_GPIO_MODE_AF_VAL);
            UEF_GPIO_SET_OTYPE(GPIOC, 9, true);
            UEF_GPIO_SET_SPEED(GPIOC, 9, 2);
            UEF_GPIO_SET_PULL(GPIOC, 9, 1);
            UEF_GPIO_SET_AF(GPIOC, 9, af);
            return 0;
        default: return -1;
    }

    UEF_GPIO_CLK_ENABLE(gpio);
    UEF_GPIO_SET_MODE(gpio, scl, UEF_GPIO_MODE_AF_VAL);
    UEF_GPIO_SET_OTYPE(gpio, scl, true);
    UEF_GPIO_SET_SPEED(gpio, scl, 2);
    UEF_GPIO_SET_PULL(gpio, scl, 1);
    UEF_GPIO_SET_AF(gpio, scl, af);
    UEF_GPIO_SET_MODE(gpio, sda, UEF_GPIO_MODE_AF_VAL);
    UEF_GPIO_SET_OTYPE(gpio, sda, true);
    UEF_GPIO_SET_SPEED(gpio, sda, 2);
    UEF_GPIO_SET_PULL(gpio, sda, 1);
    UEF_GPIO_SET_AF(gpio, sda, af);
    return 0;
}
// ============================================================
// SECTION 7: ADC - COMPLETE
// ============================================================

// ADC Status Flags
#define UEF_ADC_FLAG_EOC       ADC_SR_EOC      // End of conversion
#define UEF_ADC_FLAG_JEOC      ADC_SR_JEOC     // Injected end of conversion
#define UEF_ADC_FLAG_OVR       ADC_SR_OVR      // Overrun
#define UEF_ADC_FLAG_AWD       ADC_SR_AWD      // Analog watchdog

// ADC Control Register Bits
#define UEF_ADC_CR1_EOCIE      ADC_CR1_EOCIE   // EOC interrupt enable
#define UEF_ADC_CR1_AWDIE      ADC_CR1_AWDIE   // Analog watchdog interrupt
#define UEF_ADC_CR1_JEOCIE     ADC_CR1_JEOCIE  // JEOC interrupt enable
#define UEF_ADC_CR1_SCAN       ADC_CR1_SCAN    // Scan mode
#define UEF_ADC_CR1_RES_Pos    ADC_CR1_RES_Pos // Resolution bit position

#define UEF_ADC_CR2_ADON       ADC_CR2_ADON    // ADC enable
#define UEF_ADC_CR2_CONT       ADC_CR2_CONT    // Continuous conversion
#define UEF_ADC_CR2_DMA        ADC_CR2_DMA     // DMA enable
#define UEF_ADC_CR2_ALIGN      ADC_CR2_ALIGN   // Data alignment
#define UEF_ADC_CR2_SWSTART    ADC_CR2_SWSTART // Start conversion


static inline ADC_TypeDef* UEF_ADC_GET(uint8_t instance) {
    if (instance == 1) return ADC1;
    return NULL;
}

static inline void UEF_ADC_CLK_ENABLE(uint8_t instance) {
    (void)instance;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
}

static inline void UEF_ADC_CLK_DISABLE(uint8_t instance) {
    (void)instance;
    RCC->APB2ENR &= ~RCC_APB2ENR_ADC1EN;
}

static inline void UEF_ADC_ConfigurePin(void* port, uint16_t pin) {
    UEF_GPIO_CLK_ENABLE(port);
    UEF_GPIO_SET_MODE(port, pin, UEF_GPIO_MODE_ANALOG_VAL);
    UEF_GPIO_SET_PULL(port, pin, 0);
}

static inline void UEF_ADC_SET_RESOLUTION(void* adc, uint32_t resolution) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    a->CR1 = (a->CR1 & ~ADC_CR1_RES) | (resolution << ADC_CR1_RES_Pos);
}

static inline void UEF_ADC_SET_ALIGN(void* adc, bool left_align) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    if (left_align) {
        a->CR2 |= ADC_CR2_ALIGN;
    } else {
        a->CR2 &= ~ADC_CR2_ALIGN;
    }
}

static inline void UEF_ADC_SET_CHANNEL(void* adc, uint8_t channel) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    a->SQR3 = channel;
    a->SQR1 = 0;
}

static inline void UEF_ADC_SET_SAMPLE_TIME(void* adc, uint8_t channel, uint32_t sample_time) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    if (channel <= 9) {
        uint32_t shift = channel * 3;
        a->SMPR2 = (a->SMPR2 & ~(0x7UL << shift)) | (sample_time << shift);
    } else {
        uint32_t shift = (channel - 10) * 3;
        a->SMPR1 = (a->SMPR1 & ~(0x7UL << shift)) | (sample_time << shift);
    }
}

static inline void UEF_ADC_ENABLE(void* adc) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    a->CR2 |= ADC_CR2_ADON;
}

static inline void UEF_ADC_DISABLE(void* adc) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    a->CR2 &= ~ADC_CR2_ADON;
}

static inline void UEF_ADC_START_CONVERSION(void* adc) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    a->CR2 |= ADC_CR2_SWSTART;
}

static inline bool UEF_ADC_CONVERSION_DONE(void* adc) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    return (a->SR & ADC_SR_EOC) != 0;
}

static inline uint32_t UEF_ADC_GET_VALUE(void* adc) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    return a->DR & 0xFFFF;
}

static inline void UEF_ADC_ENABLE_INTERRUPT(void* adc) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    a->CR1 |= ADC_CR1_EOCIE;
}

static inline void UEF_ADC_DISABLE_INTERRUPT(void* adc) {
    ADC_TypeDef* a = (ADC_TypeDef*)adc;
    a->CR1 &= ~ADC_CR1_EOCIE;
}

// ============================================================
// SECTION 8: CLOCK TREE - COMPLETE
// ============================================================

static inline int UEF_ChipLink_Clock_InitHSI(void) {
    uint32_t timeout = 10000;
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY)) {
        if (--timeout == 0) return -1;
    }
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY) { /* wait */ }
    FLASH->ACR = FLASH_ACR_LATENCY_0WS;
    RCC->CFGR = (RCC->CFGR & ~0x3UL) | RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) { /* wait */ }
    return 0;
}

static inline int UEF_ChipLink_Clock_InitHSE(void) {
    uint32_t timeout = 10000;
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)) {
        if (--timeout == 0) return -1;
    }
    FLASH->ACR = FLASH_ACR_LATENCY_0WS;
    RCC->CFGR = (RCC->CFGR & ~0x3UL) | RCC_CFGR_SW_HSE;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE) { /* wait */ }
    return 0;
}

static inline int UEF_ChipLink_Clock_InitPLL(uint32_t hse_freq) {
    (void)hse_freq;
    uint32_t timeout = 10000;
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY)) { /* wait */ }
    uint32_t pllm = 16, plln = 400, pllp = 4, pllq = 4;
    RCC->PLLCFGR = pllm | (plln << 6) | (((pllp >> 1) - 1) << 16) |
                   RCC_PLLCFGR_PLLSRC_HSI | (pllq << 24);
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {
        if (--timeout == 0) return -1;
    }
    FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN;
    RCC->CFGR = (RCC->CFGR & ~0xFFFFUL) | RCC_CFGR_HPRE_DIV1 |
                RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2;
    RCC->CFGR = (RCC->CFGR & ~0x3UL) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { /* wait */ }
    return 0;
}

static inline uint32_t UEF_ChipLink_Clock_GetSystemFreq(void) {
    uint32_t sws = RCC->CFGR & RCC_CFGR_SWS;
    if (sws == RCC_CFGR_SWS_HSI) return 16000000U;
    if (sws == RCC_CFGR_SWS_HSE) return 8000000U;
    if (sws == RCC_CFGR_SWS_PLL) {
        uint32_t pllm = RCC->PLLCFGR & 0x3FU;
        uint32_t plln = (RCC->PLLCFGR >> 6) & 0x1FFU;
        uint32_t pllp_bits = (RCC->PLLCFGR >> 16) & 0x3U;
        uint32_t pllp = (pllp_bits == 0) ? 2 : (pllp_bits == 1) ? 4 : (pllp_bits == 2) ? 6 : 8;
        uint32_t pllsrc = (RCC->PLLCFGR >> 22) & 1U;
        uint32_t vco_in = pllsrc ? 8000000U : 16000000U;
        return (vco_in / pllm) * plln / pllp;
    }
    return 0;
}

static inline uint32_t UEF_ChipLink_Clock_GetHCLKFreq(void) {
    uint32_t sysclk = UEF_ChipLink_Clock_GetSystemFreq();
    uint32_t hpre = (RCC->CFGR >> 4) & 0xFU;
    static const uint16_t div_lut[16] = {1,1,1,1,1,1,1,1,2,4,8,16,64,128,256,512};
    return sysclk / div_lut[hpre];
}

static inline uint32_t UEF_ChipLink_Clock_GetAPB1Freq(void) {
    uint32_t hclk = UEF_ChipLink_Clock_GetHCLKFreq();
    uint32_t ppre1 = (RCC->CFGR >> 10) & 0x7U;
    static const uint8_t div_lut[8] = {1,1,1,1,2,4,8,16};
    return hclk / div_lut[ppre1];
}

static inline uint32_t UEF_ChipLink_Clock_GetAPB2Freq(void) {
    uint32_t hclk = UEF_ChipLink_Clock_GetHCLKFreq();
    uint32_t ppre2 = (RCC->CFGR >> 13) & 0x7U;
    static const uint8_t div_lut[8] = {1,1,1,1,2,4,8,16};
    return hclk / div_lut[ppre2];
}

static inline int UEF_ChipLink_Clock_ConfigSysTick(uint32_t sysclk, uint32_t ticks_per_second) {
    uint32_t reload = sysclk / ticks_per_second - 1U;
    if (reload > 0xFFFFFFUL) return -1;
    SysTick->CTRL = 0;
    SysTick->LOAD = reload;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    NVIC_SetPriority(SysTick_IRQn, 0);
    return 0;
}

static inline void UEF_ChipLink_Delay_Cycles(uint32_t cycles) {
    for (volatile uint32_t i = 0; i < cycles; i++) { __NOP(); }
}

// ============================================================
// SECTION 9: EXTERNAL INTERRUPT (EXTI)
// ============================================================

static inline void UEF_EXTI_CLK_ENABLE(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
}

static inline void UEF_EXTI_SelectPort(uint16_t pin, uint32_t port_index) {
    uint32_t reg  = pin / 4U;
    uint32_t pos  = (pin % 4U) * 4U;
    SYSCFG->EXTICR[reg] = (SYSCFG->EXTICR[reg] & ~(0xFUL << pos)) | (port_index << pos);
}

static inline void UEF_EXTI_SetTrigger(uint16_t pin, bool rising, bool falling) {
    uint32_t mask = (1UL << pin);
    if (rising)  EXTI->RTSR |= mask;  else EXTI->RTSR &= ~mask;
    if (falling) EXTI->FTSR |= mask;  else EXTI->FTSR &= ~mask;
}

static inline void UEF_EXTI_EnableLine(uint16_t pin) {
    EXTI->IMR |= (1UL << pin);
}

static inline void UEF_EXTI_DisableLine(uint16_t pin) {
    EXTI->IMR &= ~(1UL << pin);
}

static inline bool UEF_EXTI_IsPending(uint16_t pin) {
    return (EXTI->PR & (1UL << pin)) != 0;
}

static inline void UEF_EXTI_ClearPending(uint16_t pin) {
    EXTI->PR = (1UL << pin);
}

static inline UEF_IRQ_t UEF_EXTI_GET_IRQ(uint16_t pin) {
    switch (pin) {
        case 0: return EXTI0_IRQn;
        case 1: return EXTI1_IRQn;
        case 2: return EXTI2_IRQn;
        case 3: return EXTI3_IRQn;
        case 4: return EXTI4_IRQn;
        default:
            if (pin >= 5 && pin <= 9) return EXTI9_5_IRQn;
            if (pin >= 10 && pin <= 15) return EXTI15_10_IRQn;
            return (UEF_IRQ_t)-1;
    }
}

static inline void UEF_EXTI_ENABLE_IRQ(uint16_t pin) {
    UEF_IRQ_t i = UEF_EXTI_GET_IRQ(pin);
    if (i != (UEF_IRQ_t)-1) NVIC_EnableIRQ(i);
}

static inline void UEF_EXTI_DISABLE_IRQ(uint16_t pin) {
    UEF_IRQ_t i = UEF_EXTI_GET_IRQ(pin);
    if (i != (UEF_IRQ_t)-1) NVIC_DisableIRQ(i);
}

static inline void UEF_EXTI_ConfigurePinAsInput(void* port, uint16_t pin, uint32_t pupdr_bits) {
    UEF_GPIO_CLK_ENABLE(port);
    UEF_GPIO_SET_MODE(port, pin, UEF_GPIO_MODE_INPUT_VAL);
    UEF_GPIO_SET_PULL(port, pin, pupdr_bits);
}

// ============================================================
// SECTION 10: GENERAL PURPOSE TIMER
// ============================================================

static inline UEF_TIMER_Reg_t* UEF_TIMER_GET(uint8_t instance) {
    switch (instance) {
        case 2:  return TIM2;
        case 3:  return TIM3;
        case 4:  return TIM4;
        case 5:  return TIM5;
        case 9:  return TIM9;
        case 10: return TIM10;
        case 11: return TIM11;
        default: return NULL;
    }
}

static inline void UEF_TIMER_CLK_ENABLE(uint8_t instance) {
    switch (instance) {
        case 2:  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;  break;
        case 3:  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;  break;
        case 4:  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;  break;
        case 5:  RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;  break;
        case 9:  RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;  break;
        case 10: RCC->APB2ENR |= RCC_APB2ENR_TIM10EN; break;
        case 11: RCC->APB2ENR |= RCC_APB2ENR_TIM11EN; break;
        default: break;
    }
}

static inline void UEF_TIMER_CLK_DISABLE(uint8_t instance) {
    switch (instance) {
        case 2:  RCC->APB1ENR &= ~RCC_APB1ENR_TIM2EN;  break;
        case 3:  RCC->APB1ENR &= ~RCC_APB1ENR_TIM3EN;  break;
        case 4:  RCC->APB1ENR &= ~RCC_APB1ENR_TIM4EN;  break;
        case 5:  RCC->APB1ENR &= ~RCC_APB1ENR_TIM5EN;  break;
        case 9:  RCC->APB2ENR &= ~RCC_APB2ENR_TIM9EN;  break;
        case 10: RCC->APB2ENR &= ~RCC_APB2ENR_TIM10EN; break;
        case 11: RCC->APB2ENR &= ~RCC_APB2ENR_TIM11EN; break;
        default: break;
    }
}

static inline UEF_IRQ_t UEF_TIMER_GET_IRQ(uint8_t instance) {
    switch (instance) {
        case 2:  return TIM2_IRQn;
        case 3:  return TIM3_IRQn;
        case 4:  return TIM4_IRQn;
        case 5:  return TIM5_IRQn;
        case 9:  return TIM1_BRK_TIM9_IRQn;
        case 10: return TIM1_UP_TIM10_IRQn;
        case 11: return TIM1_TRG_COM_TIM11_IRQn;
        default: return (UEF_IRQ_t)-1;
    }
}

static inline void UEF_TIMER_ENABLE_IRQ(uint8_t instance) {
    UEF_IRQ_t i = UEF_TIMER_GET_IRQ(instance);
    if (i != (UEF_IRQ_t)-1) NVIC_EnableIRQ(i);
}

static inline void UEF_TIMER_DISABLE_IRQ(uint8_t instance) {
    UEF_IRQ_t i = UEF_TIMER_GET_IRQ(instance);
    if (i != (UEF_IRQ_t)-1) NVIC_DisableIRQ(i);
}

static inline uint32_t UEF_TIMER_GetClockFreq(uint8_t instance) {
    if (instance == 9 || instance == 10 || instance == 11) {
        return UEF_ChipLink_Clock_GetAPB2Freq() * 2U;
    }
    return UEF_ChipLink_Clock_GetAPB1Freq() * 2U;
}

// Timer register access
static inline void UEF_TIMER_SET_PRESCALER(void* tim, uint32_t psc) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->PSC = psc;
}

static inline void UEF_TIMER_SET_PERIOD(void* tim, uint32_t arr) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->ARR = arr;
}

static inline uint32_t UEF_TIMER_GET_PERIOD(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    return t->ARR;
}

static inline void UEF_TIMER_SET_COUNTER(void* tim, uint32_t cnt) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->CNT = cnt;
}

static inline uint32_t UEF_TIMER_GET_COUNTER(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    return t->CNT;
}

static inline void UEF_TIMER_START(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->CR1 |= TIM_CR1_CEN;
}

static inline void UEF_TIMER_STOP(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->CR1 &= ~TIM_CR1_CEN;
}

static inline void UEF_TIMER_ENABLE_UPDATE_INTERRUPT(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->DIER |= TIM_DIER_UIE;
}

static inline void UEF_TIMER_DISABLE_UPDATE_INTERRUPT(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->DIER &= ~TIM_DIER_UIE;
}

static inline bool UEF_TIMER_IS_UPDATE_FLAG_SET(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    return (t->SR & TIM_SR_UIF) != 0;
}

static inline void UEF_TIMER_CLEAR_UPDATE_FLAG(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->SR = ~TIM_SR_UIF;
}

static inline void UEF_TIMER_GENERATE_UPDATE(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->EGR |= TIM_EGR_UG;
}

static inline void UEF_TIMER_ENABLE_ARR_PRELOAD(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->CR1 |= TIM_CR1_ARPE;
}

// PWM support
static inline void UEF_TIMER_SET_PWM_MODE(void* tim, uint8_t channel) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    const uint32_t pwm1 = 0x6UL;
    switch (channel) {
        case 1: t->CCMR1 = (t->CCMR1 & ~(0x7UL << 4)) | (pwm1 << 4) | TIM_CCMR1_OC1PE; break;
        case 2: t->CCMR1 = (t->CCMR1 & ~(0x7UL << 12)) | (pwm1 << 12) | TIM_CCMR1_OC2PE; break;
        case 3: t->CCMR2 = (t->CCMR2 & ~(0x7UL << 4)) | (pwm1 << 4) | TIM_CCMR2_OC3PE; break;
        case 4: t->CCMR2 = (t->CCMR2 & ~(0x7UL << 12)) | (pwm1 << 12) | TIM_CCMR2_OC4PE; break;
        default: break;
    }
}

static inline void UEF_TIMER_SET_COMPARE(void* tim, uint8_t channel, uint32_t value) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    switch (channel) {
        case 1: t->CCR1 = value; break;
        case 2: t->CCR2 = value; break;
        case 3: t->CCR3 = value; break;
        case 4: t->CCR4 = value; break;
        default: break;
    }
}

static inline void UEF_TIMER_ENABLE_CHANNEL_OUTPUT(void* tim, uint8_t channel) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    switch (channel) {
        case 1: t->CCER |= TIM_CCER_CC1E; break;
        case 2: t->CCER |= TIM_CCER_CC2E; break;
        case 3: t->CCER |= TIM_CCER_CC3E; break;
        case 4: t->CCER |= TIM_CCER_CC4E; break;
        default: break;
    }
}

static inline void UEF_TIMER_ENABLE_MAIN_OUTPUT(void* tim) {
    TIM_TypeDef* t = (TIM_TypeDef*)tim;
    t->BDTR |= TIM_BDTR_MOE;
}

// PWM pin mapping
typedef struct {
    GPIO_TypeDef* port;
    uint16_t      pin;
    uint8_t       af;
    uint8_t       channel;
} UEF_PWM_PinMap_t;

#define GPIO_AF2_TIM3 2U

static inline const UEF_PWM_PinMap_t* UEF_PWM_GetPinMap(uint8_t instance, uint8_t channel, size_t* count_out) {
    static const UEF_PWM_PinMap_t tim3_ch1 = { GPIOB, 4, GPIO_AF2_TIM3, 1 };
    static const UEF_PWM_PinMap_t tim3_ch2 = { GPIOB, 5, GPIO_AF2_TIM3, 2 };
    if (instance == 3 && channel == 1) { *count_out = 1; return &tim3_ch1; }
    if (instance == 3 && channel == 2) { *count_out = 1; return &tim3_ch2; }
    *count_out = 0;
    return NULL;
}

static inline int UEF_PWM_ConfigurePin(uint8_t instance, uint8_t channel) {
    size_t count = 0;
    const UEF_PWM_PinMap_t* map = UEF_PWM_GetPinMap(instance, channel, &count);
    if (!map || count == 0) return -1;
    UEF_GPIO_CLK_ENABLE(map->port);
    UEF_GPIO_SET_MODE(map->port, map->pin, UEF_GPIO_MODE_AF_VAL);
    UEF_GPIO_SET_OTYPE(map->port, map->pin, false);
    UEF_GPIO_SET_SPEED(map->port, map->pin, 2);
    UEF_GPIO_SET_PULL(map->port, map->pin, 0);
    UEF_GPIO_SET_AF(map->port, map->pin, map->af);
    return 0;
}

// ============================================================
// SECTION 11: WATCHDOG (IWDG)
// ============================================================

#define UEF_IWDG_KEY_RELOAD   0xAAAAU
#define UEF_IWDG_KEY_UNLOCK   0x5555U
#define UEF_IWDG_KEY_START    0xCCCCU

static inline void UEF_IWDG_Unlock(void) {
    IWDG->KR = UEF_IWDG_KEY_UNLOCK;
}

static inline void UEF_IWDG_Start(void) {
    IWDG->KR = UEF_IWDG_KEY_START;
}

static inline void UEF_IWDG_Refresh(void) {
    IWDG->KR = UEF_IWDG_KEY_RELOAD;
}

static inline void UEF_IWDG_SetPrescaler(uint32_t psc_bits) {
    IWDG->PR = psc_bits;
}

static inline void UEF_IWDG_SetReload(uint32_t reload) {
    IWDG->RLR = reload & 0xFFFU;
}

static inline bool UEF_IWDG_IsPrescalerBusy(void) {
    return (IWDG->SR & IWDG_SR_PVU) != 0;
}

static inline bool UEF_IWDG_IsReloadBusy(void) {
    return (IWDG->SR & IWDG_SR_RVU) != 0;
}

static inline void UEF_IWDG_CalcTiming(uint32_t timeout_ms, uint32_t* psc_bits_out, uint32_t* reload_out) {
    const uint32_t lsi_hz = 32000U;
    static const uint16_t dividers[7] = {4, 8, 16, 32, 64, 128, 256};
    for (uint32_t bits = 0; bits < 7; bits++) {
        uint32_t tick_hz = lsi_hz / dividers[bits];
        uint32_t reload = (timeout_ms * tick_hz) / 1000U;
        if (reload <= 0xFFFU) {
            *psc_bits_out = bits;
            *reload_out = reload;
            return;
        }
    }
    *psc_bits_out = 6;
    *reload_out = 0xFFFU;
}

// ============================================================
// SECTION 12: RTC
// ============================================================

static inline void UEF_RTC_ClockDomainUnlock(void) {
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_DBP;
}

static inline int UEF_RTC_SelectAndEnableClock(void) {
    UEF_RTC_ClockDomainUnlock();
    RCC->CSR |= RCC_CSR_LSION;
    uint32_t timeout = 100000;
    while (!(RCC->CSR & RCC_CSR_LSIRDY)) {
        if (--timeout == 0) return -1;
    }
    if (!(RCC->BDCR & RCC_BDCR_RTCEN)) {
        RCC->BDCR = (RCC->BDCR & ~RCC_BDCR_RTCSEL) | RCC_BDCR_RTCSEL_1;
        RCC->BDCR |= RCC_BDCR_RTCEN;
    }
    return 0;
}

static inline void UEF_RTC_Unlock(void) {
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
}

static inline void UEF_RTC_Lock(void) {
    RTC->WPR = 0xFF;
}

static inline int UEF_RTC_EnterInitMode(void) {
    RTC->ISR |= RTC_ISR_INIT;
    uint32_t timeout = 100000;
    while (!(RTC->ISR & RTC_ISR_INITF)) {
        if (--timeout == 0) return -1;
    }
    return 0;
}

static inline void UEF_RTC_ExitInitMode(void) {
    RTC->ISR &= ~RTC_ISR_INIT;
}

static inline void UEF_RTC_SetPrescaler(uint32_t async_val, uint32_t sync_val) {
    RTC->PRER = ((async_val & 0x7FUL) << 16) | (sync_val & 0x7FFFUL);
}

static inline void UEF_RTC_WriteTR(uint32_t bcd_time) {
    RTC->TR = bcd_time;
}

static inline void UEF_RTC_WriteDR(uint32_t bcd_date) {
    RTC->DR = bcd_date;
}

static inline uint32_t UEF_RTC_ReadTR(void) {
    return RTC->TR;
}

static inline uint32_t UEF_RTC_ReadDR(void) {
    return RTC->DR;
}

static inline int UEF_RTC_WaitForSync(void) {
    RTC->ISR &= ~RTC_ISR_RSF;
    uint32_t timeout = 200000;
    while (!(RTC->ISR & RTC_ISR_RSF)) {
        if (--timeout == 0) return -1;
    }
    return 0;
}

// ============================================================
// SECTION 13: DMA
// ============================================================

typedef struct {
    DMA_TypeDef* dma;
    UEF_DMA_Stream_t* stream;
    uint8_t stream_index;
} UEF_DMA_Handle_t;

static inline int UEF_DMA_GetStream(uint8_t dma_num, uint8_t stream_num, UEF_DMA_Handle_t* out) {
    if (stream_num > 7) return -1;
    static DMA_Stream_TypeDef* const dma1_streams[8] = {
        DMA1_Stream0, DMA1_Stream1, DMA1_Stream2, DMA1_Stream3,
        DMA1_Stream4, DMA1_Stream5, DMA1_Stream6, DMA1_Stream7
    };
    static DMA_Stream_TypeDef* const dma2_streams[8] = {
        DMA2_Stream0, DMA2_Stream1, DMA2_Stream2, DMA2_Stream3,
        DMA2_Stream4, DMA2_Stream5, DMA2_Stream6, DMA2_Stream7
    };
    if (dma_num == 1) {
        out->dma = DMA1;
        out->stream = dma1_streams[stream_num];
    } else if (dma_num == 2) {
        out->dma = DMA2;
        out->stream = dma2_streams[stream_num];
    } else {
        return -1;
    }
    out->stream_index = stream_num;
    return 0;
}

static inline void UEF_DMA_CLK_ENABLE(uint8_t dma_num) {
    if (dma_num == 1) RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    else if (dma_num == 2) RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
}

static inline void UEF_DMA_ConfigureStream(UEF_DMA_Handle_t* h, uint8_t channel,
                                            uint32_t periph_addr, uint32_t mem_addr,
                                            uint16_t item_count, uint8_t direction) {
    UEF_DMA_Stream_t* s = h->stream;
    s->CR = 0;
    s->CR = ((uint32_t)channel << 25);
    s->CR |= ((uint32_t)direction << 6);
    s->CR |= (1UL << 10);
    s->PAR = periph_addr;
    s->M0AR = mem_addr;
    s->NDTR = item_count;
}

static inline void UEF_DMA_SetDataWidth(UEF_DMA_Handle_t* h, uint8_t bytes) {
    uint32_t size_bits = (bytes == 1) ? 0x0UL : (bytes == 2) ? 0x1UL : 0x2UL;
    h->stream->CR = (h->stream->CR & ~((0x3UL << 11) | (0x3UL << 13))) |
                    (size_bits << 11) | (size_bits << 13);
}

static inline void UEF_DMA_EnableTCInterrupt(UEF_DMA_Handle_t* h) {
    h->stream->CR |= (1UL << 4);
}

static inline void UEF_DMA_Enable(UEF_DMA_Handle_t* h) {
    h->stream->CR |= (1UL << 0);
}

static inline void UEF_DMA_Disable(UEF_DMA_Handle_t* h) {
    h->stream->CR &= ~(1UL << 0);
}

static inline bool UEF_DMA_IsEnabled(UEF_DMA_Handle_t* h) {
    return (h->stream->CR & 0x1UL) != 0;
}

static inline uint16_t UEF_DMA_GetRemaining(UEF_DMA_Handle_t* h) {
    return (uint16_t)h->stream->NDTR;
}

static inline bool UEF_DMA_IsTransferComplete(UEF_DMA_Handle_t* h) {
    static const uint8_t tc_bit[8] = {5, 11, 21, 27, 5, 11, 21, 27};
    volatile uint32_t* isr = (h->stream_index < 4) ? &h->dma->LISR : &h->dma->HISR;
    return (*isr & (1UL << tc_bit[h->stream_index])) != 0;
}

static inline void UEF_DMA_ClearTransferComplete(UEF_DMA_Handle_t* h) {
    static const uint8_t tc_bit[8] = {5, 11, 21, 27, 5, 11, 21, 27};
    volatile uint32_t* ifcr = (h->stream_index < 4) ? &h->dma->LIFCR : &h->dma->HIFCR;
    *ifcr = (1UL << tc_bit[h->stream_index]);
}

static inline UEF_IRQ_t UEF_DMA_GET_IRQ(uint8_t dma_num, uint8_t stream_num) {
    static const IRQn_Type dma1_irq[8] = {
        DMA1_Stream0_IRQn, DMA1_Stream1_IRQn, DMA1_Stream2_IRQn, DMA1_Stream3_IRQn,
        DMA1_Stream4_IRQn, DMA1_Stream5_IRQn, DMA1_Stream6_IRQn, DMA1_Stream7_IRQn
    };
    static const IRQn_Type dma2_irq[8] = {
        DMA2_Stream0_IRQn, DMA2_Stream1_IRQn, DMA2_Stream2_IRQn, DMA2_Stream3_IRQn,
        DMA2_Stream4_IRQn, DMA2_Stream5_IRQn, DMA2_Stream6_IRQn, DMA2_Stream7_IRQn
    };
    if (stream_num > 7) return (UEF_IRQ_t)-1;
    return (dma_num == 1) ? dma1_irq[stream_num] :
           (dma_num == 2) ? dma2_irq[stream_num] : (UEF_IRQ_t)-1;
}

// ============================================================
// SECTION 14: CRITICAL SECTION
// ============================================================

static inline void UEF_EnterCritical(void) {
    __disable_irq();
}

static inline void UEF_ExitCritical(void) {
    __enable_irq();
}

#ifdef __cplusplus
}
#endif

#endif // CHIPLINK_H