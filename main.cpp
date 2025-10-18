/**
* cd ~/Desktop/pico-lib/build
 * make
 *
 * cp ~/Documents/RoboticProjects/RCJ_SOCCER_OPEN_2025/rpi_pico/build/main.uf2 /media/iker/RPI-RP2/main.uf2
*/
#include <iostream>
#include <string>
#include <cstdio>
#include <iomanip>
#include <stdio.h>

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
#include "lib/hardware/distance_sensor.h"
#include "lib/hardware/light_sensor.h"
#include "lib/hardware/oled.h"
#include "lib/hardware/servo.h"
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

void send_data(BinarySerializationData& data) {
	std::vector<uint8_t> bytes = Serializer::serialize(data);

	if (!bytes.empty()) {
		size_t bytes_written = fwrite(
			bytes.data(),
			sizeof(uint8_t),
			bytes.size(),
			stdout
		);

		fflush(stdout);
	}
}

void print_buffer(const std::vector<uint8_t>& buffer) {
	// Set stream to output hexadecimal numbers
	std::cout << std::hex
			  // Pad with '0' instead of spaces
			  << std::setfill('0');

	for (uint8_t byte : buffer) {
		// setw(2) ensures two characters are printed (e.g., 07 instead of 7)
		// static_cast<int> is crucial to prevent cout from treating the byte as a character
		std::cout << std::setw(2) << static_cast<int>(byte) << " ";
	}

	// It's good practice to reset the stream to its default state (decimal)
	std::cout << std::dec << std::endl;
}

int main () {
	stdio_set_translate_crlf(&stdio_usb, false);
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
	Servo servo;
	ActionButton action_button;
	Distance_Sensor distance_sensor;

	light_sensor.begin(data);
	kicker.begin(data);
	motor.begin(data);
	servo.begin(data);
	action_button.begin(data);
	distance_sensor.begin(data);
	bno.begin(I2C_PORT_0, 100 * 1000, SDA, SCL, data);

	knn.upload_dataset(dataset);
	servo.open();

	pinMode(BUILTIN_LED, OUTPUT);
	digitalWrite(BUILTIN_LED, HIGH);

	std::vector<uint8_t> message = {};
	unsigned long long previous_prediction = 0;
	unsigned long long previous_motor = 0;
	unsigned long long previous_led = 0;
	unsigned int i = 0;
	bool led_state = HIGH;
	int led_rate = 500;
	for (;;) {
		motor.tick();
		bno.tick();
		light_sensor.tick();
		action_button.tick();
		kicker.tick();
		servo.tick();
		distance_sensor.tick();

		ActionButton::State button_state = action_button.getState();

		if (button_state == ActionButton::State::PRESSED) {
			data.start = !data.start;
			bno.setYawOffset(bno.raw_yaw);
			PID::reset(motor.orientation_pid);
			PID::reset(motor.motorFL.rpsPID);
			PID::reset(motor.motorFR.rpsPID);
			PID::reset(motor.motorRL.rpsPID);
			PID::reset(motor.motorRR.rpsPID);

			if (data.start) {
				led_rate = 100;
			} else {
				led_rate = 500;
			}
		}

		if ((millis() - previous_led) >= led_rate) {
			led_state = !led_state;
			digitalWrite(BUILTIN_LED, led_state);

			previous_led = millis();
		}

		if ((millis() - previous_prediction) >= 50) {
			data.front_wall = distance_sensor.front;
			data.left_wall = distance_sensor.left;
			data.right_wall = distance_sensor.right;

			auto green = static_cast<float>(light_sensor.green());
			auto red = static_cast<float>(light_sensor.red());
			auto orange = static_cast<float>(light_sensor.orange());
			auto blue = static_cast<float>(light_sensor.blue());
			auto white = static_cast<float>(light_sensor.white_average());
			std::string current_color = knn.predict({green, red, orange, blue, white});

			data.floor_color = Knn::color_to_int(current_color);
			previous_prediction = millis();
			// std::cout << "Front:" << data.front_wall << "Right:" << data.right_wall << "Left:" << data.left_wall << std::endl;
		}

		if ((millis() - previous_motor) >= 15) {
			if (data.motor_move_form == 0 || !data.start) {
				motor.stop();
			} else if(data.motor_move_form == 1) {
				motor.rotate(data.motor_facing_target, bno.yaw);
			} else if(data.motor_move_form == 2) {
				motor.forward(data.motor_speed_target / 10, data.motor_facing_target, bno.yaw);
			} else {
				motor.stop();
			}

			data._test_pid_error_sum = motor.motorFR.rpsPID.integral_error;
			previous_motor = millis();
		}

		int received_char;
		while ((received_char = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
			char byte = (char)received_char;
			message.push_back(byte);

			if (message.size() == BINARY_SERIALIZATION_DATA_SIZE) {
				std::optional<BinarySerializationData> received_data = Serializer::deserialize(message);

				if (received_data.has_value()) {
					data.motor_speed_target = received_data.value().motor_speed_target;
					data.motor_facing_target = received_data.value().motor_facing_target;
					data.motor_move_form = received_data.value().motor_move_form;
					data.servo_active = received_data.value().servo_active;
					data.kicker_active = received_data.value().kicker_active;
					data.reset_distance = received_data.value().reset_distance;

					if (data.servo_active) {
						servo.open();
					} else {
						servo.close();
					}

					if (data.kicker_active) {
						kicker.kick();
					}

					if (data.reset_pid) {
						PID::reset(motor.orientation_pid);
						PID::reset(motor.rotation_pid);
						PID::reset(motor.motorFL.rpsPID);
						PID::reset(motor.motorFR.rpsPID);
						PID::reset(motor.motorRL.rpsPID);
						PID::reset(motor.motorRR.rpsPID);
					}
				}

				data.reset_pid = false;
				data.reset_distance = false;
				data.kicker_active = false;
				send_data(data);

				message.clear();
			}

			if (message.size() > 100) {
				message.clear();
			}
		}
	}
}

#pragma clang diagnostic pop