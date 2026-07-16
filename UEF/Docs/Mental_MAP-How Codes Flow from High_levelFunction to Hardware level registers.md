
Think of embedded software as a **translation pipeline**.

You start with a **human intention**:

> "Turn on LED"
>
> "Send UART message"
>
> "Read temperature sensor"

and that instruction travels downward through multiple layers until it becomes:

> "Write 1 into bit 5 of GPIOA output data register."

The framework is just a way of organizing that translation.

* * * * *

The Big Picture
===============

```
Human Intent
     |
     v
Application (main.c)
     |
     v
Framework API
     |
     v
Service Layer
     |
     v
Driver Layer
     |
     v
Port Layer
     |
     v
Vendor Register Definition
     |
     v
CPU Instructions
     |
     v
Hardware Registers
     |
     v
Electrical Signal

```

Let's follow one example.

* * * * *

Example: Turn ON LED connected to PA5
=====================================

Your main.c:

```
int main()
{

    GPIO_Init(GPIOA,5,OUTPUT);

    GPIO_Write(GPIOA,5,HIGH);

    while(1);

}

```

At this point you are thinking:

> "I want pin PA5 to become HIGH."

You don't think about:

-   addresses

-   registers

-   bits

-   clock enable

-   voltage

* * * * *

Layer 1: Core Driver
====================

File:

```
drivers/gpio/gpio.c

```

Contains:

```
void GPIO_Write(GPIO_Pin pin, uint8_t state)
{
    PORT_GPIO_Write(pin,state);
}

```

The core says:

> "I don't know what GPIOA means. I will ask the MCU port."

* * * * *

Layer 2: Port Interface
=======================

File:

```
ports/stm32f411/gpio/port_gpio.c

```

Now we enter STM32-specific world.

Example:

```
void PORT_GPIO_Write(GPIO_Pin pin,uint8_t state)
{

    if(state)
    {
        pin.port->BSRR = pin.number;
    }
    else
    {
        pin.port->BSRR = pin.number << 16;
    }

}

```

Now the software knows:

"On STM32F411, GPIO is controlled by GPIO registers."

* * * * *

Layer 3: Vendor Header File
===========================

File:

```
stm32f411xe.h

```

This file came from ST.

It contains:

```
#define GPIOA ((GPIO_TypeDef *)0x40020000)

```

Meaning:

GPIOA starts at memory address:

```
0x40020000

```

It also defines the structure:

```
typedef struct
{

uint32_t MODER;
uint32_t OTYPER;
uint32_t OSPEEDR;
uint32_t PUPDR;
uint32_t IDR;
uint32_t ODR;
uint32_t BSRR;

}GPIO_TypeDef;

```

Now the compiler knows:

GPIOA contains these registers.

* * * * *

Layer 4: Register Access
========================

Your code:

```
GPIOA->BSRR = 0x20;

```

looks simple.

But the compiler translates it to:

```
Memory address:

GPIOA base
+
BSRR offset

```

Example:

```
GPIOA Base

0x40020000

BSRR offset

0x18

Final address

0x40020018

```

Now the CPU performs:

```
WRITE

Address:
0x40020018

Data:
0x00000020

```

* * * * *

Layer 5: Register Bits
======================

Inside the BSRR register:

```
Bit:

31 .....................16 15........0

Reset pins              Set pins

```

You write:

```
0x00000020

```

Binary:

```
0010 0000

```

Bit 5 becomes:

```
1

```

Meaning:

```
Set GPIOA Pin 5

```

* * * * *

Layer 6: Hardware
=================

The GPIO peripheral receives:

```
GPIOA Pin 5 = HIGH

```

The transistor inside the MCU switches.

Voltage changes:

```
0V

↓

3.3V

```

LED turns on.

* * * * *

Now the complete file journey
=============================

Your instruction:

```
GPIO_Write(GPIOA,5,HIGH);

```

travels:

```
main.c

↓

drivers/gpio/gpio.c

↓

ports/stm32f411/gpio/port_gpio.c

↓

stm32f411xe.h

↓

GPIOA register address

↓

CPU instruction

↓

Memory bus

↓

GPIO peripheral

↓

Pin voltage

```

* * * * *

Another Example: UART Send
==========================

You write:

```
UART_Send("Hello",5);

```

Flow:

```
main.c

↓

drivers/uart/uart.c

↓

port_uart.c

↓

stm32f411xe.h

↓

USART2 registers

↓

CPU writes data register

↓

UART peripheral

↓

TX pin

↓

Electrical waveform

```

* * * * *

The actual register operation:

Your port code:

```
USART2->DR = 'H';

```

becomes:

```
Memory write

Address:
0x40004404

Value:
0x48

```

Hardware:

```
USART transmitter

01001000

↓

PA2 pin

↓

UART signal

```

* * * * *

Where does Middleware fit?
==========================

Important:

Middleware usually sits **between application and drivers**, not between drivers and registers.

Example GPS:

```
main.c

↓

GPS Service

↓

UART Driver

↓

UART Port

↓

USART Register

↓

GPS module

```

The GPS service handles:

-   NMEA parsing

-   buffering

-   timeout

-   validation

The UART driver handles:

-   sending bytes

The port handles:

-   registers

* * * * *

Think of each layer as a translator
===================================

| Layer | Translates |
| --- | --- |
| main.c | Product idea → function call |
| Core driver | Generic function → hardware request |
| Service | Data/task management |
| Port | Generic request → MCU operation |
| Vendor header | Names → memory addresses |
| CPU | C code → machine instructions |
| Hardware | Bits → electrical signals |

* * * * *

The most important mental picture
=================================

Imagine a company hierarchy:

```
CEO
 |
 | "Make product move"
 |
Manager
 |
 | "Activate motor"
 |
Engineer
 |
 | "Set PWM output"
 |
Technician
 |
 | "Write register bit"
 |
Machine
 |
 | "Voltage changes"

```

The CEO does not know which screw turns.

Similarly:

```
main()

```

should never know:

```
GPIOA->ODR |= (1<<5);

```

because then your application becomes locked to STM32.

* * * * *

For your framework, remember this rule:
=======================================

**Upper layers describe WHAT should happen.**

Example:

```
Motor_SetSpeed(50);

```

**Lower layers describe HOW it happens.**

Example:

```
TIM1->CCR1 = 400;

```

The entire purpose of your architecture is to keep the "WHAT" separate from the "HOW".

That separation is what allows the same application code to run on different microcontrollers.