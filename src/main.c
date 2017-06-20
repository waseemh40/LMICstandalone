#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"

#include "../lmic/lmic.h"
//#include "../lmic/lmic.c"
#include "../hal/debug.h"
#include "../lmic/hal.h"
#include "../inc/rs232.h"
#include "../inc/delay.h"
#include "../inc/rgb_led.h"

#define LORAWAN_NET_ID (uint32_t) 0x8899115544221100
// TODO: enter device address below, for TTN just set ???
#define LORAWAN_DEV_ADDR (uint32_t) 0x0011334455335588
#define LORAWAN_ADR_ON 1
#define LORAWAN_CONFIRMED_MSG_ON 1
#define LORAWAN_APP_PORT 3//15

static uint8_t NwkSKey[] = {
    // TODO: enter network, or use TTN default
    // e.g. for 2B7E151628AED2A6ABF7158809CF4F3C =>
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

static uint8_t ArtSKey[] = {
    // TODO: enter application key, or use TTN default
    // e.g. for 2B7E151628AED2A6ABF7158809CF4F3C =>
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};
//////////////////////////////////////////////////
// CONFIGURATION (FOR APPLICATION CALLBACKS BELOW)
//////////////////////////////////////////////////

// application router ID (LSBF)
//static const u1_t APPEUI[8]  = { 0x70, 0xB3, 0xD5, 0x7E, 0xF0, 0x00, 0x59, 0x46 };//0xBE, 0x7A, 0x00, 0x0B, 0xE7, 0xA0, 0x00, 0x40 };//0x02, 0x00, 0x00, 0x00, 0x00, 0xEE, 0xFF, 0xC0 };

// unique device ID (LSBF)
//static const u1_t DEVEUI[8]  = { 0x70, 0xB3, 0xD5, 0x7E, 0xF0, 0x00, 0x59, 0x46 };//0xBE, 0x7A, 0x00, 0x0B, 0xE7, 0xA0, 0x00, 0x45 };//0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };

// device-specific AES key (derived from device EUI)
//static const u1_t DEVKEY[16] = {0x9C, 0x6F, 0xC7, 0x34, 0xA1, 0x22, 0x4C, 0x54, 0x15, 0xFB, 0x78, 0x09, 0x90, 0x83, 0xA3, 0x6E};

// LoRaWAN Application identifier (AppEUI)
// Not used in this example
static const u1_t APPEUI[8]  = { 0x88,	0x99,	0x11,	0x55,	0x44,	0x22,	0x11,	0x00};

// LoRaWAN DevEUI, unique device ID (LSBF)
// Not used in this example
static const u1_t DEVEUI[8]  = { 0x00,	0x80,	0x00,	0x00,	0x00,	0x00,	0x78,	0x86};

// LoRaWAN NwkSKey, network session key
// Use this key for The Things Network
//static const u1_t DEVKEY[16] = {0x88,	0x00,	0x77,	0x00,	0x66,	0x00,	0x55,	0x00,	0x44,	0x00,	0x33,	0x00,	0x22,	0x00,	0x11,	0x00};
static const u1_t DEVKEY[16] = {0x00,	0x11,	0x00,	0x22,	0x00,	0x33,	0x00,	0x44,	0x00,	0x55,	0x00,	0x66,	0x00,	0x77,	0x00,	0x88};
//////////////////////////////////////////////////
// APPLICATION CALLBACKS
//////////////////////////////////////////////////

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, DEVKEY, 16);
}


//////////////////////////////////////////////////
// MAIN - INITIALIZATION AND STARTUP
//////////////////////////////////////////////////

// initial job
// initial job
static void initfunc (osjob_t* j) {
    // reset MAC state
    LMIC_reset();
    // start joining
    LMIC_startJoining();
    // init done - onEvent() callback will be invoked...
}

