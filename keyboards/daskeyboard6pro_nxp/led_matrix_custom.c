#include "led_matrix_custom.h"
#include <nxp/imxrt.h>
#include <nxp/pins_arduino.h>

#define MOD_VALUE 5000

#define PORT_CONFIG_42 ((volatile uint32_t *)0x401F81C8)
#define PORT_CONFIG_43 ((volatile uint32_t *)0x401F81C4)
#define PORT_CONFIG_44 ((volatile uint32_t *)0x401F81C0)
#define PORT_CONFIG_45 ((volatile uint32_t *)0x401F81BC)
#define PORT_CONFIG_46 ((volatile uint32_t *)0x401F81D0)
#define PORT_CONFIG_47 ((volatile uint32_t *)0x401F81CC)

void initChannels(void);
void setUpChannel(uint8_t, float);
void setDutyCycle(uint8_t, float);

void pwm_init(void);
void flexpwm_init(IMXRT_FLEXPWM_t*);
void quadtimer_init(IMXRT_TMR_t*);

typedef struct gobmatrix_driver_t {
    uint8_t pwm_buffer[GOBMATRIX_PWM_CHANNEL_COUNT];
    bool    pwm_buffer_dirty;
} PACKED gobmatrix_driver_t;

gobmatrix_driver_t driver_buffers = {
    .pwm_buffer               = {0},
    .pwm_buffer_dirty         = false,
};

void gobmatrix_write_pwm_buffer(void) {
    for (int i = 0; i < GOBMATRIX_PWM_CHANNEL_COUNT; i++) {
        setDutyCycle(i, driver_buffers.pwm_buffer[i]/255.0);
    }
}

void gobmatrix_set_value(int index, uint8_t value) {
    if (index >= 0 && index < GOBMATRIX_PWM_CHANNEL_COUNT) {
        if (driver_buffers.pwm_buffer[index] == value) {
            return;
        }

        driver_buffers.pwm_buffer[index] = value;
        driver_buffers.pwm_buffer_dirty  = true;
    }
}

void gobmatrix_set_value_all(uint8_t value) {
    for (int i = 0; i < GOBMATRIX_PWM_CHANNEL_COUNT; i++) {
        gobmatrix_set_value(i, value);
    }
}

void gobmatrix_init(void) {
    pwm_init();

    FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(1 << 0) | FLEXPWM_MCTRL_CLDOK(1 << 1) | FLEXPWM_MCTRL_CLDOK(1 << 2);
    FLEXPWM1_SM0CTRL2 |= FLEXPWM_SMCTRL2_INDEP;
    FLEXPWM1_SM0CTRL  |= FLEXPWM_SMCTRL_FULL | FLEXPWM_SMCTRL_COMPMODE;
    FLEXPWM1_SM1CTRL2 |= FLEXPWM_SMCTRL2_INDEP;
    FLEXPWM1_SM1CTRL  |= FLEXPWM_SMCTRL_FULL | FLEXPWM_SMCTRL_COMPMODE;
    FLEXPWM1_SM2CTRL2 |= FLEXPWM_SMCTRL2_INDEP;
    FLEXPWM1_SM2CTRL  |= FLEXPWM_SMCTRL_FULL | FLEXPWM_SMCTRL_COMPMODE;
    FLEXPWM1_OUTEN = 0x0FF0;
    FLEXPWM1_MASK = 0;

    initChannels();
    for (int i = 0; i < GOBMATRIX_PWM_CHANNEL_COUNT; i++) {
        setUpChannel(i, 0);
    }

    FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(1 << 0) | FLEXPWM_MCTRL_LDOK(1 << 1) | FLEXPWM_MCTRL_LDOK(1 << 2);

    *PORT_CONFIG_42 = 1;
    *PORT_CONFIG_43 = 1;
    *PORT_CONFIG_44 = 1;
    *PORT_CONFIG_45 = 1;
    *PORT_CONFIG_46 = 1;
    *PORT_CONFIG_47 = 1;

    // Set all PWM values to zero
    gobmatrix_set_value_all(0);
}

void gobmatrix_update_pwm_buffers(void) {
    if (driver_buffers.pwm_buffer_dirty) {
        gobmatrix_write_pwm_buffer();
        driver_buffers.pwm_buffer_dirty = false;
    }
}

const led_matrix_driver_t led_matrix_driver = {
    .init          = gobmatrix_init,
    .flush         = gobmatrix_update_pwm_buffers,
    .set_value     = gobmatrix_set_value,
    .set_value_all = gobmatrix_set_value_all,
};

void initChannels() {
    FLEXPWM1_SM1INIT = 0;
    FLEXPWM1_SM0INIT = 0;
    FLEXPWM1_SM2INIT = 0;
    FLEXPWM1_SM1VAL1 = MOD_VALUE;
    FLEXPWM1_SM1VAL0 = MOD_VALUE;
    FLEXPWM1_SM0VAL1 = MOD_VALUE;
    FLEXPWM1_SM0VAL0 = MOD_VALUE;
    FLEXPWM1_SM2VAL1 = MOD_VALUE;
    FLEXPWM1_SM2VAL0 = MOD_VALUE;
}

// 0 < phaseShift < 1
void setUpChannel(uint8_t channel, float phaseShift) {
    switch (channel){
        case 0:
            FLEXPWM1_SM1VAL4 = (uint16_t)(phaseShift * MOD_VALUE);
            return;
        case 1:
            FLEXPWM1_SM1VAL2 = (uint16_t)(phaseShift * MOD_VALUE);
            return;
        case 2:
            FLEXPWM1_SM0VAL4 = (uint16_t)(phaseShift * MOD_VALUE);
            return;
        case 3:
            FLEXPWM1_SM0VAL2 = (uint16_t)(phaseShift * MOD_VALUE);
            return;
        case 4:
            FLEXPWM1_SM2VAL4 = (uint16_t)(phaseShift * MOD_VALUE);
            return;
        case 5:
            FLEXPWM1_SM2VAL2 = (uint16_t)(phaseShift * MOD_VALUE);
            return;
    }
}

