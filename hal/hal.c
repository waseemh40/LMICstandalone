/*******************************************************************************
 * Copyright (c) 2014-2015 IBM Corporation.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    IBM Zurich Research Lab - initial API, implementation and documentation
 *******************************************************************************/
#include "../lmic/hal.h"
#include "../lmic/lmic.h"
#include "../inc/pinmap.h"
#include "../inc/rgb_led.h"
#include "../inc/spi.h"
#include "em_cmu.h"
#include "em_rtc.h"
#include "em_int.h"
#include "em_core.h"
#include "em_device.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "debug.h"


// HAL state
static struct
{
    //int irqlevel;
    uint64_t ticks;
} HAL;

// -----------------------------------------------------------------------------
// I/O
extern void radio_irq_handler(u1_t dio);
int gpio_int_flag=0;
void GPIO_EVEN_IRQHandler()	//impar
 {
	u4_t i = GPIO_IntGetEnabled();
	GPIO_IntClear(i);
	//debug_str("\tEven IRQ\n");
	if (i & 1<<RADIO_IO_0){
		radio_irq_handler(0);
		//debug_str("\tEven IRQ 0\n");
		gpio_int_flag=2;
	}
	else if (i & 1<<RADIO_IO_2){
		radio_irq_handler(2);
		//debug_str("\tEven IRQ 2\n");
		gpio_int_flag=2;
	}
	//debug_str("\tEven IRQ END\n");
	return;
 }

void GPIO_ODD_IRQHandler()	//par
 {
	u4_t i = GPIO_IntGetEnabled();
	GPIO_IntClear(i);
	//debug_str("\tODD IRQ\n");
	if (i & 1<<RADIO_IO_1){
		radio_irq_handler(1);
		//debug_str("\tODD IRQ 1");
		gpio_int_flag=1;
	}
	//debug_str("\tODD IRQ End\n");
	return;
 }
static void hal_io_init ()
{

	GPIO_PinModeSet(PWR_EN_PORT,RADIO_PWR_EN,gpioModePushPull,0);
	GPIO_PinOutSet(PWR_EN_PORT,RADIO_PWR_EN);
	spi_cs_set(radio);

	GPIO_PinModeSet(RADIO_IO_0345_PORT, RADIO_IO_0, gpioModeInput, 0);	//DIO0=PayLoadReady
	GPIO_PinModeSet(RADIO_IO_12_PORT, RADIO_IO_1, gpioModeInput, 0);	//DIO1=FifoLevel
	GPIO_PinModeSet(RADIO_IO_12_PORT, RADIO_IO_2, gpioModeInput, 0);	//DIO2=SyncAddr
	GPIO_PinModeSet(RADIO_IO_0345_PORT, RADIO_IO_3, gpioModeInput, 0);	//DIO3=FifoEmpty
	GPIO_PinModeSet(RADIO_IO_0345_PORT, RADIO_IO_4, gpioModeInput, 0);	//DIO4=PreambleDetect/RSSI
	GPIO_PinModeSet(RADIO_IO_0345_PORT, RADIO_IO_5, gpioModeInput, 0);	//DIO5=ModeReady

	GPIO_IntConfig(RADIO_IO_0345_PORT, RADIO_IO_0, true, false, true);
	GPIO_IntConfig(RADIO_IO_12_PORT, RADIO_IO_1, true, false, true);
	GPIO_IntConfig(RADIO_IO_12_PORT, RADIO_IO_2, true, false, true);

	GPIO_IntClear(_GPIO_IF_MASK);

	NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);

	NVIC_EnableIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);
	//debug_str("Here shit\n");
}

// val ==1  => tx 1, rx 0 ; val == 0 => tx 0, rx 1
void hal_pin_rxtx (u1_t val)
{
	//not used in PA52 nor PA53
}


// set radio NSS pin to given value
void hal_pin_nss (u1_t val)
{
	if (val)
		spi_cs_set(radio);
	else
		spi_cs_clear(radio);
}

// set radio RST pin to given value (or keep floating!)
void hal_pin_rst (u1_t val)
{
	//NVIC_SystemReset();
	//delay_ms(7);
	return;
}
#define NUM_DIO 5
static bool dio_states[NUM_DIO] = {0};

static void hal_io_check() {
	//debug_str("hal_io_chck_called\n");
	dio_states[0]=GPIO_PinInGet(RADIO_IO_0345_PORT,RADIO_IO_0);
	if(dio_states[0]==true){
		debug_str("IRQ_0\n");
		radio_irq_handler(0);
		delay_ms(10);
		while(1);

	}
	dio_states[1]=GPIO_PinInGet(RADIO_IO_12_PORT,RADIO_IO_1);
	if(dio_states[1]==true){
		debug_str("IRQ_1\n");
		radio_irq_handler(1);
	}
	dio_states[2]=GPIO_PinInGet(RADIO_IO_12_PORT,RADIO_IO_2);
	if(dio_states[2]==true){
		debug_str("IRQ_2\n");
		radio_irq_handler(2);
	}
	return;

}


static void hal_spi_init ()
{

	spi_init();
	spi_enable();
}

// perform SPI transaction with radio
u1_t hal_spi (u1_t out)
{
	u1_t	ret_val=0;
	//char	my_str[10];
	ret_val=spi_read_write_byte(out);
	//USART_Tx(SPI_USART, out);
	//ret_val=USART_Rx(SPI_USART);;
	//spi_write_byte(out);
	//ret_val=spi_read_byte();
	//sprintf(my_str,"S=%2x\tR=%2x\n",out,ret_val);
	//debug_str(my_str);
	return ret_val;
	/* For every byte sent, one is received */
	//USART_Tx(SPI_USART, out);
	//return USART_Rx(SPI_USART);
}


