.section .text
.syntax unified
.thumb

.global Reset_Handler
.global main

/* Stack pointer initialization */
.word _estack
.word Reset_Handler
.word NMI_Handler
.word HardFault_Handler
.word MemManage_Handler
.word BusFault_Handler
.word UsageFault_Handler
.word 0
.word 0
.word 0
.word 0
.word SVC_Handler
.word DebugMon_Handler
.word 0
.word PendSV_Handler
.word SysTick_Handler
/* ... rest of vector table ... */

.type Reset_Handler, %function
Reset_Handler:
    /* Copy data from flash to RAM */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
    subs r2, r1
    ble .Lcopy_data_done
.Lcopy_data_loop:
    subs r2, #4
    ldr r3, [r0, r2]
    str r3, [r1, r2]
    bgt .Lcopy_data_loop
.Lcopy_data_done:

    /* Zero fill .bss section */
    ldr r0, =_sbss
    ldr r1, =_ebss
    subs r1, r0
    ble .Lzero_bss_done
    movs r2, #0
.Lzero_bss_loop:
    subs r1, #4
    str r2, [r0, r1]
    bgt .Lzero_bss_loop
.Lzero_bss_done:

    /* Call main (which will configure clock using UEF_Clock) */
    bl main

    /* Infinite loop if main returns */
.Lloop:
    b .Lloop

.size Reset_Handler, .-Reset_Handler


/* Weak handlers for all interrupts */
.macro DEFINE_WEAK_IRQ name
.weak \name
.type \name, %function
\name:
    b .
.endm

DEFINE_WEAK_IRQ NMI_Handler
DEFINE_WEAK_IRQ HardFault_Handler
DEFINE_WEAK_IRQ MemManage_Handler
DEFINE_WEAK_IRQ BusFault_Handler
DEFINE_WEAK_IRQ UsageFault_Handler
DEFINE_WEAK_IRQ SVC_Handler
DEFINE_WEAK_IRQ DebugMon_Handler
DEFINE_WEAK_IRQ PendSV_Handler
DEFINE_WEAK_IRQ SysTick_Handler
DEFINE_WEAK_IRQ WWDG_IRQHandler
DEFINE_WEAK_IRQ PVD_IRQHandler
DEFINE_WEAK_IRQ TAMP_STAMP_IRQHandler
DEFINE_WEAK_IRQ RTC_WKUP_IRQHandler
DEFINE_WEAK_IRQ FLASH_IRQHandler
DEFINE_WEAK_IRQ RCC_IRQHandler
DEFINE_WEAK_IRQ EXTI0_IRQHandler
DEFINE_WEAK_IRQ EXTI1_IRQHandler
DEFINE_WEAK_IRQ EXTI2_IRQHandler
DEFINE_WEAK_IRQ EXTI3_IRQHandler
DEFINE_WEAK_IRQ EXTI4_IRQHandler
DEFINE_WEAK_IRQ DMA1_Stream0_IRQHandler
DEFINE_WEAK_IRQ DMA1_Stream1_IRQHandler
DEFINE_WEAK_IRQ DMA1_Stream2_IRQHandler
DEFINE_WEAK_IRQ DMA1_Stream3_IRQHandler
DEFINE_WEAK_IRQ DMA1_Stream4_IRQHandler
DEFINE_WEAK_IRQ DMA1_Stream5_IRQHandler
DEFINE_WEAK_IRQ DMA1_Stream6_IRQHandler
DEFINE_WEAK_IRQ ADC_IRQHandler
DEFINE_WEAK_IRQ EXTI9_5_IRQHandler
DEFINE_WEAK_IRQ TIM1_BRK_TIM9_IRQHandler
DEFINE_WEAK_IRQ TIM1_UP_TIM10_IRQHandler
DEFINE_WEAK_IRQ TIM1_TRG_COM_TIM11_IRQHandler
DEFINE_WEAK_IRQ TIM1_CC_IRQHandler
DEFINE_WEAK_IRQ TIM2_IRQHandler
DEFINE_WEAK_IRQ TIM3_IRQHandler
DEFINE_WEAK_IRQ TIM4_IRQHandler
DEFINE_WEAK_IRQ I2C1_EV_IRQHandler
DEFINE_WEAK_IRQ I2C1_ER_IRQHandler
DEFINE_WEAK_IRQ I2C2_EV_IRQHandler
DEFINE_WEAK_IRQ I2C2_ER_IRQHandler
DEFINE_WEAK_IRQ SPI1_IRQHandler
DEFINE_WEAK_IRQ SPI2_IRQHandler
DEFINE_WEAK_IRQ USART1_IRQHandler
DEFINE_WEAK_IRQ USART2_IRQHandler
DEFINE_WEAK_IRQ EXTI15_10_IRQHandler
DEFINE_WEAK_IRQ RTC_Alarm_IRQHandler
DEFINE_WEAK_IRQ OTG_FS_WKUP_IRQHandler
DEFINE_WEAK_IRQ DMA1_Stream7_IRQHandler
DEFINE_WEAK_IRQ SDIO_IRQHandler
DEFINE_WEAK_IRQ TIM5_IRQHandler
DEFINE_WEAK_IRQ SPI3_IRQHandler
DEFINE_WEAK_IRQ DMA2_Stream0_IRQHandler
DEFINE_WEAK_IRQ DMA2_Stream1_IRQHandler
DEFINE_WEAK_IRQ DMA2_Stream2_IRQHandler
DEFINE_WEAK_IRQ DMA2_Stream3_IRQHandler
DEFINE_WEAK_IRQ DMA2_Stream4_IRQHandler
DEFINE_WEAK_IRQ OTG_FS_IRQHandler
DEFINE_WEAK_IRQ DMA2_Stream5_IRQHandler
DEFINE_WEAK_IRQ DMA2_Stream6_IRQHandler
DEFINE_WEAK_IRQ DMA2_Stream7_IRQHandler
DEFINE_WEAK_IRQ USART6_IRQHandler
DEFINE_WEAK_IRQ I2C3_EV_IRQHandler
DEFINE_WEAK_IRQ I2C3_ER_IRQHandler
DEFINE_WEAK_IRQ FPU_IRQHandler
DEFINE_WEAK_IRQ SPI4_IRQHandler
DEFINE_WEAK_IRQ SPI5_IRQHandler
