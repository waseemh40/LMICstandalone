# LMIC standalone
The project implements IBMS's LMIC API on EFM32GG microcontroller. The project is based on https://github.com/mirakonta/lmic repo however some changes are made in the original repo. in order to make lmic work on EFM32.

# Hardware used
Microcontroller: EFM32GG842F1024
Radio module: RFM95W (EU868)

# Structure of the project
LMIC requires that all the platfrom related functions are implemented in hal.c driver. The platfrom for this project is EFM32GG. The platform functions are implemented using silicons's lab emlib (which means, at the very least, this project can be used for EFM32GG family). Originally I implemeneted point to point communication between radio modules without LoRaWAN and implemented device drivers/wrapper functions for emlib. I used same wrapper functions for LMIC also, when I moved to LoRaWAN. These wrapper functions/drivers are present in inc folder. A pinmap.h file maps the ports and pins of EFM32 to desired USART and INT pins. Driver for timer (delay),RGB led (LTC3212 drive IC), UART (RS232) and SPI are implemented in this folder. NOT all of these are reuqired by lmic. Files can be omitted as per requirement. As an important point, nesting delay/timer functions call inside GPIO interrupts should be avoided. 

# Changes in original Repo.
As the lmic api only requires hal.c (maybe also debug.c?), changes are made only to hal.c driver of the original repo. The functions changed are;

1) hal_spi(). In original repo., this function uses two SPI transfers per call (USART_Tx and USART_Rx functions are used) whereas lmic expects only one SPI transfer per call in this functions. If two SPI transfers are used,the api fails to initialize as the devcie version is not received correctly. In this project, this issue was fixed.

2) hal_disableIRQs() & hal_enableIRQs(). These 2 functions are called by lmic in a nested way. In original repo. there is no 'guard' inside these functions and interrupts are enabled/disabled on every call. Also emlib documentation says em_int.h is updated by em_core.h and functions from em_core.h should be used instead of em_int.h. In this project, em_core.h is used to implement these two functions. Nested calls and enabling interrupts only on last enable are addressed by use of a flag and counter. This functionality is a must do thing for EFM32 to successfully implement lmic library.
