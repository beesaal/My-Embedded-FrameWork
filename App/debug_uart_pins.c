// ============================================================
// File: App/debug_uart_pins.c
// PURPOSE: Debug UART pin configuration on STM32F411RE
//          This file uses direct register access to verify
//          that the UART pins are correctly configured.
// ============================================================

#include "UEF_clock.h"
#include "UEF_uart.h"
#include "UEF_gpio.h"
#include "ChipLink.h"
#include <string.h>
#include <stdio.h>

// ============================================================
// HANDLES
// ============================================================

static UEF_UART_Handle_t uart_handle;
static UEF_GPIO_Handle_t led_handle;

// ============================================================
// DELAY
// ============================================================

static void delay_ms(uint32_t ms) {
    UEF_DelayMs(ms);
}

// ============================================================
// DIRECT UART SEND (bypass printf)
// ============================================================

static void uart_send_string(const char* str) {
    if (uart_handle.is_initialized) {
        UEF_UART_SendString(&uart_handle, str);
    }
}

static void uart_send_bytes(const uint8_t* data, uint32_t len) {
    if (uart_handle.is_initialized) {
        UEF_UART_SendBytes(&uart_handle, data, len);
    }
}

// ============================================================
// PRINT REGISTER VALUES
// ============================================================

static void print_register_status(void) {
    char buffer[128];
    
    // Read GPIOA registers
    uint32_t moder = GPIOA->MODER;
    uint32_t otyper = GPIOA->OTYPER;
    uint32_t ospeedr = GPIOA->OSPEEDR;
    uint32_t pupdr = GPIOA->PUPDR;
    uint32_t afrl = GPIOA->AFR[0];
    uint32_t afrh = GPIOA->AFR[1];
    uint32_t idr = GPIOA->IDR;
    uint32_t odr = GPIOA->ODR;
    
    // Print register values
    uart_send_string("\r\n=== GPIOA REGISTER DUMP ===\r\n");
    
    snprintf(buffer, sizeof(buffer), "MODER:   0x%08lX\r\n", (unsigned long)moder);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "OTYPER:  0x%08lX\r\n", (unsigned long)otyper);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "OSPEEDR: 0x%08lX\r\n", (unsigned long)ospeedr);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "PUPDR:   0x%08lX\r\n", (unsigned long)pupdr);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "AFRL:    0x%08lX\r\n", (unsigned long)afrl);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "AFRH:    0x%08lX\r\n", (unsigned long)afrh);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "IDR:     0x%08lX\r\n", (unsigned long)idr);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "ODR:     0x%08lX\r\n", (unsigned long)odr);
    uart_send_string(buffer);
    
    // Decode PA2 and PA3 settings
    uart_send_string("\r\n=== PIN DECODE ===\r\n");
    
    // PA2 (pin 2) - USART2 TX
    uint32_t pa2_mode = (moder >> (2 * 2)) & 0x3;
    uint32_t pa2_af = (afrl >> (2 * 4)) & 0xF;
    bool pa2_correct = (pa2_mode == 0x2 && pa2_af == 0x7);
    snprintf(buffer, sizeof(buffer), "PA2 - MODE: %lu, AF: %lu %s\r\n", 
             (unsigned long)pa2_mode, (unsigned long)pa2_af,
             pa2_correct ? "✅ CORRECT (AF7, USART2 TX)" : "❌ INCORRECT");
    uart_send_string(buffer);
    
    // PA3 (pin 3) - USART2 RX
    uint32_t pa3_mode = (moder >> (3 * 2)) & 0x3;
    uint32_t pa3_af = (afrl >> (3 * 4)) & 0xF;
    bool pa3_correct = (pa3_mode == 0x2 && pa3_af == 0x7);
    snprintf(buffer, sizeof(buffer), "PA3 - MODE: %lu, AF: %lu %s\r\n", 
             (unsigned long)pa3_mode, (unsigned long)pa3_af,
             pa3_correct ? "✅ CORRECT (AF7, USART2 RX)" : "❌ INCORRECT");
    uart_send_string(buffer);
    
    // Check USART2 clock
    uint32_t apb1enr = RCC->APB1ENR;
    uart_send_string("\r\n=== CLOCK STATUS ===\r\n");
    snprintf(buffer, sizeof(buffer), "RCC->APB1ENR: 0x%08lX\r\n", (unsigned long)apb1enr);
    uart_send_string(buffer);
    
    const char* usart2_en = (apb1enr & RCC_APB1ENR_USART2EN) ? "✅ ENABLED" : "❌ DISABLED";
    snprintf(buffer, sizeof(buffer), "USART2 EN: %s\r\n", usart2_en);
    uart_send_string(buffer);
    
    // Check USART2 registers
    uint32_t cr1 = USART2->CR1;
    uint32_t cr2 = USART2->CR2;
    uint32_t cr3 = USART2->CR3;
    uint32_t brr = USART2->BRR;
    uint32_t sr = USART2->SR;
    
    uart_send_string("\r\n=== USART2 STATUS ===\r\n");
    
    const char* ue = (cr1 & USART_CR1_UE) ? "✅ ENABLED" : "❌ DISABLED";
    snprintf(buffer, sizeof(buffer), "CR1: 0x%08lX %s\r\n", (unsigned long)cr1, ue);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "CR2: 0x%08lX\r\n", (unsigned long)cr2);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "CR3: 0x%08lX\r\n", (unsigned long)cr3);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "BRR: 0x%08lX (should be ~0x8B for 115200@16MHz)\r\n", (unsigned long)brr);
    uart_send_string(buffer);
    
    snprintf(buffer, sizeof(buffer), "SR:  0x%08lX\r\n", (unsigned long)sr);
    uart_send_string(buffer);
    
    const char* txe = (sr & USART_SR_TXE) ? "✅ Ready" : "❌ Not ready";
    snprintf(buffer, sizeof(buffer), "  TXE: %s\r\n", txe);
    uart_send_string(buffer);
    
    const char* tc = (sr & USART_SR_TC) ? "✅ Complete" : "❌ In progress";
    snprintf(buffer, sizeof(buffer), "  TC:  %s\r\n", tc);
    uart_send_string(buffer);
    
    const char* rxne = (sr & USART_SR_RXNE) ? "✅ Data waiting" : "❌ No data";
    snprintf(buffer, sizeof(buffer), "  RXNE: %s\r\n", rxne);
    uart_send_string(buffer);
}

