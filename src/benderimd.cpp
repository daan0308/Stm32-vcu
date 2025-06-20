/*
 * This file is part of the ZombieVerter project.
 *
 * Copyright (C) 2025 Daan Posthumus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "benderimd.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

void BenderIMD::DeInit() {
    StopTimer();
}

void BenderIMD::Task100Ms() {
    bool ok = gpio_get(GPIOC, GPIO3);

    uint16_t period = TIM_CCR3(TIM3);
    uint16_t high_time = TIM_CCR4(TIM3);

    // Calculate the duty cycle and frequency of the PWM pulse
    double duty_cycle;
    uint32_t frequency;

    // If period is 0, then the frequency and duty_cycle are also 0
    if (period == 0) {
        duty_cycle = 0;
        frequency = 0;
    }

    // If the period is not 0, then we calculate the duty cycle and frequency normally
    else {
        // Duty cycle is the time the duty is high vs the period
        duty_cycle = (double) high_time / (double) period;

        // For the determining the state of the IMD a frequency of either 0, 10, 20, 30, 40 or 50 Hz is expected,
        // so integer division is used to ignore small errors in the period.
        float timer_clk = rcc_apb1_frequency; // or apb2 if using another timer
        float freq = timer_clk / (TIM_PSC(TIM3) + 1);
        frequency = freq / period;
    }

    enum IMD_states state;
    uint16_t resistance = 50000;
    uint8_t fault_flag = 0;

    // Find the state of the IMD by checking the frequency of the PWM signal. Read the manual for more explanation how
    // the IR155-3204 works
    switch (frequency) {
        case 0:

            // If signal is high, there is a short between the 24V supply and the output.
            // If the signal is low, there is a short between the ground and the output.
            if (!ok) {
                state = IMD_OFF;
            } else {
                state = IMD_MEASUREMENT_ERROR;
            }

            break;
        case 10:

            // Normal operation, duty cycle should be between 5% and 95%, measurement error otherwise
            if (duty_cycle >= 0.05 && duty_cycle <= 0.95) {
                state = IMD_NORMAL;
            }
            else {
                state = IMD_MEASUREMENT_ERROR;
            }

            break;
        case 20:

            // Undervoltage operation, voltage on power lines is lower than 30V. Measurements work in the same way is
            // for normal operation
            if (duty_cycle >= 0.05 && duty_cycle <= 0.95) {
                state = IMD_UNDERVOLTAGE;
            }
            else {
                state = IMD_MEASUREMENT_ERROR;
            }

            break;
        case 30:

            // Speed start measurement, only indicates good (DC of 5 to 10%) or bad (DC of 90 to 95%).
            // I assume that the boundary of being good or bad is at 200kOhm (IMD specific)
            if ((duty_cycle >= 0.05 && duty_cycle <= 0.10) || (duty_cycle >= 0.90 && duty_cycle <= 0.95)) {
                state = IMD_SPEED_MEASUREMENT;
            }
            else {
                state = IMD_MEASUREMENT_ERROR;
            }

            break;
        case 40:

            // Device error
            if (duty_cycle >= 0.475 && duty_cycle <= 0.525) {
                state = IMD_DEVICE_ERROR;
            }
            else {
                state = IMD_MEASUREMENT_ERROR;
            }

            break;
        case 50:

            // Fault detected on the earth connection
            if (duty_cycle >= 0.475 && duty_cycle <= 0.525) {
                state = IMD_EARTH_FAULT;
            }
            else {
                state = IMD_MEASUREMENT_ERROR;
            }
            break;

        default:
            state = IMD_MEASUREMENT_ERROR;
            break;
    }


    // If there is and measurement error, just wait on the next measurement (100ms at 10Hz)
    if (state == IMD_MEASUREMENT_ERROR) {
        imd_state = state;
        return;
    }

    // Calculate the resistance using the formula from the datasheet of the IMD, if the measured data both is correct
    // and has the right frequency
    if (state == IMD_NORMAL || state == IMD_UNDERVOLTAGE) {
        if (duty_cycle > 0.07) {
            resistance = (0.9 * 1200) / (duty_cycle - 0.05) - 1200;
        } else {
            resistance = 50000;
        }
    }

    // If there is a speed measurment, fault is determined differently, also when resistance is earth fault or below 80, set flag
    if ((state == IMD_SPEED_MEASUREMENT && duty_cycle >= 0.9) || state == IMD_EARTH_FAULT || resistance < 80) {
        fault_flag = 1;
    }

    imd_resistance = resistance;
    imd_state = state;
    imd_fault_flag = fault_flag;
    imd_period = period;
    imd_on_time = high_time;

    Param::SetInt(Param::ImdRes, imd_resistance);
    Param::SetInt(Param::ImdState, imd_state);
    Param::SetInt(Param::ImdFault, imd_fault_flag);
};

void BenderIMD::StartTimer() {
    // Enable GPIOC + TIM3
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_TIM3);

    // Set PC2 as input (TIM3_CH3)
    gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO2);
    gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO3);

    // TIM3 CH3: rising edge for period
    timer_ic_set_input(TIM3, TIM_IC3, TIM_IC_IN_TI3);
    timer_ic_set_polarity(TIM3, TIM_IC3, TIM_IC_RISING);
    timer_ic_set_prescaler(TIM3, TIM_IC3, TIM_IC_PSC_OFF);

    // TIM3 CH4: falling edge for high time
    timer_ic_set_input(TIM3, TIM_IC4, TIM_IC_IN_TI3);
    timer_ic_set_polarity(TIM3, TIM_IC4, TIM_IC_FALLING);
    timer_ic_set_prescaler(TIM3, TIM_IC4, TIM_IC_PSC_OFF);

    // Start TIM3
    timer_enable_counter(TIM3);
};

void BenderIMD::StopTimer() {
    timer_disable_counter(TIM3);
    rcc_periph_reset_pulse(RST_TIM3);

    // Optionally reset GPIOC2
    gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO2);
    gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO3);
};
