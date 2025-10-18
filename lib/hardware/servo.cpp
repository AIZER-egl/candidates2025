#include "servo.h"

Servo::Servo() = default;

void Servo::begin(BinarySerializationData& data) {
	dataPtr = &data;

	pinMode(SERVO_PIN, OUTPUT_PWM);
	slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
	chan = pwm_gpio_to_channel(SERVO_PIN);
	pwm_set_clkdiv(slice_num, 125.0);
	pwm_set_wrap(slice_num, 20000);
	pwm_set_enabled(slice_num, true);

}

void Servo::tick() {
	if (millis() - previous_toggle >= 2000) {
		allow_toggle = true;
	}
}

void Servo::open() {
	if (!allow_toggle) return;
	if (servo_open) return;
	allow_toggle = false;
	servo_open = true;
	previous_toggle = millis();

	pwm_set_chan_level(slice_num, chan, 500);
}

void Servo::close() {
	if (!allow_toggle) return;
	if (!servo_open) return;
	allow_toggle = false;
	servo_open = false;
	previous_toggle = millis();

	pwm_set_chan_level(slice_num, chan, 1500);
}

