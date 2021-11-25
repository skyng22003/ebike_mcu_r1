/*
 * Bluetooth.cpp
 * Created on: 01 Mar 2015
 * Author: Sky Ng
 */

#include "global.h"
#include "Config.h"
#include "Bluetooth.h"
#include <Arduino.h>
//#define USBDEBUG
//long now = 0;

bool notify = false;
uint8_t rxBuf[20] = {0};
float rssi;
int PrintTime = 0;
long Scantime = 0;
long lastScan = 0;
long lastMsg = 0;
int flag = false;
uint8_t BLEtimeout = 0;
boolean BT_HS = false;
int doConnect_Count = 0;
int BTfail_Count = 0;
boolean BT_Send = false;
boolean BT_RCV = false;
uint8_t RCV_Cnt = 0;
uint8_t BT_conStatus = 0;
boolean prevBTServerCon = false;
uint32_t BTpass = 492581;
bool BLEinit = false;

static BLEUUID	serviceUUID("0783b03e-8535-b5a0-7140-a304d2495cb7");
static BLEUUID    charUUID("0783b03e-8535-b5a0-7140-a304d2495cba");
static BLEUUID    rxUUID("0783b03e-8535-b5a0-7140-a304d2495cb8");

static BLEAddress *pServerAddress;

BLERemoteService* pRemoteService;

 BLEScan* pBLEScan;
//static BLEClient* pClient;


static BLEClient* pClient = BLEDevice::createClient();
static BLERemoteCharacteristic* pRemoteCharacteristic;

//static bool doConnect = false;
//static bool connected = false;
//bool Scanning = false;



// Private variables

// Private function



class MySecurity : public BLESecurityCallbacks {

	uint32_t onPassKeyRequest() {
		ESP_LOGI(LOG_TAG, "PassKeyRequest");
		return BTpass;
	}
	void onPassKeyNotify(uint32_t pass_key) {
		ESP_LOGE(LOG_TAG, "The passkey Notify number:%d", pass_key);
	}
	bool onConfirmPIN(uint32_t pass_key) {
		ESP_LOGI(LOG_TAG, "The passkey YES/NO number:%d", pass_key);
		vTaskDelay(5000);
		return true;
	}
	bool onSecurityRequest() {
		ESP_LOGI(LOG_TAG, "Security Request");
		return true;
	}
	void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl) {
		if (auth_cmpl.success) {
			ESP_LOGI(LOG_TAG, "remote BD_ADDR:");
			//      esp_log_buffer_hex(LOG_TAG, auth_cmpl.bd_addr, sizeof(auth_cmpl.bd_addr));
			ESP_LOGI(LOG_TAG, "address type = %d", auth_cmpl.addr_type);
		}
		ESP_LOGI(LOG_TAG, "pair status = %s", auth_cmpl.success ? "success" : "fail");
	}
};



class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
	  BT_conStatus = Connected;
  }

  void onDisconnect(BLEClient* pclient) {
	  //	pClient->disconnect();
	//pBLEScan->clearResults();
	//	BLEDevice::deinit(false);
    BT_conStatus = doDisconnect;
    Serial.println("onDisconnect");
  }
};





void BLELib::GetRssi(void) {
	rssi = pClient->getRssi();


}

void BLELib::Bluetooth_Init(void) {
/*
 //    pinMode(4, OUTPUT); //configure bluetooth pin mode
 // pinMode(6, OUTPUT); // put your setup code here, to run once:
//digitalWrite(4, LOW); //enable configuration mode
 // digitalWrite(6, LOW);
 // digitalWrite(6, HIGH);
  //pinMode(14, OUTPUT);
  //digitalWrite(14,LOW);
 pinMode(dvBTAtModePin, OUTPUT); //configure bluetooth pin mode
 pinMode(dvBTRstPin, OUTPUT);
 pinMode(dvBT1AtModePin, OUTPUT); //configure bluetooth pin mode
 pinMode(dvBT1RstPin, OUTPUT);
digitalWrite(dvBTAtModePin, LOW); //enable configuration mode
digitalWrite(dvBTRstPin, LOW); //Reset BT
digitalWrite(dvBTRstPin, HIGH);

digitalWrite(dvBT1AtModePin,LOW);
digitalWrite(dvBT1RstPin,LOW);
digitalWrite(dvBT1RstPin, HIGH);
// pinMode(dvBTRstPin, OUTPUT); 
// digitalWrite(dvBTRstPin, HIGH); //Reset when pin is low, make sure it is high now

 // Serial4.end();
 // Serial4.begin(dvBTBaudRate); //set baud rate for serial3
*/

	BLEDevice::init("");
	BLEDevice::setPower(ESP_PWR_LVL_P9);

BT_conStatus = Disconnected;
	BLEinit = true;
}





