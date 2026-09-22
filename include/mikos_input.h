#ifndef MIKOSCRAFT_INPUT_H
#define MIKOSCRAFT_INPUT_H

typedef struct {

    unsigned int held;
    unsigned int pressed;

    float left_x;
    float left_y;

    float right_x;
    float right_y;

    int analog_valid;

} InputState;

extern InputState g_input;

int input_init(void);
void input_update(void);

#define INPUT_SELECT    0x0001
#define INPUT_START     0x0008

#define INPUT_UP        0x0010
#define INPUT_RIGHT     0x0020
#define INPUT_DOWN      0x0040
#define INPUT_LEFT      0x0080

#define INPUT_L2        0x0100
#define INPUT_R2        0x0200
#define INPUT_L1        0x0400
#define INPUT_R1        0x0800

#define INPUT_TRIANGLE  0x1000
#define INPUT_CIRCLE    0x2000
#define INPUT_CROSS     0x4000
#define INPUT_SQUARE    0x8000

#endif
