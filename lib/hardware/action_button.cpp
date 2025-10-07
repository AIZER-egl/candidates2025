#include <stdio.h>
#include "pico/stdlib.h"
#include "action_button.h"

void ActionButton::begin() {
	gpio_init(BOTON);
	gpio_set_dir(BOTON, GPIO_IN);
	state = NONE;
	sinceKeydown = 0;
	sinceKeyup = 0;
	buttonTimestamp = 0;
	previousPhysicalButton = false;
	physicalButton = false;
}

void ActionButton::tick() {
	uint32_t now = time_us_64() / 1000; // compute now each tick

	if (now - buttonTimestamp >= 5) {
		physicalButton = gpio_get(BOTON);
		bool button_just_pressed = physicalButton && !previousPhysicalButton;
		bool button_just_released = !physicalButton && previousPhysicalButton;

		if (state != NONE) state = NONE;

		if (button_just_pressed) {
			sinceKeydown = now;
			sinceKeyup = 0;
		} else if (button_just_released) {
			if (now - sinceKeydown >= 1000) {
				state = HOLD;
			} else if (now - sinceKeydown >= 20) {
				state = PRESSED;
			}

			sinceKeydown = 0;
			sinceKeyup = now;
		}

		previousPhysicalButton = physicalButton;
		buttonTimestamp = now;
	}
}

ActionButton::State ActionButton::getState() {
	State original_state = state;
	if (state != NONE) state = NONE;
	return original_state;
}