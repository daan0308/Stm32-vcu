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

#include "iBooster.h"

/* IBST_status frame from the Bosch iBooster Gen2 */
#define IBST_STATUS_ID        0x39D

/* !!! TODO: VERIFY AGAINST A CAN TRACE OF YOUR IBOOSTER BEFORE RELYING ON THIS !!!
 *
 * Layout below is the commonly reported Tesla Model 3 "IBST_status" layout:
 *   driverBrakeApply = byte 5, bits 0-1
 *   value 2 = DRIVER_APPLYING_BRAKES, 1 = NOT_APPLIED, 0 = FAULT/INIT
 *
 * To verify: log the bus with the pedal released and pressed, and check which
 * byte/bits of 0x39D change. Update the three constants below to match.
 */
#define DRIVER_BRAKE_BYTE     5
#define DRIVER_BRAKE_MASK     0x03
#define DRIVER_BRAKE_APPLIED  2

/* Frame arrives every 10 ms; declare the signal stale after 500 ms without one.
 * On timeout BrakeApplied() returns false so the hardwired brake switch and
 * regen-based logic remain the fallback. */
#define IBST_TIMEOUT_TICKS    5

uint8_t IBooster::timeoutTicks = 0;
bool IBooster::brakeApplied = false;

void IBooster::RegisterCanMessages(CanHardware* can)
{
    can->RegisterUserMessage(IBST_STATUS_ID);
}

void IBooster::DecodeCAN(uint32_t id, uint32_t data[2])
{
    if (id == IBST_STATUS_ID)
    {
        uint8_t* bytes = (uint8_t*)data;
        uint8_t apply = bytes[DRIVER_BRAKE_BYTE] & DRIVER_BRAKE_MASK;
        brakeApplied = (apply == DRIVER_BRAKE_APPLIED);
        timeoutTicks = IBST_TIMEOUT_TICKS;
    }
}

void IBooster::Task100Ms()
{
    if (timeoutTicks > 0)
    {
        timeoutTicks--;
    }
    else
    {
        brakeApplied = false; //signal stale, fail safe to "not braking"
    }
}

bool IBooster::BrakeApplied()
{
    return brakeApplied;
}
