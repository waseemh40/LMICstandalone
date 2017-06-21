#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"

#include "../lmic/lmic.h"
#include "../hal/debug.h"
#include "../lmic/hal.h"
#include "../inc/rs232.h"
#include "../inc/delay.h"
#include "../inc/rgb_led.h"

//////////////////////////////////////////////////
// CONFIGURATION (FOR APPLICATION CALLBACKS BELOW)
//////////////////////////////////////////////////
// LoRaWAN Application identifier (AppEUI)
// Not used in this example
	//my gateway EUIs
static const u1_t APPEUI[8]  = { 0x88,	0x99,	0x11,	0x55,	0x44,	0x22,	0x11,	0x00};

// LoRaWAN DevEUI, unique device ID (LSBF)
// Not used in this example
static const u1_t DEVEUI[8]  = { 0x00,	0x80,	0x00,	0x00,	0x00,	0x00,	0x78,	0x86};

// LoRaWAN NwkSKey, network session key
// Use this key for The Things Network
//static const u1_t DEVKEY[16] = {0x88,	0x00,	0x77,	0x00,	0x66,	0x00,	0x55,	0x00,	0x44,	0x00,	0x33,	0x00,	0x22,	0x00,	0x11,	0x00};
static const u1_t DEVKEY[16] = {0x00,	0x11,	0x00,	0x22,	0x00,	0x33,	0x00,	0x44,	0x00,	0x55,	0x00,	0x66,	0x00,	0x77,	0x00,	0x88};

/*
	//TTN EUIs
static const u1_t DEVEUI[8]  = { 0x46,	0x59,	0x00,	0xF0,	0x7E,	0xD5,	0xB3,	0x70};

// LoRaWAN DevEUI, unique device ID (LSBF)
// Not used in this example
static const u1_t APPEUI[8]  = { 0x46,	0x59,	0x00,	0xF0,	0x7E,	0xD5,	0xB3,	0x70};

// LoRaWAN NwkSKey, network session key
// Use this key for The Things Network
//static const u1_t DEVKEY[16] = {0x88,	0x00,	0x77,	0x00,	0x66,	0x00,	0x55,	0x00,	0x44,	0x00,	0x33,	0x00,	0x22,	0x00,	0x11,	0x00};
static const u1_t DEVKEY[16] = {0x9C,	0x6F,	0xC7,	0x34,	0xA1,	0x22,	0x4C,	0x54,	0x15,	0xFB,	0x78,	0x09,	0x90,	0x83,	0xA3,	0x6E};
*/

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
static int tx_function (void) {
	unsigned char buf[220];
	sprintf((char*)buf,"insh A ALLAH txt msg will be reveiced\n");// in good form and time will be much lesser I think so. Hello world this is Things Network for TTK8108 course...insh A ALLAH txt msg will be reveiced in good form\n");
	int channel=4;
	LMIC_setupBand(BAND_AUX,14,100);
	LMIC_setupChannel(4,868500000,DR_RANGE_MAP(DR_SF12,DR_SF7),BAND_AUX);
	for(int i=0; i<9; i++) { // For EU; for US use i<71
	  if(i != channel) {
	    LMIC_disableChannel(i);
	  }
	}
	LMIC_setDrTxpow(DR_SF7, 7);
	LMIC_setAdrMode(false);
	return (LMIC_setTxData2(2,buf,strlen((char*)buf),1));
}

static void initfunc (osjob_t* j) {
    // reset MAC state
    LMIC_reset();
    // start joining
    LMIC_startJoining();
    // init done - onEvent() callback will be invoked...
    /*if(tx_function()<0){
    	debug_str("Tx gave -ive value\n");
    }
    else{
    	debug_str("Tx gave +ive value\n");
    }*/
}
static int cnt = 0;
static void initfunc2 (osjob_t* job) { // say hello
	debug_str("Hello World!\n");
	// log counter
	debug_val("cnt = ", cnt);
	// toggle LED
	debug_led(++cnt & 1);
	os_setTimedCallback(job, os_getTime()+sec2osticks(1), initfunc2);
}

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
	osjob_t initjob;
	rgb_init();
	delay_init();
	debug_init();
    hal_init();
	rgb_on(false,false,true);
	delay_ms(10);
    debug_str("Hello World!\n");
    rgb_shutdown();
	delay_ms(10);
    // initialize runtime env
    os_init();
    debug_str("\t\t\tOS init done\n");
    // setup initial job
    os_setCallback(&initjob, initfunc);
    // execute scheduled jobs and events
    debug_str("\t\t\tOS callback job added!\n");
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
    tx_function();
    os_setTimedCallback(j, os_getTime()+sec2osticks(20), blinkfunc);
}


//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////

void onEvent (ev_t ev) {
    //debug_event(ev);

    switch(ev) {

      // starting to join network
      case EV_JOINING:
          debug_str("EV_JOINING\n");
          break;

      // network joined, session established
      case EV_JOINED:
    	  debug_str("EV_JOINED\n");
    	  blinkfunc(&blinkjob);
          break;
      //transmission complete
      case EV_TXCOMPLETE:
    	  debug_str("EV_TXCOMPLETE\n");
    	  break;
      case EV_JOIN_FAILED:
    	  debug_str("EV_JOIN_FAILED\n");
    	  break;
      case EV_RXCOMPLETE:
    	  debug_str("EV_RXCOMPLETE\n");
    	  break;
      case EV_SCAN_TIMEOUT:
    	  debug_str("EV_SCAN_TIMEOUT\n");
    	  break;
      case EV_LINK_DEAD:
    	  debug_str("EV_LINK_DEAD\n");
	      break;
      case EV_LINK_ALIVE:
    	  debug_str("EV_LINK_ALIVE\n");
	      break;
      default:
    	  debug_str("Default Event..\n");
    	  break;
    }
}
