#include "DI2.h"



/* Example for the ANT+ Library @ https://github.com/brodykenrick/ANTPlus_Arduino
Copyright 2013 Brody Kenrick.

Developed for http://retrorunnerreadout.blogspot.com

Interfacing of Garmin ANT+ device (via a cheap Nordic nRF24AP UART module) to an Arduino.

Opens an ANT+ channel listening for HRM. Prints out computed hear rate.

Hardware
An Arduino Pro Mini 3v3 connected to this nRF24AP2 module : http://www.goodluckbuy.com/nrf24ap2-networking-module-zigbee-module-with-ant-transceiver-.html

The connector on nRF24AP2 board is (looking from the front, pin 1 is marked []):
[]GND(=VSS) | VDD(=3.3 volts)
UART_TX   | UART_RX
!(SUSP)   | SLEEP
RTS       | !(RESET)

Wiring to the Arduino Pro Mini 3v3 can be seen in 'antplus' below.
*/
#include <Arduino.h>
#include "ANTPlus.h"
#include "DI2.h"
//#define NDEBUG
#define __ASSERT_USE_STDERR
#include <assert.h>

//#define ANTPLUS_ON_HW_UART //!< H/w UART (i.e. Serial) instead of software serial. NOTE: There seems to be issues in not getting as many broadcast packets when using hardware serial.........
//#if !defined(ANTPLUS_ON_HW_UART)
//#include <SoftwareSerial.h>
//#endif




#define USE_SERIAL_CONSOLE //!<Use the hardware serial as the console. This needs to be off if using hardware serial for driving the ANT+ module.

//#if defined(NDEBUG) || defined(ANTPLUS_ON_HW_UART)
//#undef USE_SERIAL_CONSOLE
//#endif

//Logging macros
//********************************************************************
//#define SERIAL_DEBUG
#define ANTPLUS_DEBUG
//#if !defined(USE_SERIAL_CONSOLE)
//Disable logging under these circumstances
//#undef SERIAL_DEBUG
//#endif

//F() stores static strings that come into existence here in flash (makes things a bit more stable)
//#ifdef SERIAL_DEBUG

#define SERIAL_DEBUG_PRINT(x)  	        (Serial.print(x))
#define SERIAL_DEBUG_PRINTLN(x)	        (Serial.println(x))
#define SERIAL_DEBUG_PRINT_F(x)  	(Serial.print(F(x)))
#define SERIAL_DEBUG_PRINTLN_F(x)	(Serial.println(F(x)))
#define SERIAL_DEBUG_PRINT2(x,y)  	(Serial.print(x,y))
#define SERIAL_DEBUG_PRINTLN2(x,y)	(Serial.println(x,y))
/*
#else

#define SERIAL_DEBUG_PRINT(x)
#define SERIAL_DEBUG_PRINTLN(x)
#define SERIAL_DEBUG_PRINT_F(x)
#define SERIAL_DEBUG_PRINTLN_F(x)
#define SERIAL_DEBUG_PRINT2(x,y)
#define SERIAL_DEBUG_PRINTLN2(x,y)

#endif
*/
//********************************************************************

#define ANTPLUS_BAUD_RATE (57600) //!< The moduloe I am using is hardcoded to this baud rate.


//The ANT+ network keys are not allowed to be published so they are stripped from here.
//They are available in the ANT+ docs at thisisant.co

#define ANT_SENSOR_NETWORK_KEY {0xA9, 0xAD, 0x32, 0x68, 0x3D, 0x76, 0xC7, 0x4D}
//#define ANT_GPS_NETWORK_KEY    {0xA9, 0xAD, 0x32, 0x68, 0x3D, 0x76, 0xC7, 0x4D}

//#if !defined(ANT_SENSOR_NETWORK_KEY) || !defined(ANT_GPS_NETWORK_KEY)
//#error "The Network Keys are missing. Better go find them by signing up at thisisant.com"
//#endif

// ****************************************************************************
// ******************************  GLOBALS  ***********************************
// ****************************************************************************

