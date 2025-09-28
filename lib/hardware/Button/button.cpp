#include <stdio.h>
#include "pico/stdlib.h"
#include "button.h"

#define BOTON 15

void ActionButton::begin() {
    gpio_init(BOTON);
    gpio_set_dir(BOTON, GPIO_IN);
    state = NONE;
    sinceKeydown = 0;
    sinceKeyup = 0;
    buttonTimestamp = 0;
    lastPhysicalButton = false;
    physicalButton = false;
}

void ActionButton::tick() {
    bool buttonPressed = gpio_get(BOTON);
    uint32_t now = time_us_64() / 1000; // compute now each tick

    if (now - buttonTimestamp >= 10) {
        buttonTimestamp = now;

        if (buttonPressed && !lastPhysicalButton) {
            // Button was just pressed
            if (now - sinceKeydown < 50) return;
            state = PRESSED;
            sinceKeydown = now;

        } else if (!buttonPressed && lastPhysicalButton) {
            // Button was just released
            state = NONE;
            sinceKeyup = now;

        } else if (buttonPressed && lastPhysicalButton) {
            // Button is being held down
            if (now - sinceKeydown >= 1000) state = HOLD;

        } else {
            // Button is not pressed
            state = NONE;
        }

        lastPhysicalButton = buttonPressed;
    }
}

State ActionButton::getState() {
    return state;
}