static int cnt = 0;
static void initfunc2 (osjob_t* job) { // say hello
	debug_str("Hello World!\r\n");
	// log counter
	debug_val("cnt = ", cnt);
	// toggle LED
	debug_led(++cnt & 1);
	// reschedule job every second
	//reportEvent(EV_JOINED);
	 //LMIC_reset();
	//LMIC_startJoining();
    //ON_LMIC_EVENT(EV_JOINED);
    //engineUpdate();
	os_setTimedCallback(job, os_getTime()+sec2osticks(1), initfunc2);
}// application entry point
extern int gpio_int_flag;
int main () {
	/*
	 ********************* Chip initialization*************
	 */
		CHIP_Init();
		CMU_HFRCOBandSet(cmuHFRCOBand_7MHz );
		CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
		CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
		CMU_ClockEnable(cmuClock_HFPER, true);//////////////////////////////////extra addition from lmic git repo
		/*
		 *******************************************************
 	 */
		//rs232_init();
		//rs232_enable();
		rgb_init();
		delay_init();
	    debug_init();
		debug_str("Hello World!\r\n");
		rgb_on(false,false,true);
		delay_ms(10);
		delay_ms(10);
    osjob_t initjob;
    hal_init();
	GPIO_PinOutSet(PWR_EN_PORT,RADIO_PWR_EN);
	GPIO_PinOutSet(PWR_EN_PORT,RADIO_PWR_EN);
	GPIO_PinOutSet(PWR_EN_PORT,RADIO_PWR_EN);
    debug_str("Hello World!\r\n");
/*while(1){		rgb_on(false,false,true);
		delay_ms(10);
		rgb_shutdown();
		delay_ms(10);
	    debug_str("\t\t Loop!\r\n");
	    if(gpio_int_flag!=0){
		    debug_str("\t\t INT Happened!\r\n");
		    gpio_int_flag=0;
	    }
	}*/
    // initialize runtime env
    rgb_shutdown();
	delay_ms(10);
    os_init();
    debug_str("\t\t\tOS Init!\r\n");
    // setup initial job
	//	initfunc(&initjob);
    os_setCallback(&initjob, initfunc);
		//buildJoinRequest(HDR_FTYPE_JREQ);
    // execute scheduled jobs and events
    debug_str("\t\t\tOS callback job added!\r\n");
    os_runloop();
    // (not reached)
    return 0;
}


//////////////////////////////////////////////////
// UTILITY JOB
//////////////////////////////////////////////////

static osjob_t blinkjob;
static u1_t ledstate = 0;

static void blinkfunc (osjob_t* j) {
    // toggle LED
    ledstate = !ledstate;
    debug_led(ledstate);
    // reschedule blink job
    os_setTimedCallback(j, os_getTime()+sec2osticks(1), blinkfunc);
}


//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////

