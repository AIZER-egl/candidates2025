#ifndef BUTTON_H
#define BUTTON_H

#define BOTON 21

#include "../software/binarySerializationData.h"
#include "pico/stdlib.h"

class ActionButton {
public:

	enum State {
		NONE,
		PRESSED,
		HOLD
	};

	void begin(BinarySerializationData& data);

	void tick();

	State getState();
private:
	BinarySerializationData* dataPtr{};
	State state{};
	uint32_t sinceKeydown{};
	uint32_t sinceKeyup{};
	uint32_t buttonTimestamp{};
	bool physicalButton{};
	bool previousPhysicalButton{};
};

#endif //BUTTON_H