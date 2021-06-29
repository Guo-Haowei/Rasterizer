#include "application.h"
#include "common/core_assert.h"
#include "example_base.h"

void initialize() {
    g_pExample->initialize();
}

void finalize() {
}

void pollEvents() {
}

bool shouldQuit() {
    return false;
}

void setWindowTitle(const char* title) {
}

void drawPixels(int width, int height, const void* data) {
}

bool getKey(enum KeyCode keycode) {
    return false;
}