//Arduino Pro Mini pins to the nrf24AP2 modules pinouts
static const int RTS_PIN = 34; //!< RTS on the nRF24AP2 module
static const int RTS_PIN_INT = 34; //!< The interrupt equivalent of the RTS_PIN

#define RXD2 26
#define TXD2 25

/*
#if !defined(ANTPLUS_ON_HW_UART)
static const int TX_PIN       = 8; //Using software serial for the UART
static const int RX_PIN       = 9; //Ditto
static SoftwareSerial ant_serial(TX_PIN, RX_PIN); // RXArd, TXArd -- Arduino is opposite to nRF24AP2 module
#else

//Using Hardware Serial (0,1) instead
#endif
*/
ANTPlus antplus = ANTPlus(34, 27/*SUSPEND*/, 32/*SLEEP*/, 33/*RESET*/);

//ANT Channel config for HRM
ANT_Channel hrm_channel
{
  0, //Channel Number
  PUBLIC_NETWORK,
  DEVCE_TIMEOUT,
  DEVCE_TYPE_DI2,
  DEVCE_DI2_FREQ,
  DEVCE_DI2_RATE,
  ANT_SENSOR_NETWORK_KEY,
  ANT_CHANNEL_ESTABLISH_PROGRESSING,
  FALSE,
  0, //state_counter
};

volatile int rts_ant_received = 0; //!< ANT RTS interrupt flag see isr_rts_ant()

// **************************************************************************************************
// *********************************  ISRs  *********************************************************
// **************************************************************************************************

//! Interrupt service routine to get RTS from ANT messages
void isr_rts_ant()
{
    rts_ant_received = 1;
}

// **************************************************************************************************
// ***********************************  ANT+  *******************************************************
// **************************************************************************************************

void process_packet(ANT_Packet* packet)
{
    //#if defined(USE_SERIAL_CONSOLE) && defined(ANTPLUS_DEBUG)
      //This function internally uses Serial.println
      //Only use it if the console is available and if the ANTPLUS library is in debug mode
    antplus.printPacket(packet, true);
    //#endif //defined(USE_SERIAL_CONSOLE) && defined(ANTPLUS_DEBUG)

    switch (packet->msg_id)
    {
    case MESG_BROADCAST_DATA_ID:
    {
        const ANT_Broadcast* broadcast = (const ANT_Broadcast*)packet->data;
        SERIAL_DEBUG_PRINT_F("CHAN ");
        SERIAL_DEBUG_PRINT(broadcast->channel_number);
        SERIAL_DEBUG_PRINT_F(" ");
        const ANT_DataPage* dp = (const ANT_DataPage*)broadcast->data;

        //Update received data
        if (broadcast->channel_number == hrm_channel.channel_number)
        {
            hrm_channel.data_rx = true;
            //To determine the device type -- and the data pages -- check channel setups
            if (hrm_channel.device_type == DEVCE_TYPE_DI2)
            {
                switch (dp->data_page_number)
                {
                case DATA_PAGE_DI2_SHIFT_SYSTEM:
                {
                    //As we only care about the computed heart rate
                    // we use a same struct for all HRM pages
                    const ANT_DI2_DataPage0* hrm_dp = (const ANT_DI2_DataPage0*)dp;
                    SERIAL_DEBUG_PRINT_F("DI2.RearGear = ");
                    SERIAL_DEBUG_PRINTLN(hrm_dp->rear_gear);
                }
                break;

                default:
                    SERIAL_DEBUG_PRINT_F(" HRM DP# ");
                    SERIAL_DEBUG_PRINTLN(dp->data_page_number);
                    break;
                }
            }
        }
        break;
    }

    default:
        SERIAL_DEBUG_PRINTLN_F("Non-broadcast data received.");
        break;
    }
}




