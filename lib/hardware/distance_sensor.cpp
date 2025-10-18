#include "distance_sensor.h"

Distance_Sensor::Distance_Sensor() = default;

void Distance_Sensor::begin(BinarySerializationData &data) {
	dataPtr = &data;

	pinMode(SENSOR_1_XSHUT, OUTPUT);
	pinMode(SENSOR_2_XSHUT, OUTPUT);
	pinMode(SENSOR_3_XSHUT, OUTPUT);

	digitalWriteBulk({SENSOR_1_XSHUT, SENSOR_2_XSHUT, SENSOR_3_XSHUT}, LOW);
	delay(500);

	digitalWrite(SENSOR_1_XSHUT, HIGH);
	delay(150);
	sensor1.init();
	sensor1.setAddress(SENSOR_1_ADDR);

	digitalWrite(SENSOR_2_XSHUT, HIGH);
	delay(150);
	sensor2.init();
	sensor2.setAddress(SENSOR_2_ADDR);

	digitalWrite(SENSOR_3_XSHUT, HIGH);
	delay(150);
	sensor3.init();
	sensor3.setAddress(SENSOR_3_ADDR);


	sensor1.setTimeout(10);
	sensor2.setTimeout(10);
	sensor3.setTimeout(10);

	sensor1.startContinuous();
	sensor2.startContinuous();
	sensor3.startContinuous();
}

bool Distance_Sensor::exists_uninitialized_sensor() {
	uint8_t dummy_rx;

	uint32_t timeout_us = 10000;

	int ret = i2c_read_timeout_us(i2c0, DEFAULT_ADDR, &dummy_rx, 1, false, timeout_us);

	if (ret == 1) {
		return true;
	} else if (ret == PICO_ERROR_GENERIC) {
		return false;
	} else if (ret == PICO_ERROR_TIMEOUT) {
		return false;
	} else {
		return false;
	}
}

void Distance_Sensor::tick() {
	if (millis() - previous_read >= 50) {
		get_right();
		get_left();
		get_front();

		previous_read = millis();
	}
}

void Distance_Sensor::get_front() {
	int val = sensor1.readRangeContinuousMillimeters();
	front = val;
}

void Distance_Sensor::get_right() {
	int val = sensor3.readRangeContinuousMillimeters();
	right = val;
}

void Distance_Sensor::get_left() {
	int val = sensor2.readRangeContinuousMillimeters();
	left = val;
}