// 0 < dutyCycle < 1
void setDutyCycle(uint8_t channel, float dutyCycle) {
    uint16_t pulseStart;
    uint16_t pulseEnd;
    switch (channel){
        case 5:
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(1 << 1);
            pulseStart = FLEXPWM1_SM1VAL4;
            pulseEnd = pulseStart + (uint16_t)(dutyCycle * MOD_VALUE);
            FLEXPWM1_SM1VAL5 = pulseEnd;
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(1 << 1);
            break;
        case 0:
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(1 << 1);
            pulseStart = FLEXPWM1_SM1VAL2;
            pulseEnd = pulseStart + (uint16_t)(dutyCycle * MOD_VALUE);
            FLEXPWM1_SM1VAL3 = pulseEnd;
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(1 << 1);
            break;
        case 1:
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(1 << 0);
            pulseStart = FLEXPWM1_SM0VAL4;
            pulseEnd = pulseStart + (uint16_t)(dutyCycle * MOD_VALUE);
            FLEXPWM1_SM0VAL5 = pulseEnd;
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(1 << 0);
            break;
        case 2:
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(1 << 0);
            pulseStart = FLEXPWM1_SM0VAL2;
            pulseEnd = pulseStart + (uint16_t)(dutyCycle * MOD_VALUE);
            FLEXPWM1_SM0VAL3 = pulseEnd;
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(1 << 0);
            break;
        case 3:
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(1 << 2);
            pulseStart = FLEXPWM1_SM2VAL4;
            pulseEnd = pulseStart + (uint16_t)(dutyCycle * MOD_VALUE);
            FLEXPWM1_SM2VAL5 = pulseEnd;
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(1 << 2);
            break;
        case 4:
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_CLDOK(1 << 2);
            pulseStart = FLEXPWM1_SM2VAL2;
            pulseEnd = pulseStart + (uint16_t)(dutyCycle * MOD_VALUE);
            FLEXPWM1_SM2VAL3 = pulseEnd;
            FLEXPWM1_MCTRL |= FLEXPWM_MCTRL_LDOK(1 << 2);
            break;
    }
}

void pwm_init(void)
{
    //printf("pwm init\n");
    CCM_CCGR4 |= CCM_CCGR4_PWM1(CCM_CCGR_ON) | CCM_CCGR4_PWM2(CCM_CCGR_ON) |
        CCM_CCGR4_PWM3(CCM_CCGR_ON) | CCM_CCGR4_PWM4(CCM_CCGR_ON);
    CCM_CCGR6 |= CCM_CCGR6_QTIMER1(CCM_CCGR_ON) | CCM_CCGR6_QTIMER2(CCM_CCGR_ON) |
        CCM_CCGR6_QTIMER3(CCM_CCGR_ON) | CCM_CCGR6_QTIMER4(CCM_CCGR_ON);
    flexpwm_init(&IMXRT_FLEXPWM1);
    flexpwm_init(&IMXRT_FLEXPWM2);
    flexpwm_init(&IMXRT_FLEXPWM3);
    flexpwm_init(&IMXRT_FLEXPWM4);
    quadtimer_init(&IMXRT_TMR1);
    quadtimer_init(&IMXRT_TMR2);
    quadtimer_init(&IMXRT_TMR3);
}

void flexpwm_init(IMXRT_FLEXPWM_t *p)
{
    int i;

    p->FCTRL0 = FLEXPWM_FCTRL0_FLVL(15); // logic high = fault
    p->FSTS0 = 0x000F; // clear fault status
    p->FFILT0 = 0;
    p->MCTRL |= FLEXPWM_MCTRL_CLDOK(15);
    for (i=0; i < 4; i++) {
        p->SM[i].CTRL2 = FLEXPWM_SMCTRL2_INDEP | FLEXPWM_SMCTRL2_WAITEN
            | FLEXPWM_SMCTRL2_DBGEN;
        p->SM[i].CTRL = FLEXPWM_SMCTRL_FULL;
        p->SM[i].OCTRL = 0;
        p->SM[i].DTCNT0 = 0;
        p->SM[i].INIT = 0;
        p->SM[i].VAL0 = 0;
        p->SM[i].VAL1 = 33464;
        p->SM[i].VAL2 = 0;
        p->SM[i].VAL3 = 0;
        p->SM[i].VAL4 = 0;
        p->SM[i].VAL5 = 0;
    }
    p->MCTRL |= FLEXPWM_MCTRL_LDOK(15);
    p->MCTRL |= FLEXPWM_MCTRL_RUN(15);
}

void quadtimer_init(IMXRT_TMR_t *p)
{
    int i;

    for (i=0; i < 4; i++) {
        p->CH[i].CTRL = 0; // stop timer
        p->CH[i].CNTR = 0;
        p->CH[i].SCTRL = TMR_SCTRL_OEN | TMR_SCTRL_OPS | TMR_SCTRL_VAL | TMR_SCTRL_FORCE;
        p->CH[i].CSCTRL = TMR_CSCTRL_CL1(1) | TMR_CSCTRL_ALT_LOAD;
        // COMP must be less than LOAD - otherwise output is always low
        p->CH[i].LOAD = 24000;   // low time  (65537 - x) - 
        p->CH[i].COMP1 = 0;  // high time (0 = always low, max = LOAD-1)
        p->CH[i].CMPLD1 = 0;
        p->CH[i].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(8) |
            TMR_CTRL_LENGTH | TMR_CTRL_OUTMODE(6);
    }
}