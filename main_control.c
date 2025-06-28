#include "stm32f1xx_hal.h"
#include <stdbool.h>

#define PRESSURE_SETPOINT   50.0f    // psi
#define RELIEF_PRESSURE     80.0f    // psi
#define PWM_MIN             1000     // us
#define PWM_MAX             2000     // us

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim3; // PWM for actuator

float read_pressure(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint32_t adc = HAL_ADC_GetValue(&hadc1);
    float voltage = (adc * 3.3f) / 4095.0f;
    return (voltage / 5.0f) * 100.0f; // scale to psi
}

void set_actuator_position(float percent)
{
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    uint16_t pulse = PWM_MIN + (PWM_MAX - PWM_MIN) * (percent / 100.0f);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
}

void control_loop(void)
{
    float pressure = read_pressure();

    if (pressure > RELIEF_PRESSURE)
        set_actuator_position(100);  // fully open
    else if (pressure < PRESSURE_SETPOINT)
        set_actuator_position(0);    // close
    else {
        float range = RELIEF_PRESSURE - PRESSURE_SETPOINT;
        float percent = ((pressure - PRESSURE_SETPOINT) / range) * 100.0f;
        set_actuator_position(percent);
    }
}
