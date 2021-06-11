#pragma once

void initialize();
void finalize();

void pollEvents();
bool shouldQuit();

void setWindowTitle(const char* title);
void drawPixels(int width, int height, const void* data);

enum KeyCode {
    KEY_LEFT = 37,
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_A = 'A',
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
};

bool getKey(enum KeyCode keycode);