// ============================================================
// MANUAL PIN CONFIGURATION (Direct register access)
// ============================================================

static void manual_pin_config(void) {
    // Enable GPIOA clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    
    // PA2 - TX (AF7)
    GPIOA->MODER &= ~(0x3UL << (2 * 2));   // Clear bits for PA2
    GPIOA->MODER |= (0x2UL << (2 * 2));    // Set to AF mode
    GPIOA->OTYPER &= ~(1UL << 2);          // Push-pull
    GPIOA->OSPEEDR |= (0x2UL << (2 * 2));  // High speed
    GPIOA->PUPDR &= ~(0x3UL << (2 * 2));   // No pull
    GPIOA->AFR[0] &= ~(0xFUL << (2 * 4));  // Clear AF for PA2
    GPIOA->AFR[0] |= (0x7UL << (2 * 4));   // AF7 for USART2
    
    // PA3 - RX (AF7)
    GPIOA->MODER &= ~(0x3UL << (3 * 2));   // Clear bits for PA3
    GPIOA->MODER |= (0x2UL << (3 * 2));    // Set to AF mode
    GPIOA->OTYPER &= ~(1UL << 3);          // Push-pull
    GPIOA->OSPEEDR |= (0x2UL << (3 * 2));  // High speed
    GPIOA->PUPDR &= ~(0x3UL << (3 * 2));   // Clear pull
    GPIOA->PUPDR |= (0x1UL << (3 * 2));    // Pull-up on RX
    GPIOA->AFR[0] &= ~(0xFUL << (3 * 4));  // Clear AF for PA3
    GPIOA->AFR[0] |= (0x7UL << (3 * 4));   // AF7 for USART2
}

// ============================================================
// UART INIT WITH MANUAL CONFIG
// ============================================================

