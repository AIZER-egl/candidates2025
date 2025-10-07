#ifndef PICO_LIB_MOTOR_H
#define PICO_LIB_MOTOR_H

#include <algorithm>
#include <cstdint>

#include "../software/pid.h"
#include "../../pico-lib/time.h"
#include "../../pico-lib/usb.h"
#include "../../pico-lib/gpio.h"
#include "../software/binarySerializationData.h"

#define MOTOR_FR_DIR 3
#define MOTOR_FR_PWM 2
#define MOTOR_FR_ENC 6

#define MOTOR_RR_DIR 8
#define MOTOR_RR_PWM 7
#define MOTOR_RR_ENC 9

#define MOTOR_RL_DIR 10
#define MOTOR_RL_PWM 11
#define MOTOR_RL_ENC 12

#define MOTOR_FL_DIR 13
#define MOTOR_FL_PWM 14
#define MOTOR_FL_ENC 15

#define MOTOR_FR 0b0000
#define MOTOR_RR 0b0001
#define MOTOR_FL 0b0010
#define MOTOR_RL 0b0011

#define TICKS_PER_REVOLUTION_ENCODER 12
#define GEAR_RATIO 9.68
#define WHEEL_DIAMETER 65

#define MAX_SPEED 255
#define MIN_SPEED 15

#define MAX_RPS 18
#define MIN_RPS 0

#define PI 3.141592

class Motor {
public:
	Motor();

	struct individualMotor {
		std::string name;
		uint8_t id{};
		int8_t direction{};
		uint16_t speed{};
		double rps{};
		double previous_rps{};

		void setSpeed(int16_t new_speed, bool save_direction = true);
		void move(double new_rps);

		static void callback (unsigned int gpio, unsigned long events);

		void getRPS ();
		double getRPS_average ();

		PID::PidParameters rpsPID;

		int pwm_pin{};
		int dir_pin{};
		uint8_t encoder_pin{};

		double rps_summatory = 0;
		unsigned long long rps_count = 0;
		unsigned long long lastIteration_ms = 0;
		double previousOut{};
	};

	individualMotor motorFR;
	individualMotor motorRR;
	individualMotor motorFL;
	individualMotor motorRL;

	PID::PidParameters rotationPID;

	void begin (BinarySerializationData& data);
	void stop ();
	void tick ();

	void forward (float rps, float facing_target, float facing_current);
	float get_distance();

	float current_direction{};
private:
	int pulse_sum = 0;
	bool robot_stopped = false;
	BinarySerializationData* dataPtr;
	unsigned long long previousTick{};
};

#endif //PICO_LIB_MOTOR_H