#ifndef BUTTON_H
#define BUTTON_H

#include "../../software/binarySerializationData.h"

enum State {
    NONE,
    PRESSED,
    HOLD
};

class ActionButton {
public:
    void begin();

    void tick();

    State getState();

private:
    BinarySerializationData* data;
    State state;
    uint32_t sinceKeydown;
    uint32_t sinceKeyup;
    uint32_t buttonTimestamp;
    bool physicalButton;
    bool previousPhysicalButton;
};

#endif //BUTTON_H