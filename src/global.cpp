#include "global.h"


//UARTComms SMcomms, BLEcomms;
//SMCUTxData SMTx;
//SMCURxData SMRx;

BLELib BLE;
UARTComms L_Bcomms, R_Bcomms;
BLETxData L_BTx, R_BTx;
BLERxData L_BRx, R_BRx;
PCF8574 PCF(0x20);


bool prev_shiftUp = 0;
bool prev_shiftDown = 0;
uint8_t prev_joy = 0;
//CanBus CAN;
