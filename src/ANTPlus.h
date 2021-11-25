//Copyright 2013 Brody Kenrick.
//An Ant+ library over UART ('Serial' or SoftwareSerial)

//Provides message send and receive functionality
//Also provides stringisers for a few messages
//Provides higher-level functionality for setting up listening channels for a sensor

//This works with a Nordic nRF24AP2 (should work with AP1 also)
//Tested with this nRF24AP2 module : http://www.goodluckbuy.com/nrf24ap2-networking-module-zigbee-module-with-ant-transceiver-.html

//Thanks to DigitalHack
//http://digitalhacksblog.blogspot.com.au/2012_10_01_archive.html
//That code was taken as a starting point

//Works with the hardware 'Serial' or a SoftwareSerial
//NOTE: That hardware 'Serial' might have issues when other interrupts
// are present (e.g. SPI) and might not receive all messages.
// SS does not have this issue.

#ifndef ANTPLus_h
#define ANTPLus_h

#include <Arduino.h>
#include <Stream.h>

#define ANTPLUS_MINIMAL_RECEIVE_BUFFER_FOR_BROADCAST_DATA //<! Saves SRAM if we only expect to be receiving ANT+ HRM and SDM buffers.

//#define ANTPLUS_DEBUG //!< Prints various debug messages. Disable here or via using NDEBUG externally
//#define ANTPLUS_MSG_STR_DECODE //<! Stringiser for various codes for easier debugging

#if defined(NDEBUG)
#undef ANTPLUS_DEBUG
#undef ANTPLUS_MSG_STR_DECODE
#endif

//These are from the ANT+ packages (under Apache license)
#include "antdefines.h"
#include "antmessage.h"

#define ANT_PACKET_READ_NEXT_BYTE_TIMEOUT_MS  (10) //<! If we get a byte in a read -- how long do we wait for the next byte before timing out...

#if defined(ANTPLUS_MINIMAL_RECEIVE_BUFFER_FOR_BROADCAST_DATA)
#define ANT_MAX_PACKET_LEN        (16) //!< This is the size of a packet buffer that should be presented for a read function (optimised for size with only small broadcast packets (e.g. HRM) ).
#else
#define ANT_MAX_PACKET_LEN        (80)             //!< This is the size of a packet buffer that should be presented for a read function.
#endif

//BK - Hack to save the teeniest of SRAM space....
//#define ANT_DEVICE_NUMBER_CHANNELS (8) //!< nRF24AP2 has an 8 channel version.
#define ANT_DEVICE_NUMBER_CHANNELS (1) //!< nRF24AP2 has an 8 channel version. However -- it seems there are issues bringing up two channels with this code. TODO: Review and fix.



//TODO: Make this into a class
#define DATA_PAGE_HEART_RATE_0              (0x00)
#define DATA_PAGE_HEART_RATE_0ALT           (0x80)
#define DATA_PAGE_HEART_RATE_1              (0x01)
#define DATA_PAGE_HEART_RATE_1ALT           (0x81)
#define DATA_PAGE_HEART_RATE_2              (0x02)
#define DATA_PAGE_HEART_RATE_2ALT           (0x82)
#define DATA_PAGE_HEART_RATE_3              (0x03)
#define DATA_PAGE_HEART_RATE_3ALT           (0x83)
#define DATA_PAGE_HEART_RATE_4              (0x04)
#define DATA_PAGE_HEART_RATE_4ALT           (0x84)

#define DATA_PAGE_SPEED_DISTANCE_1              (0x01) 
#define DATA_PAGE_SPEED_DISTANCE_2              (0x02) 



#define DATA_PAGE_DI2_SHIFT_SYSTEM             (0x00)


#define PUBLIC_NETWORK     (  0)

#define DEVCE_TYPE_DI2     (1)
#define DEVCE_TYPE_HRM     (120)
#define DEVCE_TYPE_CADENCE (121)
#define DEVCE_TYPE_SDM     (124)
#define DEVCE_TYPE_GPS     (  0)

#define DEVCE_TIMEOUT      (30) //!< N * 2.5 : 12 > 30 seconds
#define DEVCE_GPS_FREQ     (50) //!< 2400 + N MHz : 50 > 2450
#define DEVCE_SENSOR_FREQ  (57) //!< 2400 + N MHz : 57 > 2457
#define DEVCE_DI2_FREQ (57)

//TODO: add other rates
#define DEVCE_SDM_LOWEST_RATE     (16268)
#define DEVCE_HRM_LOWEST_RATE     (32280)

#define DEVCE_GPS_RATE     (8070)
#define DEVCE_CADENCE_RATE     (8085)
#define DEVCE_DI2_RATE  (8198)

//! ANT Packet coming off the wire.
typedef struct ANT_Packet_struct
{
   byte sync;
   byte length;
   byte msg_id;
   byte data[];//Variable -- elements == length
   //byte checksum... This is data[length]. See ANT_PACKET_CHECKSUM macro "function".
} ANT_Packet;

#define ANT_PACKET_CHECKSUM(/*Ant_Packet * */ packet) (packet->data[packet->length])

//TODO: Rename?
typedef struct ANT_Broadcast_struct
{
   byte channel_number;
   byte data[8];
} ANT_Broadcast;

typedef struct ANT_DataPage_struct
{
   byte data_page_number;
   byte sensor_specific_data[7];
} ANT_DataPage;





