
#include <Arduino.h>
#include <driver/can.h>
#include <driver/gpio.h>
#include <esp_system.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"#pragma once
#include "eeprom_local.h"


void CANinit();
void CANloop();

/*
class CanBus {
public:
	void init();
	void loop();


private:





};
*/