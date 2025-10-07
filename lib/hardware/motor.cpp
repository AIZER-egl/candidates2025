#include "motor.h"

volatile unsigned long pulses_FR;
volatile unsigned long pulses_RR;
volatile unsigned long pulses_FL;
volatile unsigned long pulses_RL;

unsigned long long previous_pulses_RR_timestamp;
unsigned long long previous_pulses_FR_timestamp;
unsigned long long previous_pulses_RL_timestamp;
unsigned long long previous_pulses_FL_timestamp;

Motor::Motor() : dataPtr(nullptr) {
	motorFR.pwm_pin     = MOTOR_FR_PWM;
	motorFR.dir_pin     = MOTOR_FR_DIR;
	motorFR.encoder_pin = MOTOR_FR_ENC;

	motorRR.pwm_pin     = MOTOR_RR_PWM;
	motorRR.dir_pin     = MOTOR_RR_DIR;
	motorRR.encoder_pin = MOTOR_RR_ENC;

	motorFL.pwm_pin     = MOTOR_FL_PWM;
	motorFL.dir_pin     = MOTOR_FL_DIR;
	motorFL.encoder_pin = MOTOR_FL_ENC;

	motorRL.pwm_pin     = MOTOR_RL_PWM;
	motorRL.dir_pin     = MOTOR_RL_DIR;
	motorRL.encoder_pin = MOTOR_RL_ENC;

	motorRR.rpsPID.kp = 5;
	motorRR.rpsPID.ki = 50;
	motorRR.rpsPID.kd = 0;
	motorRR.rpsPID.max_output = MAX_SPEED;
	motorRR.rpsPID.error_threshold = 0;
	motorRR.rpsPID.one_direction_only = true;

	motorRL.rpsPID.kp = 5;
	motorRL.rpsPID.ki = 50;
	motorRL.rpsPID.kd = 0;
	motorRL.rpsPID.max_output = MAX_SPEED;
	motorRL.rpsPID.error_threshold = 0;
	motorRL.rpsPID.one_direction_only = true;

	motorFL.rpsPID.kp = 5;
	motorFL.rpsPID.ki = 50;
	motorFL.rpsPID.kd = 0;
	motorFL.rpsPID.max_output = MAX_SPEED;
	motorFL.rpsPID.error_threshold = 0;
	motorFL.rpsPID.one_direction_only = true;

	motorFR.rpsPID.kp = 5;
	motorFR.rpsPID.ki = 50;
	motorFR.rpsPID.kd = 0;
	motorFR.rpsPID.max_output = MAX_SPEED;
	motorFR.rpsPID.error_threshold = 0;
	motorFR.rpsPID.one_direction_only = true;

	rotationPID.kp = 0.05;
	rotationPID.ki = 0.15;
	rotationPID.kd = 0;
	rotationPID.max_output = 3;
	rotationPID.error_threshold = 3;
	rotationPID.reset_within_threshold = true;
	rotationPID.one_direction_only = false;
};

void Motor::begin (BinarySerializationData& data) {
	dataPtr = &data;

	motorFR.id = MOTOR_FR;
	motorRR.id = MOTOR_RR;
	motorFL.id = MOTOR_FL;
	motorRL.id = MOTOR_RL;

	pinMode(motorFR.dir_pin, OUTPUT);
	pinMode(motorFR.pwm_pin, OUTPUT_PWM);
	pinMode(motorFL.dir_pin, OUTPUT);
	pinMode(motorFL.pwm_pin, OUTPUT_PWM);
	pinMode(motorRR.dir_pin, OUTPUT);
	pinMode(motorRR.pwm_pin, OUTPUT_PWM);
	pinMode(motorRL.dir_pin, OUTPUT);
	pinMode(motorRL.pwm_pin, OUTPUT_PWM);

	interrupts(
		{ motorFR.encoder_pin, motorRR.encoder_pin, motorFL.encoder_pin, motorRL.encoder_pin },
		[] (const unsigned int gpio, const unsigned long events) {
			Motor::individualMotor::callback(gpio, events);
		}
	);

	motorFR.name = "FR";
	motorFL.name = "FL";
	motorRR.name = "RR";
	motorRL.name = "RL";
}

void Motor::tick() {
	if (!dataPtr) return;

	if (millis() - previousTick >= 5) {
		pulse_sum += pulses_FR + pulses_FL + pulses_RL + pulses_RR;
		motorFR.getRPS();
		motorFL.getRPS();
		motorRR.getRPS();
		motorRL.getRPS();

		// dataPtr -> motor_nw_rps = static_cast<float>(motorNW.rps);
		// dataPtr -> motor_ne_rps = static_cast<float>(motorNE.rps);
		// dataPtr -> motor_sw_rps = static_cast<float>(motorSW.rps);
		// dataPtr -> motor_se_rps = static_cast<float>(motorSE.rps);

		previousTick = millis();

		if (robot_stopped) stop();
	}
}

void Motor::individualMotor::setSpeed(int16_t new_speed, bool save_direction) {
	new_speed = std::clamp(new_speed, static_cast<int16_t>(-MAX_SPEED), static_cast<int16_t>(MAX_SPEED));

	if (std::abs(new_speed) < MIN_SPEED) {
		if (std::abs(new_speed) < (MIN_SPEED / 2)) {
			new_speed = 0;
		} else {
			new_speed = MIN_SPEED * (new_speed > 0 ? 1 : -1);
		}
	}

	analogWrite(pwm_pin, std::abs(new_speed));

	if (dir_pin == MOTOR_FL_DIR) {
		digitalWrite(dir_pin, new_speed < 0);
	} else {
		digitalWrite(dir_pin, new_speed > 0);
	}

	if (save_direction) {
		speed = std::abs(new_speed);
		if (new_speed != 0) direction = new_speed > 0 ? 1 : -1;
		// If direction is equal to 0, direction will remain unchanged
		// (asumming a PID controller is probably doing some things with speed 0 to keep things working well)
	}
}

