#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
#include <string.h>

#include "mikos_input.h"

InputState g_input = {
    0, 0,
    0.0f, 0.0f,
    0.0f, 0.0f,
    0
};

static char pad_buffer[256]
    __attribute__((aligned(64)));

static unsigned int previous_buttons = 0;


/*
 * ============================================================
 * PAD INITIALIZATION STATE MACHINE
 * ============================================================
 *
 * Nic tutaj nie blokuje gry.
 *
 * Każde input_update() wykonuje najwyżej jeden etap.
 */

typedef enum {

    PAD_INIT_WAIT_STABLE = 0,

    PAD_INIT_REQUEST_ANALOG,

    PAD_INIT_WAIT_REQUEST,

    PAD_INIT_WAIT_ANALOG_STABLE,

    PAD_INIT_READY

} PadInitState;


static PadInitState g_pad_init_state =
    PAD_INIT_WAIT_STABLE;


/*
 * Kilka poprawnych analogowych pakietów zanim
 * faktycznie oddamy analogi Playerowi.
 */
static int g_analog_good_frames = 0;


/*
 * ============================================================
 * ANALOG
 * ============================================================
 */

static float normalize_axis(
    unsigned char raw
)
{
    const int DEADZONE = 24;

    int value;
    int abs_value;

    float result;

    value =
        (int)raw - 128;

    abs_value =
        value < 0
        ? -value
        : value;

    if (abs_value <= DEADZONE)
        return 0.0f;


    /*
     * Remap:
     *
     * deadzone -> 0
     * max      -> 1
     */

    if (value > 0) {

        result =
            (float)(
                value - DEADZONE
            ) /
            (float)(
                127 - DEADZONE
            );

    } else {

        result =
            (float)(
                value + DEADZONE
            ) /
            (float)(
                128 - DEADZONE
            );
    }


    if (result > 1.0f)
        result = 1.0f;

    if (result < -1.0f)
        result = -1.0f;

    return result;
}


static void clear_analog(void)
{
    g_input.left_x = 0.0f;
    g_input.left_y = 0.0f;

    g_input.right_x = 0.0f;
    g_input.right_y = 0.0f;

    g_input.analog_valid = 0;
}


static void clear_all_input(void)
{
    g_input.held = 0;
    g_input.pressed = 0;

    clear_analog();
}


/*
 * ============================================================
 * INIT
 * ============================================================
 */

int input_init(void)
{
    int ret;

    SifInitRpc(0);


    ret =
        SifLoadModule(
            "rom0:XSIO2MAN",
            0,
            NULL
        );

    if (ret < 0)
        return -1;


    ret =
        SifLoadModule(
            "rom0:XPADMAN",
            0,
            NULL
        );

    if (ret < 0)
        return -2;


    if (!padInit(0))
        return -3;


    memset(
        pad_buffer,
        0,
        sizeof(pad_buffer)
    );


    if (
        !padPortOpen(
            0,
            0,
            pad_buffer
        )
    ) {
        return -4;
    }


    previous_buttons = 0;

    g_pad_init_state =
        PAD_INIT_WAIT_STABLE;

    g_analog_good_frames = 0;

    clear_all_input();

    return 0;
}


/*
 * ============================================================
 * PAD STATE MACHINE
 * ============================================================
 */

