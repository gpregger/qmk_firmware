#include "quantum.h"
#include "backlight.h"

#define BACKLIGHT_STEP BACKLIGHT_LIMIT_VAL / BACKLIGHT_LEVELS

void backlight_init_ports(void) {
    DDRC |= _BV(5) | _BV(6);
    TCCR3A = _BV(COM3A1) | _BV(COM3B1) | _BV(COM3B0);
    TCCR3B = _BV(WGM33) | _BV(CS30);
    // ICR1 = 31250; // 31250 -> 1Hz  Sets frequency with prescaler as f=F_CPU/PRESCALE/ICR/2
    ICR3 = 250; // 250 -> 32kHz w PRESCALE_1 Sets frequency with prescaler as f=F_CPU/PRESCALE/ICR/2

    TCNT3 = 0;         // reset counter
}
void backlight_set(uint8_t level) {
    uint8_t duty = level * BACKLIGHT_STEP;
    OCR3A = duty;
    OCR3B =  ICR3 - OCR3A;
}