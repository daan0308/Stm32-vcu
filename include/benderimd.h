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

#ifndef BENDERIMD_H
#define BENDERIMD_H
#include <stdint.h>
#include "params.h"
#include "imd.h"

class BenderIMD: public IMD
{
public:
    void DeInit() override;
    void Task100Ms() override;
    void StartTimer();
    void StopTimer();

private:
    enum IMD_states {
        IMD_OFF,
        IMD_NORMAL,
        IMD_UNDERVOLTAGE,
        IMD_SPEED_MEASUREMENT,
        IMD_DEVICE_ERROR,
        IMD_EARTH_FAULT,
        IMD_MEASUREMENT_ERROR
    };

    enum IMD_states imd_state;
    uint16_t imd_resistance;
    uint8_t imd_fault_flag;
    uint16_t imd_period;
    uint16_t imd_on_time;

};
#endif // BENDERIMD_H