void onEvent (ev_t ev) {
    debug_event(ev);

    switch(ev) {

      // starting to join network
      case EV_JOINING:
    	  ledstate = !ledstate;
    	  //rgb_shutdown();
    	  //debug_led(ledstate);
          //blinkfunc(&blinkjob);
          debug_str("Here Done!\r\n");
    	  //rgb_on(true,false,false);
          break;

      // network joined, session established
      case EV_JOINED:
          // cancel blink job
          //os_clearCallback(&blinkjob);
          // switch on LED
          //debug_led(1);
          // (don't schedule any new actions)
    	  //rgb_shutdown();
    	  debug_str("iA Joined!\r\n");
    	  //rgb_on(true,false,false);//blinkfunc(&blinkjob);
          break;

      default:
    	  debug_str("Default Event!\r\n");
    	  //rgb_shutdown();
    	  //rgb_on(false,true,false);
    	  //delay_ms(10);
    	  //debug_led(ledstate);
    }
}/*
//////////////////////////////////////////////////
// CONFIGURATION (FOR APPLICATION CALLBACKS BELOW)
//////////////////////////////////////////////////

// application router ID (LSBF)
static const u1_t APPEUI[8]  = { 0x70, 0xB3, 0xD5, 0x7E, 0xF0, 0x00, 0x59, 0x46 };//0xBE, 0x7A, 0x00, 0x0B, 0xE7, 0xA0, 0x00, 0x40 };//0x02, 0x00, 0x00, 0x00, 0x00, 0xEE, 0xFF, 0xC0 };

// unique device ID (LSBF)
static const u1_t DEVEUI[8]  = { 0x10, 0x20, 0x30, 0x40, 0x0C, 0x0D, 0x0E, 0x0F };//0xBE, 0x7A, 0x00, 0x0B, 0xE7, 0xA0, 0x00, 0x45 };//0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };

// device-specific AES key (derived from device EUI)
static const u1_t DEVKEY[16] = { 0xC6, 0x1B, 0xD5, 0x68, 0xAA, 0x8F, 0xCA, 0x66, 0xD7, 0xD1, 0x1B, 0xF8, 0x74, 0xC4, 0xCF, 0x4B };

static const u1_t nwkKey[16] = { 0xE7, 0x63, 0x2E, 0x1C, 0xF3, 0x61, 0x7E, 0xAD, 0xF5, 0xC0, 0xBC, 0x7E, 0x38, 0xEA, 0x09, 0xA8  };

static const u1_t artKey[16] = { 0xE7, 0x63, 0x2E, 0x1C, 0xF3, 0x61, 0x7E, 0xAD, 0xF5, 0xC0, 0xBC, 0x7E, 0x38, 0xEA, 0x09, 0xA8  };
//////////////////////////////////////////////////
// APPLICATION CALLBACKS
//////////////////////////////////////////////////

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, DEVKEY, 16);
}


//////////////////////////////////////////////////
// MAIN - INITIALIZATION AND STARTUP
//////////////////////////////////////////////////

// counter
static int cnt = 0;

// log text to USART and toggle LED
/*static void initfunc (osjob_t* job) {
    // say hello

	const unsigned char str="Hello World!\r\n";
    debug_str(str);
    // log counter
    debug_val("cnt = ", cnt);
    // toggle LED
    debug_led(++cnt & 1);
    // reschedule job every second
    os_setTimedCallback(job, os_getTime()+sec2osticks(1), initfunc);
}*//*
// initial job
// initial job
// log text to USART and toggle LED
static void initfunc (osjob_t* job) {
    // say hello
    debug_str("Hello World!\r\n");
    // log counter
    debug_val("cnt = ", cnt);
    // toggle LED
    debug_led(++cnt & 1);
    // reschedule job every second
    os_setTimedCallback(job, os_getTime()+sec2osticks(1), initfunc);
}
// application entry point
int main ()
{
	/*
	 ********************* Chip initialization*************
	 */
		/*CHIP_Init();
		CMU_HFRCOBandSet(cmuHFRCOBand_7MHz );
		CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
		CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
		/*
		 *******************************************************
 	 *//*
		rgb_init();
		delay_init();
		rgb_on(false,true,false);
		delay_ms(10);
		//debug_init();
		rgb_shutdown();
		rgb_on(false,false,true);
		/*while(1){
			rs232_init();
			rs232_enable();
		rs232_transmit_string((unsigned char*)"Hello\n",7);
		rgb_on(false,false,true);
		delay_ms(10);
		rgb_shutdown();
		delay_ms(10);
		}*/
	/*osjob_t initjob;
	u4_t time;

	debug_init();	// initialize debug library
	delay_ms(10);
	rgb_shutdown();
    /*hal_init();
    while(1)
	{
		debug_str("\r\nT=");
		time = hal_ticks();
		debug_hex(time>>24);
		debug_hex(time>>16);
		debug_hex(time>>8);
		debug_hex(time>>0);
	}*/
	/*debug_str((unsigned char*)"Reached here\r\n");
	debug_str((unsigned char*)"Done here\r\n");
	rgb_on(false,true,false);
	delay_ms(10);
	 rgb_shutdown();
		rgb_on(true,false,false);
		delay_ms(10);
		 rgb_shutdown();
	//os_init();	// initialize runtime env
		    hal_init();
		    radio_init();
		    debug_str((unsigned char*)"Calling LMIC\r\n");
		    LMIC_init();
	rgb_on(false,false,true);
	delay_ms(10);
	rgb_shutdown();
	debug_str((unsigned char*)"Reached here\r\n");
	rgb_shutdown();
	os_setCallback(&initjob, initfunc);	// setup initial job
	rgb_on(false,false,true);
	delay_ms(10);
	rgb_shutdown();
	os_runloop();	// execute scheduled jobs and events


	return 0;	// (not reached)
}

//////////////////////////////////////////////////
// UTILITY JOB
//////////////////////////////////////////////////

static osjob_t blinkjob;
static u1_t ledstate = 0;

static void blinkfunc (osjob_t* j) {
    // toggle LED
    ledstate = !ledstate;
    debug_led(ledstate);
    // reschedule blink job
    os_setTimedCallback(j, os_getTime()+ms2osticks(100), blinkfunc);
}


//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////

void onEvent (ev_t ev) {
    debug_event(ev);

    switch(ev) {

      // starting to join network
      case EV_JOINING:
          // start blinking
          //blinkfunc(&blinkjob);
          break;

      // network joined, session established
      case EV_JOINED:
          // cancel blink job
          //os_clearCallback(&blinkjob);
          // switch on LED
          //debug_led(1);
          // (don't schedule any new actions)
          break;
    }
}
*/
