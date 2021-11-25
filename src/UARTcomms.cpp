#include "UARTcomms.h"
//#include "TypeDef.h"






void UARTComms::begin(HardwareSerial *theSerial) {
	_serial = theSerial;
	//rx_buffer = (uint8_t*)malloc(50);
	//txPkgAdd = (uint8_t*)malloc(txLength);
	//rxPkgAdd = (uint8_t*)malloc(rxLength);
	//	txPkgLengthls = txPkgLength;
	//	rxPkgLengthls = rxPkgLength;
	/*
	for (uint8_t i = 0; i < txPkgLengthls; i++) {
	txPkgAdd[i] = txPkgAddPtr[i];
	txPkgSize[i] = sizeof(txPkgAdd[i]);
	}

	for (uint8_t x = 0; x < txPkgLengthls; x++) {
	rxPkgAdd[x] = rxPkgAddPtr[x];
	rxPkgSize[x] = sizeof(rxPkgAdd[x]);
	}

	*/
	//dynamic creation of rx parsing buffer in RAM

}



void UARTComms::sendData(uint8_t packet_type) {

	_serial->write(0x06);
	//_serial->write(0x85);
	_serial->write(packet_type);
	uint8_t CS = txPkgSize[packet_type];
	//uint8_t CS = packet_state.size;
	//_serial->write(packet_state.size);
	_serial->write(txPkgSize[packet_type]);

	for (uint8_t i = 0; i < txPkgSize[packet_type]; i++) {
		CS ^= *(txPkgAdd[packet_type] + i);
		_serial->write(*(txPkgAdd[packet_type] + i));
	}

	/*
	for (int i = 0; i<(packet_state.size); i++) {
	CS ^= *(packet_state.address + i);
	_serial->write(*(packet_state.address + i));
	}

	*/
	_serial->write(CS);
}



void UARTComms::sendDataArray(uint8_t* tx, uint8_t packet_type) {
	tx[0] = (0x06);
	tx[1] = (packet_type);
	uint8_t CS = txPkgSize[packet_type];
	tx[2] = txPkgSize[packet_type];

	for (uint8_t i =0; i < txPkgSize[packet_type]; i++) {
		CS ^= *(txPkgAdd[packet_type] + i);
		//Serial.print("CS: ");
			//Serial.println(CS);
		tx[3+i]= *(txPkgAdd[packet_type] + i);
	}

	tx[2 + txPkgSize[packet_type]+ 1] = (CS);

}





boolean UARTComms::receiveDataArray( uint8_t* rx ) {

					if (rx[0] != 0x06) {
					//	Serial.println("F1");
						return false;
					}
					pkgtype = rx[1];

				if (pkgtype > rxPkgLength) {
					rx_len = 0;
				//	Serial.println("F2");
					return false;
				}

				rx_len = rx[2];
				
				if (rx_len != rxPkgSize[pkgtype]) {
				/*
					Serial.print("Pkgtype: ");
					Serial.println(pkgtype);
					Serial.print("RXL: ");
					Serial.println(rx_len);
					Serial.print("F3: ");
					Serial.println(rxPkgSize[pkgtype]);
					*/
					rx_len = 0;
					return false;
				}

		

	//we get here if we already found the header bytes, the struct size matched what we know, and now we are byte aligned.
	if (rx_len != 0) {
	//	Serial.println("Head OK");
		for (rx_array_inx =0; rx_array_inx <= rx_len; rx_array_inx++) {
			rx_buffer[rx_array_inx] = rx[3 + rx_array_inx];
		}

		if (rx_len == (rx_array_inx - 1)) {
			//seem to have got whole message
			//last uint8_t is CS
			calc_CS = rx_len;
			for (uint8_t i = 0; i< rx_len; i++) {
				calc_CS ^= rx_buffer[i];
			}

			if (calc_CS == rx_buffer[rx_array_inx - 1]) {//CS good
				memcpy(rxPkgAdd[pkgtype], rx_buffer, rxPkgSize[pkgtype]);
				rx_len = 0;
				rx_array_inx = 0;
		//			Serial.println("Checksum OK");

				return true;
			}

			else {
				//failed checksum, need to clear this out anyway
				rx_len = 0;
				rx_array_inx = 0;
		//		Serial.println("Checksum Failed");

				return false;
			}

		}
	}

	return false;
}















boolean UARTComms::receiveData() {
	//start off by looking for the header bytes. If they were already found in a previous call, skip it.
	if (rx_len == 0) {
		//this size check may be redundant due to the size check below, but for now I'll leave it the way it is.
		if (_serial->available() >= 3) {
			//this will block until a 0x06 is found or buffer size becomes less then 3.
			while (_serial->read() != 0x06) {
				//This will trash any preamble junk in the serial buffer
				//but we need to make sure there is enough in the buffer to process while we trash the rest
				//if the buffer becomes too empty, we will escape and try again on the next call
				if (_serial->available() < 3)
					// Serial.println("Header Failed");
					return false;
			}

			pkgtype = _serial->read();

			if (pkgtype < rxPkgLength)
			{
				//memset(packetTemp.rx_buffer, 0, sizeof(packetTemp.rx_buffer));
				//Serial.print("Receive SW Packet:");
				//Serial.println(pkgsw);
				rx_len = _serial->read();
				//Serial.print("RX Len:");
				//Serial.println(packet_state.rx_len);
				//Serial.println("Size OK");
				//make sure the binary structs on both Arduinos are the same size.
				if (rx_len != rxPkgSize[pkgtype]) {
					rx_len = 0;
					//Serial.println("Size Failed");

					return false;
				}

			}

		}
	}

	//we get here if we already found the header bytes, the struct size matched what we know, and now we are byte aligned.
	if (rx_len != 0) {

		while (_serial->available() && rx_array_inx <= rx_len) {
			rx_buffer[rx_array_inx++] = _serial->read();
		}

		if (rx_len == (rx_array_inx - 1)) {
			//seem to have got whole message
			//last uint8_t is CS
			calc_CS = rx_len;
			for (uint8_t i = 0; i< rx_len; i++) {
				calc_CS ^= rx_buffer[i];
			}

			if (calc_CS == rx_buffer[rx_array_inx - 1]) {//CS good
				memcpy(rxPkgAdd[pkgtype], rx_buffer, rxPkgSize[pkgtype]);
				rx_len = 0;
				rx_array_inx = 0;
				//	Serial.println("Checksum OK");

				return true;
			}

			else {
				//failed checksum, need to clear this out anyway
				rx_len = 0;
				rx_array_inx = 0;
				//	Serial.println("Checksum Failed");

				return false;
			}

		}
	}

	return false;
}