void BLELib::BLELp() {
	long now = millis();
//#ifdef USBDEBUG
	if (now - PrintTime > 500) {
		String newValue = "Time since boot: " + String(millis() / 1000);
		Serial.println(newValue);
		Serial.print("RCV Count:");
		Serial.println(RCV_Cnt);
		//Serial.print("BLE TimeOut: ");
		//Serial.println(BLEtimeout);
		Serial.print("BT_conStatus: ");
		Serial.println(BT_conStatus);
		Serial.print("RSSI: ");
		Serial.println(pClient->getRssi());
		PrintTime = now;
	}

//#endif

	if (now - lastMsg > 200) {

		if (BT_conStatus == Disconnected) {
			
			BT_conStatus = Scanning;
			lastScan = millis() / 1000;
#ifdef USBDEBUG
			Serial.println("Scanning");
#endif
			//BLEScan* pBLEScan = BLEDevice::getScan();
			if (BLEinit == false) {
				BLEDevice::init("");
				BLEDevice::setPower(ESP_PWR_LVL_P9);
				BLEinit = true;
			}
			


			pBLEScan = BLEDevice::getScan();
			pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
			MyAdvertisedDeviceCallbacks myCallbacks;
			//pBLEScan->setAdvertisedDeviceCallbacks(&myCallbacks);
			pBLEScan->setActiveScan(true);
			pBLEScan->start(30);
			BTfail_Count = 0;
			doConnect_Count = 0;
		}

		if (BT_conStatus == Scanning) {
			Scantime = (millis() / 1000) - lastScan;

		}

		if (BT_conStatus == doConnect) {
			BTfail_Count = 0;
			Scantime = 0;
			if (connectToServer(*pServerAddress)) {
#ifdef USBDEBUG
				Serial.println("We are now connected to the BLE Server.");
#endif
				//BT_conStatus = Connected;
				doConnect_Count = 0;
				//delay(100);
				//connected = true;
				// Scanning = false;
				//RCV_Cnt = 0;
			}
			else {
#ifdef USBDEBUG
				Serial.println("We have failed to connect to the server; there is nothin more we will do.");
#endif
				//Scantime = 0;
				BT_conStatus = Disconnected;
			}

		}


		if (BT_conStatus == Connected) {
			Scantime = 0;
			doConnect_Count = 0;

			/*
			if (now - lastMsg > 500) {
				String newValue = "Time since boot: " + String(millis() / 1000);
				Serial.println(newValue);
				Serial.print("BLE TimeOut: ");
				Serial.println(BLEtimeout);
				Serial.print("BT_conStatus: ");
				Serial.println(BT_conStatus);
			}
			*/
			/*
			if (BTfail_Count > 50) {
				BT_conStatus = doDisconnect;
			//	BT_HS = false;
			}

			/*
			if (pClient->isConnected() != true) {
				BT_conStatus = Disconnected;
			}
			*/
		}


		

		if (BT_conStatus == doDisconnect) {
			//if (pClient->isConnected() != true) {
			pClient->disconnect();
			pBLEScan->clearResults();
			BLEDevice::deinit(false);
			BLEinit = false;
			//}
			BT_conStatus = Disconnected;

		}
		if (BT_conStatus == Scanning && Scantime > 35) {
			BT_conStatus = Disconnected;
			Scantime = 0;

		}





		lastMsg = now;

	}
	
}






void BLELib::BLESend(uint8_t* txData, uint8_t packet_size)
{

	if (BT_conStatus == Connected) {
		//String Tx = "6110000050000";


			pRemoteCharacteristic->writeValue(txData, packet_size);
			Serial.println("Sent BLE");
	}
}


