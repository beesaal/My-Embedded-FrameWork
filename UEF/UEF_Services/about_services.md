
Services are not hardware.

They solve repeated software problems.

* * * * *

1\. Memory Manager
==================

Problem:

Every project needs:

-   buffers
-   pools
-   queues

```
services/

memory/
```

API:

```
Memory_Allocate();

Memory_Free();
```

* * * * *

2\. Ring Buffer Service
=======================

Almost every embedded project needs this.

```
services/

ringbuffer/
```

Example:

UART RX:

```
UART Interrupt

      |
      v

Ring Buffer

      |
      v

Application
```

API:

```
RingBuffer_Push();

RingBuffer_Pop();

RingBuffer_IsEmpty();
```

* * * * *

3\. Logging Service
===================

Instead of:

```
printf()
```

everywhere.

Create:

```
services/logger/
```

API:

```
LOG_INFO();

LOG_ERROR();

LOG_DEBUG();
```

Example:

```
[UART]
GPS connected

[ERROR]
I2C timeout
```

* * * * *

4\. Event System
================

Very useful for large systems.

Instead of:

```
if(sensor_ready)
{
}
```

You create events.

```
services/events/
```

Example:

Sensor:

```
EVENT_SENSOR_READY
```

Motor:

```
EVENT_MOTOR_STOPPED
```

* * * * *

5\. State Machine Framework
===========================

Almost every embedded product uses state machines.

```
services/state_machine/
```

Example:

Robot:

```
IDLE

 |

MOVE

 |

STOP

 |

ERROR
```

* * * * *

6\. Scheduler
=============

Before using RTOS.

```
services/scheduler/
```

Example:

```
Task 1
Read sensors
10ms

Task 2
Update display
100ms

Task 3
Send data
1s
```

* * * * *

7\. Configuration Manager
=========================

Very useful.

Instead of:

```
#define UART_SPEED 115200
```

everywhere.

```
services/config/
```

Stores:

```
device_config.json

or

flash memory
```

* * * * *

8\. Error Manager
=================

```
services/error/
```

Handles:

```
I2C failed

UART timeout

Sensor missing
```

* * * * *

9\. Communication Protocol Layer
================================

Very useful for IoT.

```
services/protocol/
```

Examples:

```
Packet format

Header

Length

Payload

CRC
```