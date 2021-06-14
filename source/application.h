#pragma once
#include "keycode.h"

namespace app {

void initialize();
void finalize();

void pollEvents();
bool shouldQuit();

void setWindowTitle(const char* title);
void drawPixels(int width, int height, const void* data);

bool getKey(enum KeyCode keycode);

}  // namespace app