// **************************************************************************************************
// ************************************  Setup  *****************************************************
// **************************************************************************************************
void ant_setup()
{
    //#if defined(USE_SERIAL_CONSOLE)
    Serial.begin(115200);
    while (!Serial)
    {
        ;
    }
    //#endif //defined(USE_SERIAL_CONSOLE)

    SERIAL_DEBUG_PRINTLN("ANTPlus DI2 Test!");
    SERIAL_DEBUG_PRINTLN_F("Setup.");

    SERIAL_DEBUG_PRINTLN_F("ANT+ Config.");

    //We setup an interrupt to detect when the RTS is received from the ANT chip.
    //This is a 50 usec HIGH signal at the end of each valid ANT message received from the host at the chip
    pinMode(34, INPUT);
    attachInterrupt(34, isr_rts_ant, RISING);


    //#if defined(ANTPLUS_ON_HW_UART)
      //Using hardware UART
      //Serial.begin(ANTPLUS_BAUD_RATE); 
      //antplus.begin( Serial );
    //#else
      //Using soft serial
     //ant_serial.begin( ANTPLUS_BAUD_RATE ); 

    antplus.begin(Serial2);
    Serial2.begin(ANTPLUS_BAUD_RATE, SERIAL_8N1, RXD2, TXD2);
    //#endif

    SERIAL_DEBUG_PRINTLN_F("ANT+ Config Finished.");
    SERIAL_DEBUG_PRINTLN_F("Setup Finished.");
    //delay(2000);
}

// **************************************************************************************************
// ************************************  Loop *******************************************************
// **************************************************************************************************

void ant_loop()
{
    byte packet_buffer[ANT_MAX_PACKET_LEN];
    ANT_Packet* packet = (ANT_Packet*)packet_buffer;
    MESSAGE_READ ret_val = MESSAGE_READ_NONE;

    if (rts_ant_received == 1)
    {
        SERIAL_DEBUG_PRINTLN_F("Received RTS Interrupt. ");
        antplus.rTSHighAssertion();
        //Clear the ISR flag
        rts_ant_received = 0;
    }

    //Read messages until we get a none
    while ((ret_val = antplus.readPacket(packet, ANT_MAX_PACKET_LEN, 0)) != MESSAGE_READ_NONE)
    {
        if ((ret_val == MESSAGE_READ_EXPECTED) || (ret_val == MESSAGE_READ_OTHER))
        {
            SERIAL_DEBUG_PRINT_F("ReadPacket success = ");
            if ((ret_val == MESSAGE_READ_EXPECTED))
            {
                SERIAL_DEBUG_PRINTLN_F("Expected packet");
            }
            else
                if ((ret_val == MESSAGE_READ_OTHER))
                {
                    SERIAL_DEBUG_PRINTLN_F("Other packet");
                }
            process_packet(packet);
        }
        else
        {
            SERIAL_DEBUG_PRINT_F("ReadPacket Error = ");
            SERIAL_DEBUG_PRINTLN(ret_val);
            if (ret_val == MESSAGE_READ_ERROR_MISSING_SYNC)
            {
                //Nothing -- allow a re-read to get back in sync
            }
            else
                if (ret_val == MESSAGE_READ_ERROR_BAD_CHECKSUM)
                {
                    //Nothing -- fully formed package just bit errors
                }
                else
                {
                    break;
                }
        }
    }


    if (hrm_channel.channel_establish != ANT_CHANNEL_ESTABLISH_COMPLETE)
    {
        antplus.progress_setup_channel(&hrm_channel);
        if (hrm_channel.channel_establish == ANT_CHANNEL_ESTABLISH_COMPLETE)
        {
            SERIAL_DEBUG_PRINT(hrm_channel.channel_number);
            SERIAL_DEBUG_PRINTLN_F(" - Established.");
        }
        else
            if (hrm_channel.channel_establish == ANT_CHANNEL_ESTABLISH_PROGRESSING)
            {
                SERIAL_DEBUG_PRINT(hrm_channel.channel_number);
                SERIAL_DEBUG_PRINTLN_F(" - Progressing.");
            }
            else
            {
                SERIAL_DEBUG_PRINT(hrm_channel.channel_number);
                SERIAL_DEBUG_PRINTLN_F(" - ERROR!");
            }
    }
}