static int uart_init_manual(void) {
    // Step 1: Configure pins manually
    manual_pin_config();
    
    // Step 2: Enable USART2 clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    
    // Step 3: Configure USART2
    USART2->CR1 &= ~USART_CR1_UE;  // Disable UART
    
    // Set baud rate for 16MHz / 115200
    USART2->BRR = (16000000 + (115200 / 2)) / 115200;  // ~0x8B
    
    // 8N1 configuration
    USART2->CR1 &= ~(USART_CR1_M | USART_CR1_PCE);  // 8 data bits, no parity
    USART2->CR2 &= ~USART_CR2_STOP;  // 1 stop bit
    
    // Enable TX, RX, and UART
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
    
    // Wait for UART to be ready
    for (volatile int i = 0; i < 1000; i++);
    
    return 0;
}

// ============================================================
// MAIN
// ============================================================

int main(void) {
    // STEP 1: Configure system clock
    UEF_Clock_ConfigHSI();
    
    // STEP 2: Initialize LED
    UEF_GPIO_Config_t led_config = {
        .port = GPIOA,
        .pin = 5,
        .mode = UEF_GPIO_MODE_OUTPUT_PP,
        .pull = UEF_GPIO_PULL_NONE,
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    UEF_GPIO_Init(&led_handle, &led_config);
    
    // STEP 3: Blink LED to show we're alive
    for (int i = 0; i < 5; i++) {
        UEF_GPIO_Set(&led_handle);
        delay_ms(200);
        UEF_GPIO_Reset(&led_handle);
        delay_ms(200);
    }
    UEF_GPIO_Set(&led_handle);  // LED ON = running
    
    // STEP 4: Try UART init - first attempt with manual config
    uart_handle.is_initialized = false;
    int init_result = uart_init_manual();
    
    if (init_result == 0) {
        uart_handle.is_initialized = true;
        uart_handle.instance_num = 2;
    }
    
    // STEP 5: Send initial message (with delay for terminal connection)
    delay_ms(500);
    
    uart_send_string("\r\n");
    uart_send_string("========================================\r\n");
    uart_send_string("UART PIN DEBUG v1.0\r\n");
    uart_send_string("========================================\r\n");
    
    if (init_result == 0) {
        uart_send_string("UART initialized successfully!\r\n");
    } else {
        uart_send_string("UART initialization FAILED!\r\n");
    }
    
    // STEP 6: Print pin status
    print_register_status();
    
    // STEP 7: Send test message
    uart_send_string("\r\n=== TEST MESSAGE ===\r\n");
    uart_send_string("If you can read this, UART is working!\r\n");
    uart_send_string("Type '1' to turn LED ON, '0' to turn LED OFF\r\n");
    uart_send_string("Type 'd' to dump registers again\r\n");
    uart_send_string("========================================\r\n\r\n");
    
    // STEP 8: Main loop - echo and LED control
    uint32_t counter = 0;
    uint8_t rx_byte;
    
    while (1) {
        counter++;
        if (counter >= 10000) {
            counter = 0;
            UEF_GPIO_Toggle(&led_handle);
            uart_send_string("Alive!\r\n");
        }
        
        // Check for incoming data
        if (UEF_UART_ReceiveByte(&uart_handle, &rx_byte) == 0) {
            // Echo back
            UEF_UART_SendByte(&uart_handle, rx_byte);
            
            // Handle commands
            if (rx_byte == '1') {
                UEF_GPIO_Set(&led_handle);
                uart_send_string("\r\nLED ON\r\n");
            } else if (rx_byte == '0') {
                UEF_GPIO_Reset(&led_handle);
                uart_send_string("\r\nLED OFF\r\n");
            } else if (rx_byte == 'd' || rx_byte == 'D') {
                print_register_status();
            } else if (rx_byte == 'r' || rx_byte == 'R') {
                uart_send_string("\r\nResetting MCU...\r\n");
                delay_ms(100);
                NVIC_SystemReset();
            } else if (rx_byte == '?' || rx_byte == 'h') {
                uart_send_string("\r\nCommands:\r\n");
                uart_send_string("  '1' - LED ON\r\n");
                uart_send_string("  '0' - LED OFF\r\n");
                uart_send_string("  'd' - Dump registers\r\n");
                uart_send_string("  'r' - Reset MCU\r\n");
                uart_send_string("  '?' - Show help\r\n");
                uart_send_string("\r\n");
            }
        }
        
        delay_ms(1);
    }
}