static void update_pad_initialization(void)
{
    int state;
    unsigned char req;


    state =
        padGetState(
            0,
            0
        );


    switch (g_pad_init_state) {


        /*
         * ----------------------------------------------------
         * Czekamy aż padman znajdzie kontroler.
         * ----------------------------------------------------
         */

        case PAD_INIT_WAIT_STABLE:

            if (
                state ==
                PAD_STATE_STABLE
            ) {

                g_pad_init_state =
                    PAD_INIT_REQUEST_ANALOG;
            }

            break;


        /*
         * ----------------------------------------------------
         * Prośba o DualShock analog mode.
         * ----------------------------------------------------
         */

        case PAD_INIT_REQUEST_ANALOG:

            if (
                state !=
                PAD_STATE_STABLE
            ) {
                break;
            }


            if (
                padSetMainMode(
                    0,
                    0,
                    PAD_MMODE_DUALSHOCK,
                    PAD_MMODE_LOCK
                )
            ) {

                g_pad_init_state =
                    PAD_INIT_WAIT_REQUEST;

            } else {

                /*
                 * Spróbujemy ponownie w kolejnej
                 * klatce.
                 */

                g_pad_init_state =
                    PAD_INIT_WAIT_STABLE;
            }

            break;


        /*
         * ----------------------------------------------------
         * padSetMainMode jest asynchroniczne.
         * ----------------------------------------------------
         */

        case PAD_INIT_WAIT_REQUEST:

            req =
                padGetReqState(
                    0,
                    0
                );


            if (
                req ==
                PAD_RSTAT_COMPLETE
            ) {

                g_pad_init_state =
                    PAD_INIT_WAIT_ANALOG_STABLE;

            }

            else if (
                req ==
                PAD_RSTAT_FAILED
            ) {

                /*
                 * Nie zabijamy gry.
                 * Ponawiamy inicjalizację.
                 */

                g_pad_init_state =
                    PAD_INIT_WAIT_STABLE;
            }

            /*
             * BUSY:
             * nic nie robimy do kolejnej klatki.
             */

            break;


        /*
         * ----------------------------------------------------
         * Request zakończony, ale pad musi jeszcze
         * wrócić do STABLE.
         * ----------------------------------------------------
         */

        case PAD_INIT_WAIT_ANALOG_STABLE:

            if (
                state ==
                PAD_STATE_STABLE
            ) {

                g_analog_good_frames = 0;

                g_pad_init_state =
                    PAD_INIT_READY;
            }

            break;


        case PAD_INIT_READY:

        default:
            break;
    }
}


/*
 * ============================================================
 * FRAME UPDATE
 * ============================================================
 */

void input_update(void)
{
    struct padButtonStatus buttons;

    int state;
    unsigned int current;


    /*
     * Najpierw state machine.
     */

    if (
        g_pad_init_state !=
        PAD_INIT_READY
    ) {

        update_pad_initialization();

        clear_all_input();

        previous_buttons = 0;

        return;
    }


    state =
        padGetState(
            0,
            0
        );


    /*
     * Po odłączeniu kontrolera wracamy
     * do początku inicjalizacji.
     */

    if (
        state !=
        PAD_STATE_STABLE
    ) {

        clear_all_input();

        previous_buttons = 0;

        g_analog_good_frames = 0;

        g_pad_init_state =
            PAD_INIT_WAIT_STABLE;

        return;
    }


    memset(
        &buttons,
        0,
        sizeof(buttons)
    );


    if (
        padRead(
            0,
            0,
            &buttons
        ) == 0
    ) {

        clear_all_input();

        return;
    }


    /*
     * ========================================================
     * BUTTONS
     * ========================================================
     */

    current =
        0xFFFF ^
        buttons.btns;


    g_input.held =
        current;


    g_input.pressed =
        current &
        ~previous_buttons;


    previous_buttons =
        current;


    /*
     * ========================================================
     * ANALOG PACKET VALIDATION
     * ========================================================
     *
     * DualShock:
     *
     * 0x73 = analog
     * 0x79 = analog + pressure
     */

    if (
        buttons.mode != 0x73 &&
        buttons.mode != 0x79
    ) {

        clear_analog();

        g_analog_good_frames = 0;

        return;
    }


    /*
     * 0,0,0,0 jest dla naszego przypadku podejrzane.
     *
     * Fizycznie oznaczałoby oba sticki dokładnie
     * w lewym-górnym rogu jednocześnie.
     *
     * To był właśnie symptom wcześniejszych
     * niezainicjalizowanych danych.
     */

    if (
        buttons.ljoy_h == 0 &&
        buttons.ljoy_v == 0 &&
        buttons.rjoy_h == 0 &&
        buttons.rjoy_v == 0
    ) {

        clear_analog();

        g_analog_good_frames = 0;

        return;
    }


    /*
     * Dajemy padmanowi kilka kolejnych poprawnych
     * pakietów zanim zaczniemy ruszać graczem.
     */

    if (
        g_analog_good_frames <
        4
    ) {

        g_analog_good_frames++;

        clear_analog();

        return;
    }


    /*
     * ========================================================
     * ANALOG READY
     * ========================================================
     */

    g_input.left_x =
        normalize_axis(
            buttons.ljoy_h
        );


    g_input.left_y =
        normalize_axis(
            buttons.ljoy_v
        );


    g_input.right_x =
        normalize_axis(
            buttons.rjoy_h
        );


    g_input.right_y =
        normalize_axis(
            buttons.rjoy_v
        );


    g_input.analog_valid =
        1;
}
