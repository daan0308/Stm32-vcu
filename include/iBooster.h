/*
 * This file is part of the ZombieVerter project.
 *
 * Copyright (C) 2026 Allard
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

#ifndef IBOOSTER_H
#define IBOOSTER_H

#include <stdint.h>
#include "canhardware.h"

/* Bosch iBooster brake-by-wire booster listener.
 *
 * Decodes the driver-brake-apply state from the iBooster status frame so the
 * brake light output and regen logic can react to pedal presses reported
 * over CAN. The protocol variant is selected with Param::iBooster:
 * 0 = Off, 1 = Bosch native (0x219), 2 = Tesla style (0x39D).
 */
class IBooster
{
public:
    enum ibooster_types
    {
        IBOOSTER_OFF = 0,
        IBOOSTER_BOSCH = 1,
        IBOOSTER_TESLA = 2
    };

    static void RegisterCanMessages(CanHardware* can);
    static void DecodeCAN(uint32_t id, uint32_t data[2]);
    static void Task100Ms();
    static bool BrakeApplied();

private:
    static uint8_t timeoutTicks;
    static bool brakeApplied;
};

#endif // IBOOSTER_H
