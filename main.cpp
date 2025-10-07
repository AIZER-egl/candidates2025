/**
* cd ~/Desktop/pico-lib/build
 * make
 *
 * cp ~/Documents/RoboticProjects/RCJ_SOCCER_OPEN_2025/rpi_pico/build/main.uf2 /media/iker/RPI-RP2/main.uf2
*/
#include <iostream>
#include <string>
#include <cstdio>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "lib/hardware/action_button.h"
#include "pico-lib/gpio.h"
#include "lib/software/serializer.h"
#include "lib/software/pid.h"
#include "lib/software/hex.h"
#include "lib/hardware/kicker.h"
#include "lib/hardware/motor.h"
#include "lib/hardware/compass_classes.h"
#include "lib/hardware/light_sensor.h"
#include "lib/hardware/oled.h"
#include "lib/software/knn.h"
#include "lib/software/calibrations.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#define SDA 4
#define SCL 5
#define ARDUINO_I2C 8
#define ACTION_BUTTON 28

#define ROBOT_ON true
#define ROBOT_OFF false

int main () {
	stdio_init_all();
	i2c_init(OLED_I2C_INSTANCE, 400 * 1000);
	gpio_set_function(SDA, GPIO_FUNC_I2C);
	gpio_set_function(SCL, GPIO_FUNC_I2C);

	delay(2000);

	BinarySerializationData data{};
	Knn knn{};
	Light_Sensor light_sensor;
	Kicker kicker;
	Motor motor;
	Adafruit_BNO055 bno;

	light_sensor.begin(data);
	// kicker.begin(data);
	motor.begin(data);
	bno.begin(I2C_PORT_0, 100 * 1000, SDA, SCL, data);
	knn.upload_dataset(dataset);

	pinMode(BUILTIN_LED, OUTPUT);
	digitalWrite(BUILTIN_LED, HIGH);

	unsigned long long previous_motor = 0;
	int i = 0;
	for (;;) {
		motor.tick();
		bno.tick();
		// kicker.tick();

		if (millis() - previous_motor >= 5) {
			motor.forward(0, 90, bno.yaw);
			std::cout << "distance: " << motor.get_distance() << std::endl;
			previous_motor = millis();
		}
	}
}

#pragma clang diagnostic pop