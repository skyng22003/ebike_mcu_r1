#ifndef global_h
#define global_h

#include <PCF8574.h>
#include "Bluetooth.h"
#include "UARTcomms.h"
#include "CAN.h"

extern PCF8574 PCF;
extern BLELib BLE;
extern UARTComms L_Bcomms, R_Bcomms;
extern BLETxData L_BTx, R_BTx;
extern BLERxData L_BRx, R_BRx;

extern bool prev_shiftUp;
extern bool prev_shiftDown;
extern uint8_t prev_joy;
//extern CanBus CAN;




//PCF8574 Hardware PINs

#define PCF_CAN_SLIENT 0
#define PCF_MTR_EN 1
#define PCF_CHG_EN 2
#define PCF_GEAR_DOWN 3
#define PCF_GEAR_UP 4
#define PCF_5V_SW_EN 5
#define PCF_GEAR_BLE_SW 6
#define PCF_SERVO_EN 7




























#endif // !
