#include "led_matrix.h"

#define GOBMATRIX_PWM_CHANNEL_COUNT 6

void gobmatrix_init(void);

void gobmatrix_set_value(int index, uint8_t value);

void gobmatrix_set_value_all(uint8_t value);

void gobmatrix_update_pwm_buffers(void);