// -----------------------------------------------------------------------------
// TIME
static uint8_t       rtcInitialized = 0;    /**< 1 if rtc is initialized */
static uint32_t      rtcFreq;               /**< RTC Frequence. 32.768 kHz */

/***************************************************************************//**
 * @brief RTC Interrupt Handler, invoke callback function if defined.
 ******************************************************************************/
void RTC_IRQHandler(void)
{
	//debug_str("\tRTC IRQ");
	if (RTC_IntGet() & RTC_IF_OF)
	{
		HAL.ticks ++;
	}

    if(RTC_IntGet() & RTC_IF_COMP0) // expired
    {
        // do nothing, only wake up cpu
    }
	RTC_IntClear(_RTC_IF_MASK); // clear IRQ flags
}


static void hal_time_init ()
{
	RTC_Init_TypeDef init;

	rtcInitialized = 1;

	/* Ensure LE modules are accessible */
	CMU_ClockEnable(cmuClock_CORELE, true);

	/* Enable LFACLK in CMU (will also enable oscillator if not enabled) */
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	/* Use the prescaler to reduce power consumption. */
	CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_1);

	rtcFreq = CMU_ClockFreqGet(cmuClock_RTC);

	/* Enable clock to RTC module */
	CMU_ClockEnable(cmuClock_RTC, true);

	init.enable   = false;
	init.debugRun = false;
	init.comp0Top = false;
	//init.comp0Top = true; /* Count to max before wrapping */
	RTC_Init(&init);

	/* Disable interrupt generation from RTC0 */
	RTC_IntDisable(_RTC_IF_MASK);
	RTC_IntEnable(RTC_IF_OF);	//Enable interrupt on overflow

	/* Enable interrupts */
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Enable RTC */
	RTC_Enable(true);
}

u4_t hal_ticks ()
{
    hal_disableIRQs();
    u4_t t = HAL.ticks;
    u4_t cnt = RTC_CounterGet();
	if (RTC_IntGet() & RTC_IF_OF)	// Overflow before we read CNT?
	{
        cnt = RTC_CounterGet();
        t ++;	// Include overflow in evaluation but leave update of state to ISR once interrupts enabled again
    }
    hal_enableIRQs();
	return (t<<24)|cnt;
}

// return modified delta ticks from now to specified ticktime (0 for past, FFFF for far future)
static u2_t deltaticks (u4_t time)
{
    u4_t t = hal_ticks();
    s4_t d = time - t;
    if( d<=0 ) return 0;    // in the past
    if( (d>>16)!=0 ) return 0xFFFF; // far ahead
    return (u2_t)d;
}

void hal_waitUntil (u4_t time)
{
	 while( deltaticks(time) != 0 ); // busy wait until timestamp is reached
}

// check and rewind for target time
u1_t hal_checkTimer (u4_t time)
{
    u2_t dt;
	RTC_IntClear(RTC_IF_COMP0);		//clear any pending interrupts
    if((dt = deltaticks(time)) < 5) // event is now (a few ticks ahead)
    {
    	RTC_IntDisable(RTC_IF_COMP0);	// disable IE
        return 1;
    }
    else // rewind timer (fully or to exact time))
    {
    	RTC_CompareSet(0, RTC_CounterGet() + dt);   // set comparator
    	RTC_IntEnable(RTC_IF_COMP0);  // enable IE
        return 0;
    }
}



// -----------------------------------------------------------------------------
// IRQ
static uint8_t irqlevel = 0;
static int count_int=0;
static bool int_flag=false;

CORE_DECLARE_IRQ_STATE;
void hal_disableIRQs ()
{
	char buf[32];
	count_int++;
	//sprintf(buf,"\t\t\t\tIRQ Dis count=%d\n",count_int);
	//debug_str(buf);
	if(int_flag==false){
		int_flag=true;
		//sprintf(buf,"\t\t\t\tIRQ Disabling count=%d\n",count_int);
		//debug_str(buf);
		//INT_Disable();
		CORE_ENTER_ATOMIC();
	}
	 irqlevel++;

	return;

}

void hal_enableIRQs ()
{
	char buf[32];
	count_int++;
	//sprintf(buf,"\t\t\t\tIRQ En_OUT %d count=%d\n",irqlevel,count_int);
	//debug_str(buf);

	if(--irqlevel == 0) {
		CORE_EXIT_ATOMIC();
		//INT_Enable();
		int_flag=false;
		//delay_ms(7);
		//sprintf(buf,"\t\t\t\tIRQ Enabling count=%d\n",irqlevel);
		//debug_str(buf);
		//hal_io_check();
	}
	return;
}

void hal_sleep ()
{
	//EMU_EnterEM2(false);
}

// -----------------------------------------------------------------------------

void hal_init ()
{

    memset(&HAL, 0x00, sizeof(HAL));
    hal_disableIRQs();

	hal_io_init();	// configure radio I/O and interrupt handler

    hal_spi_init();	// configure radio SPI

    hal_time_init();	// configure timer and interrupt handler

    hal_enableIRQs();
 }

void hal_failed ()
{
	rgb_on(true,false,false);
	CORE_EXIT_ATOMIC();
	debug_str("Failed");
	spi_cs_clear(radio);
	delay_ms(9);
	while(1){
	spi_write_byte(0x81);
	spi_write_byte(0x80);
	//delay_ms(1);
	spi_write_byte(0x01);
	if(spi_read_byte()==0x80){debug_str("OK");break;};
	delay_ms(9);
	}
	spi_cs_set(radio);
	//while(spi_read_write_byte(0x81)!=0x80);
	NVIC_SystemReset();
	// HALT...
    hal_sleep();
    while(1);
}