void Motor::individualMotor::getRPS() {
	unsigned long pulses;
	unsigned long long previous_pulses_timestamp;
	switch (id) {
		case MOTOR_RR: pulses = pulses_RR; previous_pulses_timestamp = previous_pulses_RR_timestamp; break;
		case MOTOR_RL: pulses = pulses_RL; previous_pulses_timestamp = previous_pulses_RL_timestamp; break;
		case MOTOR_FR: pulses = pulses_FR; previous_pulses_timestamp = previous_pulses_FR_timestamp; break;
		case MOTOR_FL: pulses = pulses_FL; previous_pulses_timestamp = previous_pulses_FL_timestamp; break;
		default: pulses = 0; previous_pulses_timestamp = 0; break;
	}

	previous_rps = rps;
	const double elapsed_us = (micros() - previous_pulses_timestamp);
	if (elapsed_us > 0)
		rps = (pulses * (SECOND * 1000)) / (GEAR_RATIO * TICKS_PER_REVOLUTION_ENCODER * elapsed_us);
	else
		rps = 0;

	if (rps > 18) rps = previous_rps; // SW Motor, Robot 1 encoder is failing, using this guard to prevent making things worse

	rps_summatory += rps;
	rps_count += 1;

	switch (id) {
		case MOTOR_RR: pulses_RR = 0; previous_pulses_RR_timestamp = micros(); break;
		case MOTOR_RL: pulses_RL = 0; previous_pulses_RL_timestamp = micros(); break;
		case MOTOR_FR: pulses_FR = 0; previous_pulses_FR_timestamp = micros(); break;
		case MOTOR_FL: pulses_FL = 0; previous_pulses_FL_timestamp = micros(); break;
		default: break;
	}
}

void Motor::individualMotor::callback(const unsigned int gpio, unsigned long events) {
	switch (gpio) {
		case MOTOR_RR_ENC: pulses_RR++; break;
		case MOTOR_RL_ENC: pulses_RL++; break;
		case MOTOR_FR_ENC: pulses_FR++; break;
		case MOTOR_FL_ENC: pulses_FL++; break;
		default: break;
	}
}

double Motor::individualMotor::getRPS_average() {
	if (rps_count == 0) return 0;
	return rps_summatory / rps_count;
}

void Motor::stop() {
	if (motorRL.rps == 0 && motorRR.rps == 0 && motorFR.rps == 0 && motorFL.rps == 0) return;

	if (!robot_stopped) robot_stopped = true;
	dataPtr -> robot_speed = 0;
	dataPtr -> robot_direction = 0;

	if (motorRL.speed != 0) motorRL.setSpeed(-MAX_SPEED * motorRL.speed / std::abs(motorRL.speed), false);
	if (motorRR.speed != 0) motorRR.setSpeed(-MAX_SPEED * motorRR.speed / std::abs(motorRR.speed), false);
	if (motorFL.speed != 0) motorFL.setSpeed(-MAX_SPEED * motorFL.speed / std::abs(motorFL.speed), false);
	if (motorFR.speed != 0) motorFR.setSpeed(-MAX_SPEED * motorFR.speed / std::abs(motorFR.speed), false);

	if (motorRL.rps < 1.5 || motorRL.rps > (motorRL.previous_rps + 0.2)) motorRL.setSpeed(0);
	if (motorRR.rps < 1.5 || motorRR.rps > (motorRR.previous_rps + 0.2)) motorRR.setSpeed(0);
	if (motorFL.rps < 1.5 || motorFL.rps > (motorFL.previous_rps + 0.2)) motorFL.setSpeed(0);
	if (motorFR.rps < 1.5 || motorFR.rps > (motorFR.previous_rps + 0.2)) motorFR.setSpeed(0);

	PID::reset(motorRL.rpsPID);
	PID::reset(motorFR.rpsPID);
	PID::reset(motorRR.rpsPID);
	PID::reset(motorFL.rpsPID);

	// robot_stopped flag is automatically set to false when move method is called
}

void Motor::individualMotor::move(const double new_rps) {
	if (std::abs(new_rps) < 0.1) {
		setSpeed(0);
		return;
	}

	const double current_signed_rps = rps * static_cast<double>(direction);
    rpsPID.error = new_rps - current_signed_rps;
	rpsPID.target = new_rps;

	if (new_rps > 0) rpsPID.accept_type = PID_ACCEPT_POSITIVES_ONLY;
	else rpsPID.accept_type = PID_ACCEPT_NEGATIVES_ONLY;

	PID::compute(rpsPID);

	setSpeed(static_cast<int16_t>(rpsPID.output));
}

void Motor::forward(float rps, float facing_target, float facing_current) {
	if (robot_stopped) robot_stopped = false;

	rotationPID.target = facing_target;
	rotationPID.error = facing_target - facing_current;
	PID::compute(rotationPID);

	motorRR.move(rps - rotationPID.output);
	motorFR.move(rps - rotationPID.output);
	motorRL.move(rps + rotationPID.output);
	motorFL.move(rps + rotationPID.output);
}

float Motor::get_distance() {
	float pulse_average = pulse_sum / 4;
	float pulses_per_rotation = GEAR_RATIO * TICKS_PER_REVOLUTION_ENCODER;
	float distance_per_rotation = PI * WHEEL_DIAMETER;
	float rotations = pulse_average / pulses_per_rotation;

	return distance_per_rotation * rotations;
}
