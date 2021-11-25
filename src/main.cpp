#include <Arduino.h>


#include "uart.h"
#include "state.h"
#include "Config.h"
#include "encryption.h"
#include "DI2.h"
#include "eeprom_local.h"
#include "global.h"
#include "Bluetooth.h"

//#define USBDEBUG




/*
#include <driver/can.h>
#include <driver/gpio.h>
#include <esp_system.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"#pragma once
*/


uint8_t mtrrx_cnt = 0;
bool walken = false;

//CanBus CAN;
//PCF8574 PCF(0x20);


/*
can_message_t myMessageToSend = { 0, 0x123, 8, {0x01, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x99} };

uint32_t previousMillis;
const uint32_t interval = 1000;
*/


void Ble_Rx(){
	if (R_Bcomms.receiveDataArray(BLE.rxBuf) == true) {
		BT_HS = true;
		//Serial.println("Pkg RCV");
		BTfail_Count = 0;
		RCV_Cnt++;
	if (prev_shiftDown == 0 && R_BRx.ButVal.down == 1) {
    	 PCF.digitalWrite(PCF_GEAR_DOWN,LOW);

			Serial.println("Shift Down");
		}
		else{
 		PCF.digitalWrite(PCF_GEAR_DOWN,HIGH);

		}


		if (prev_shiftUp == 0 && R_BRx.ButVal.up == 1) {
			Serial.println("Shift UP");
			PCF.digitalWrite(PCF_GEAR_UP,LOW);
		}
		else{
		PCF.digitalWrite(PCF_GEAR_UP,HIGH);

		}

		if(R_BRx.ButVal.btn_type != prev_joy){

		

			if(R_BRx.ButVal.btn_type == 1){
			Serial.println("JOY UP");
			ui_vars.ui8_assist_level++;
			 if (ui_vars.ui8_assist_level > ui_vars.ui8_number_of_assist_levels) {
        ui_vars.ui8_assist_level = ui_vars.ui8_number_of_assist_levels;
      }
		}

			if(R_BRx.ButVal.btn_type == 2){
			Serial.println("JOY FRONT");
		}

			if(R_BRx.ButVal.btn_type == 3){
			Serial.println("JOY BACK");
		}

			if(R_BRx.ButVal.btn_type == 4){
			Serial.println("JOY DOWN");
			  if (ui_vars.ui8_assist_level > 0)
        ui_vars.ui8_assist_level--;
		}

			if(R_BRx.ButVal.btn_type == 5){
			Serial.println("JOY CEN");
		}
		
		}
		
		
		prev_shiftDown = R_BRx.ButVal.down;
		prev_shiftUp = R_BRx.ButVal.up;
		prev_joy = R_BRx.ButVal.btn_type;
		




}



}

void PassAdd() {

	L_Bcomms.txPkgLength = L_BTx.Length;
	L_Bcomms.rxPkgLength = L_BRx.Length;

	L_Bcomms.txPkgAdd[1] = (byte*)&L_BTx.AckVal;
	L_Bcomms.txPkgSize[1] = sizeof(L_BTx.AckVal);

	L_Bcomms.rxPkgAdd[1] = (byte*)&L_BRx.ButVal;
	L_Bcomms.rxPkgSize[1] = sizeof(L_BRx.ButVal);




	R_Bcomms.txPkgLength = R_BTx.Length;
	R_Bcomms.rxPkgLength = R_BRx.Length;

	R_Bcomms.txPkgAdd[R_BTx.ACK] = (byte*)&R_BTx.AckVal;
	R_Bcomms.txPkgSize[R_BTx.ACK] = sizeof(R_BTx.AckVal);

	R_Bcomms.rxPkgAdd[R_BRx.BUT_VALUES] = (byte*)&R_BRx.ButVal;
	R_Bcomms.rxPkgSize[R_BRx.BUT_VALUES] = sizeof(R_BRx.ButVal);





}

static void tSerialRcv(void* pvParameters) {
	portTickType xLastWakeTime;
	const portTickType xFrequency = 1;
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		if (MtrSerialRX() == true) {
			MtrRx_TO=0;
		}
	if (BLE.notify == true){
    Ble_Rx();
    BLE.rxBuf[20] = {0};
	BLE.notify = false;
	}

		
	}
}




