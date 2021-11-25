

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include<BLEDevice.h>
#include<BLEClient.h>


extern bool BT_HS;
extern int BTfail_Count;
extern int doConnect_Count;
extern bool BT_Send;
extern bool BT_RCV;
extern uint8_t RCV_Cnt;
extern uint8_t BT_conStatus;
extern float rssi;



typedef enum {
	Disconnected,
	Scanning,
	doConnect,
	Connected,
	doDisconnect
} Status;




class BLELib {

public:
	// Functions
	void Bluetooth_Init(void);
	bool BluetoothStats(void); //read bluetooth connection status
	void BLELp();
	void BLESend(uint8_t* txData, uint8_t packet_size);
	uint8_t txBuf[20];
	void GetRssi(void);
    uint8_t rxBuf[20];
	bool notify;


	


private:
	bool connectToServer(BLEAddress pAddress);
	

};

class MyAdvertisedDeviceCallbacks :
public BLEAdvertisedDeviceCallbacks{
		void onResult(BLEAdvertisedDevice advertisedDevice);
};






typedef struct DRIVE_Val_t {
	unsigned short Arm; //  Arm state
	unsigned short State; //other states, charging etc not implimented yet
	unsigned short B_PC; //battery capacity
	unsigned short rps; //average speed in rps
	//unsigned short CtrlMd;
	//unsigned short FlashW;
	bool ESC1_Error;
	bool ESC2_Error;
};

typedef struct CRTL_Val_t {
	unsigned short Arm;
	unsigned short throttle;
	//unsigned short CtrlMd;
	//unsigned short FlashW;
	//bool accel;
	//bool brake;
};




typedef enum {
	NONE = 0,
	UP = 1,
	DOWN = 2,
	LEFT = 3,
	RIGHT = 4,
	CEN = 5,
} JOY_VAL;

typedef enum
{
	GEAR_UP = 1,
	GEAR_DOWN = 2,
	DROPPER = 3,
} SHIFT_VAL;



typedef struct
{
	uint8_t btn_type;
}Shift_Data_t;

typedef struct
{
	uint8_t btn_type;
}Joy_Data_t;


typedef struct
{
	uint8_t brake_status;
}Brake_Data_t;

typedef struct
{
	uint8_t batt_pct;
}Batt_Data_t;

typedef struct
{
	uint8_t Ack;
}Ack_Data_t;



typedef struct
{
	uint8_t up;
	uint8_t down;
	uint8_t alt;
	uint8_t brake_status;
	uint8_t btn_type;
	uint8_t batt_pc;
} But_Data_t;



class BLETxData {
public:
	typedef enum Type
	{
		ACK=1,
		Length
	};
	Ack_Data_t AckVal;
};

class BLERxData {
public:
	typedef enum Type
	{
		BUT_VALUES=1,
		Length
	};
	But_Data_t ButVal;
	//Shift_Data_t ShiftVal;
	//Joy_Data_t JoyVal;
	//Batt_Data_t BattVal;
};







#endif
