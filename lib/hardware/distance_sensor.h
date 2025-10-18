#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H

#include "../software/binarySerializationData.h"
#include "VL53L0X.h"
#include "../../pico-lib/gpio.h"
#include "../../pico-lib/time.h"

#define DEFAULT_ADDR 0x29

#define SENSOR_1_ADDR 0x2a
#define SENSOR_1_XSHUT 22

#define SENSOR_2_ADDR 0X2b
#define SENSOR_2_XSHUT 26

#define SENSOR_3_ADDR 0X2c
#define SENSOR_3_XSHUT 27

class Distance_Sensor {
public:
	Distance_Sensor();
	void begin(BinarySerializationData& data);
	void tick();

	int right;
	int left;
	int front;
private:
	void get_front();
	void get_left();
	void get_right();

	bool exists_uninitialized_sensor();

	VL53L0X sensor1;
	VL53L0X sensor2;
	VL53L0X sensor3;

	void reinitialize_sensor(VL53L0X &sensor, int xshut, int addr);
	long long uninitialized_found;
	unsigned long long previous_read;
	BinarySerializationData* dataPtr;
};



#endif //DISTANCE_SENSOR_H