static void tMainCtrlLoop(void* pvParameters) {
	portTickType xLastWakeTimeArm;
	const portTickType xFrequencyArm = 50;
	xLastWakeTimeArm = xTaskGetTickCount();
	for (;;) {
		vTaskDelayUntil(&xLastWakeTimeArm, xFrequencyArm);
		/*
		Serial.print("BT_conStatus:");
		Serial.println(BT_conStatus);
		Serial.print("BT_FailCount:");
		Serial.println(BTfail_Count);
		Serial.print("RCV Count:");
		Serial.println(RCV_Cnt);
		*/
		if (BT_conStatus == doConnect) {
			doConnect_Count++;

			if (doConnect_Count > 500) {
				BT_conStatus = doDisconnect;

			}


		}
		
		
		if (BT_conStatus == Connected)
		{
			doConnect_Count = 0;
			BTfail_Count++;


			if (BTfail_Count > 50) {
				BT_conStatus = doDisconnect;

			}
			}
		if (RCV_Cnt > 50) {
		
			R_BTx.AckVal.Ack = 1;
			R_Bcomms.sendDataArray(BLE.txBuf, R_BTx.ACK);
			BLE.BLESend(BLE.txBuf, R_Bcomms.txPkgSize[R_BTx.ACK] + 4);
			
		
			Serial.println("BLE Sent");
		
	RCV_Cnt = 0;
		}


		rt_processing();
		
		if (Serial.available()) {
			switch (Serial.read()) {
			case 'D': {
				Serial.print("RX TO:");
				Serial.println(MtrRx_TO);
				Serial.print("G_Motor_INT_STATE:");
				Serial.println(g_motor_init_state);
				Serial.print("Batt Vlt: ");
				Serial.println(ui_vars.ui16_adc_battery_voltage);
				Serial.print("Walk Assist Factor: ");
				Serial.println(ui_vars.ui8_walk_assist_level_factor[1]);
				Serial.print("Walk Assist EN Vals: ");
				Serial.print(ui_vars.ui8_walk_assist);
				Serial.print(rt_vars.ui8_walk_assist);
				Serial.print(ui_vars.ui8_assist_level);
				Serial.println(rt_vars.ui8_assist_level);
				break;
			}
			case 'W': {
				Serial.println("Walk Assist EN");
				ui_vars.ui8_walk_assist = 1;
			//	rt_vars.ui8_walk_assist = 1;
				ui_vars.ui8_assist_level = 2;
			//	rt_vars.ui8_assist_level = 2;
				break;




			}
			case 'S': {

				walken = 0;
				Serial.println("Walk Assist Disable");
				ui_vars.ui8_walk_assist = 0;
			//	rt_vars.ui8_walk_assist = 0;
				ui_vars.ui8_assist_level = 2;
			//	rt_vars.ui8_assist_level = 2;

				break;
			}

			}
		}
	}
}


void setup() {
		Wire.begin(22, 21);
	PCF.pinMode(PCF_5V_SW_EN,OUTPUT,HIGH);
	PCF.pinMode(PCF_MTR_EN,OUTPUT,HIGH);
	PCF.pinMode(PCF_CAN_SLIENT,OUTPUT,LOW);
	PCF.pinMode(PCF_GEAR_UP,OUTPUT,HIGH);
	PCF.pinMode(PCF_GEAR_DOWN,OUTPUT,HIGH);
	PCF.digitalWrite(PCF_CAN_SLIENT, LOW);
	PCF.digitalWrite(PCF_MTR_EN, HIGH);

  // put your setup code here, to run once:
	//Serial.begin(38400);
	PassAdd();
	Serial.begin(115200);

	uart_init();
	delay(1000);
	Serial.println("BOOT");
	//eeprom_test();


	CANinit();
	eeprom_init();


	BLE.Bluetooth_Init();

	
	//ant_setup();
   xTaskCreatePinnedToCore(tMainCtrlLoop, "MainLoop", 10000, NULL, tskIDLE_PRIORITY, NULL, Core0);
  xTaskCreatePinnedToCore(tSerialRcv, "SerialRcv", 10000, NULL, tskIDLE_PRIORITY + 1, NULL, Core0);
	//TaskStartScheduler();

}

void loop() {
	CANloop();

	BLE.BLELp();
	
}
