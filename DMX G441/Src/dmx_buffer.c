#include "dmx_buffer.h"

static char dmxBuffer[4][512] = {{0}};
static dmx_changeCallback dmxCallbacks[4] = {0};

void DMX_In(char universe, char *buffer, short length) {
    char changed = 0;

    if (universe > 3)
        return;

    if (length > 512)
        return;

    for (short i = 0; i < length; i++) {
        if (dmxBuffer[universe][i] != buffer[i]) {
            changed = 1;
        }

        dmxBuffer[universe][i] = buffer[i];
    }

    if (changed != 0 && dmxCallbacks[universe] != 0) {
        dmxCallbacks[universe](universe, dmxBuffer[universe]);
    }
}

void DMX_RegisterCallback(char universe, dmx_changeCallback callback) {
    if (universe > 3)
        return;

    dmxCallbacks[universe] = callback;
}