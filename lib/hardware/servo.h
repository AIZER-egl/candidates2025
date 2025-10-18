#ifndef SERVO_H
#define SERVO_H

#include "../../pico-lib/time.h"
#include "../../pico-lib/gpio.h"
#include "../software/binarySerializationData.h"

#define SERVO_PIN 1

class Servo {
public:
	Servo();
	void begin(BinarySerializationData& data);
	void tick();
	void open();
	void close();

	bool servo_open = false;
private:
	bool allow_toggle = true;
	BinarySerializationData* dataPtr{};
	unsigned long long previous_toggle{};

	uint slice_num{};
	uint chan{};

};



#endif //SERVO_H