typedef struct ANT_DI2_DataPage0_struct
{
    byte data_page_number : 7;
    byte page_change_toggle : 1;
    //TODO: Union below
    byte unkown_1;
    byte front_gear;
    byte rear_gear;
    byte battery_pct;
    byte shift_mode;
    //TODO: Union above
    byte unkown_6;
    byte unkown_7;

} ANT_DI2_DataPage0;




typedef struct ANT_HRMDataPage_struct
{
  byte data_page_number:7;
  byte page_change_toggle:1;
//TODO: Union below
   byte who_cares_1;
   byte who_cares_2;
   byte who_cares_3;
   byte who_cares_4;
   byte who_cares_5;
//TODO: Union above
   byte heart_beat_count;
   byte computed_heart_rate;

} ANT_HRMDataPage;

typedef struct ANT_SDMDataPage1_struct
{
  byte data_page_number;
  byte last_time_frac; //  1/200 of a second
  byte last_time_int;
  byte distance_int;
  byte inst_speed_int:4;
  byte distance_frac:4;  //  1/16 of metre
  byte inst_speed_frac;
  byte stride_count;
  byte update_latency;
} ANT_SDMDataPage1;


typedef struct ANT_SDMDataPage2_struct
{
  byte data_page_number;
  byte reserved1;
  byte reserved2;
  byte cadence_int;
  byte inst_speed_int:4;
  byte cadence_frac:4;
  byte inst_speed_frac;
  byte reserved6;
  byte status;
} ANT_SDMDataPage2;

//! See progress_setup_channel().
typedef enum
{
  ANT_CHANNEL_ESTABLISH_PROGRESSING,
  ANT_CHANNEL_ESTABLISH_COMPLETE,
  ANT_CHANNEL_ESTABLISH_ERROR,

}   ANT_CHANNEL_ESTABLISH;

#define ANT_CHANNEL_NUMBER_INVALID (-1)

//! Details required to establish an ANT+ (and ANT?) channel. See progress_setup_channel().
typedef struct ANT_Channel_struct
{
   //Configuration items for channel
   int channel_number; //TODO: Look at making this internally assigned
   int network_number;
   int timeout;
   int device_type;
   int freq;
   int period;
   unsigned char ant_net_key[8];
   
   ANT_CHANNEL_ESTABLISH channel_establish; //Read-only from external
   boolean data_rx;                         //Broadcast data received. For now this is only updated from external. TODO: Move internally
   int state_counter; //Private for internal use only
} ANT_Channel;
 


//! See readPacket().
typedef enum
{
  MESSAGE_READ_NONE, //No message available (immediately or after timeout period)
  MESSAGE_READ_ERROR_BAD_CHECKSUM,
  MESSAGE_READ_ERROR_MISSING_SYNC,
  MESSAGE_READ_ERROR_PACKET_SIZE_EXCEEDED,
  MESSAGE_READ_INFO_TIMEOUT_MIDMESSAGE, //!< This might be recoverable on a subsequent call
  MESSAGE_READ_INTERNAL, //This is remapped to one of the next two in the internal read function
  MESSAGE_READ_OTHER,
  MESSAGE_READ_EXPECTED

} MESSAGE_READ;




//TODO: Look at ANT and ANT+ and work out the appropriate breakdown for a subclass/separate class
class ANTPlus
{
  public:
    ANTPlus(
        byte RTS_PIN,
        byte SUSPEND_PIN,
        byte SLEEP_PIN,
        byte RESET_PIN
    );

    void     begin(Stream &serial);
    void     hardwareReset( );

    boolean send(unsigned msgId, unsigned msgId_ResponseExpected, unsigned char argCnt, ...);
    MESSAGE_READ readPacket( ANT_Packet * packet, int packetSize, int wait_timeout );
    
    void         printPacket(const ANT_Packet * packet, boolean final_carriage_return);

    void sleep( boolean activate_sleep=true );
    void suspend(boolean activate_suspend=true );
    
    //Callback from the main code
    void   rTSHighAssertion();

    boolean awaitingResponseLastSent() {return (msgResponseExpected != MESG_INVALID_ID);};

    //!ANT+ to setup a channel
    ANT_CHANNEL_ESTABLISH progress_setup_channel( ANT_Channel * channel );

#if defined(ANTPLUS_MSG_STR_DECODE)
    static const char * get_msg_id_str(byte msg_id);
#endif /*defined(ANTPLUS_MSG_STR_DECODE)*/

    static int update_sdm_rollover( byte MessageValue, unsigned long int * Cumulative, byte * PreviousMessageValue );

  private:
    MESSAGE_READ      readPacketInternal( ANT_Packet * packet, int packetSize, unsigned int readTimeout);
    unsigned char     writeByte(unsigned char out, unsigned char chksum);

    static void serial_print_byte_padded_hex(byte value);
    static void serial_print_int_padded_dec(long int value, unsigned int width, boolean final_carriage_return = false);

  private:
    Stream* mySerial; //!< Serial -- Software serial or Hardware serial

  public: //TODO: Just temp (to eventually be removed -- or added to the interface properly)
    long rx_packet_count;
    long tx_packet_count;
    long hw_reset_count;

  private:
    unsigned msgResponseExpected; //TODO: This should be an enum.....
    
    volatile boolean clear_to_send;
    
    int rxBufCnt;
    unsigned char rxBuf[ANT_MAX_PACKET_LEN];

    byte RTS_PIN;
    byte SUSPEND_PIN;
    byte SLEEP_PIN;
    byte RESET_PIN;

};

#endif //ANTPLus_h

