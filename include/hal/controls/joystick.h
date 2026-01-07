#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>

typedef struct
{
    float x_norm;        // Normalized value X (-1.0 to 1.0)
    float y_norm;        // Normalized value Y (-1.0 to 1.0)
    bool button_pressed; // true, if pressed, otherwise false
} joystick_event_t;

void joystick_init_full_range(void);
void joystick_init_simple_center(void);
void joystick_read(joystick_event_t *event);

#endif // JOYSTICK_H