static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {


	rxBuf[20] = {0};
	/*
	Serial.print("Notify callback for characteristic ");

	Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
	Serial.print(" of data length ");
	Serial.println(length);

	Serial.print("Data: ");
	Serial.print(pData[0]);
	Serial.print(pData[1]);
	Serial.print(pData[2]);
	Serial.print(pData[3]);
	Serial.print(pData[4]);
	Serial.print(pData[5]);
	Serial.print(pData[6]);
	Serial.print(pData[7]);
	Serial.print(pData[8]);
	Serial.print(pData[9]);
	Serial.println(pData[10]);
	*/
memcpy( rxBuf, pData, 20 );
notify = true;
/*
	for (int i = 0; i < 20; i++) {
		rxBuf[i] = pData[i];
		pData[i] = 0;
	}
*/
		/*
		Serial.print("UP: ");
		Serial.println(R_BRx.ButVal.up);
		Serial.print("Down: ");
		Serial.println(R_BRx.ButVal.down);
		Serial.print("Alt: ");
		
		Serial.println(R_BRx.ButVal.alt);
		
		Serial.print("Joy: ");
		Serial.println(R_BRx.ButVal.btn_type);
		Serial.print("Batt PC: ");
		Serial.println(R_BRx.ButVal.batt_pc);
		*/
		//uint8_t txBuf[20];
		
	//	R_BTx.AckVal.Ack = 1;


	/*
	Serial.print("Notify callback for characteristic ");
	Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
	Serial.print(" of data length ");
	Serial.println(length);
	Serial.print("Data: ");
	Serial.print(pData[0]);
	Serial.print(pData[1]);
	Serial.print(pData[2]);
	Serial.print(pData[3]);
	Serial.print(pData[4]);
	Serial.print(pData[5]);
	Serial.print(pData[6]);
	Serial.print(pData[7]);
	Serial.print(pData[8]);
	Serial.print(pData[9]);
	Serial.println(pData[10]);
BLEtimeout = 0;
*/


}



bool BLELib::connectToServer(BLEAddress pAddress) {
	Serial.print("Forming a connection to ");
	Serial.println(pAddress.toString().c_str());

    pClient  = BLEDevice::createClient();
	//pClient->createClient();
	
 pClient->setClientCallbacks(new MyClientCallback());
		//Security
		/*
		BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
		BLEDevice::setSecurityCallbacks(new MySecurity());

		BLESecurity *pSecurity = new BLESecurity();
		pSecurity->setKeySize();
		pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
		pSecurity->setCapability(ESP_IO_CAP_KBDISP);//ESP_IO_CAP_OUT ESP_IO_CAP_IO
		pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

		*/

		//Serial.println(" - Created client");

		// Connect to the remove BLE Server.

	pClient->connect(pAddress);
	Serial.println(" - Connected to server");

	// Obtain a reference to the service we are after in the remote BLE server.
	pRemoteService = pClient->getService(serviceUUID);
	if (pRemoteService == nullptr) {

#ifdef USBDEBUG
		Serial.print("Failed to find our service UUID: ");
		Serial.println(serviceUUID.toString().c_str());
#endif
		return false;
	}
	Serial.println(" - Found our service");





	pRemoteCharacteristic = pRemoteService->getCharacteristic(rxUUID);
	if (pRemoteCharacteristic == nullptr) {
#ifdef USBDEBUG
		Serial.print("Failed to find RX UUID: ");
		Serial.println(rxUUID.toString().c_str());
#endif
		return false;
	}

	Serial.println(" - Found RX characteristic");
	// Read the value of the characteristic.
	//std::string value1 = pRemoteCharacteristic->readValue();
	// Serial.print("The characteristic value was: ");
	// Serial.println(value1.c_str());

	pRemoteCharacteristic->registerForNotify(notifyCallback);
	//pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);

	// Obtain a reference to the characteristic in the service of the remote BLE server.
	pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
	if (pRemoteCharacteristic == nullptr) {
#ifdef USBDEBUG
		Serial.print("Failed to find our characteristic UUID: ");
		Serial.println(charUUID.toString().c_str());
#endif
		return false;
	}

	else {
		return true;
	}
#ifdef USBDEBUG
	Serial.println(" - Found our characteristic");
#endif
	//std::string value = pRemoteCharacteristic->readValue();
	//Serial.print("The characteristic value was: ");
	//Serial.println(value.c_str());
}





bool BLELib:: BluetoothStats(void) //read bluetooth connection status
{
	/*
	if (digitalRead(dvBTConStatPin) == LOW) {
		return false;
	}
	if (digitalRead(dvBTConStatPin) == HIGH) {
		return true;
	}
	*/
};


 


  

  void MyAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {

	  /**
	  * Called for each advertising BLE server.
	  */
#ifdef USBDEBUG
	  Serial.print("BLE Advertised Device found: ");
	  Serial.println(advertisedDevice.toString().c_str());
#endif
	  // We have found a device, let us now see if it contains the service we are looking for.
	  if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {

		  // 
#ifdef USBDEBUG
		  Serial.print("Found our device!  address: ");

#endif		
		  advertisedDevice.getScan()->stop();
		  pServerAddress = new BLEAddress(advertisedDevice.getAddress());

		  BT_conStatus = doConnect;
	  } // Found our server
  }; // onResult




