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
#include "params.h"

/* iBooster protocol variants, selected with Param::iBooster:
 *
 * 1 = Bosch: native Bosch protocol. Layout verified 2026-09-13 against a CAN
 *     trace of this car's unit (ibooster.asc, 7 pedal presses):
 *       0x219, 5 bytes, ~20 ms cycle
 *       byte 0 = 0x24 pedal released, 0x25 driver applying brakes (bit 0)
 *       byte 1 = unit state (0x2A init, 0x59 ready)
 *       byte 2 = rolling counter, byte 3 = checksum
 *     The faster 0x214 frame carries the same flag (byte 2 bit 3) plus an
 *     11-bit rod position in bytes 1-2, should proportional detection ever
 *     be needed.
 *
 * 2 = Tesla: Tesla Model 3 style IBST_status frame:
 *       0x39D, byte 5 bits 0-1: 2 = driver applying, 1 = not applied,
 *       0 = fault/init.
 *     NOTE: commonly reported community layout, not verified against a real
 *     unit yet - trace a pedal press before relying on it.
 */
#define IBST_BOSCH_ID         0x219
#define IBST_TESLA_ID         0x39D

/* Frames arrive every 10-20 ms; declare the signal stale after 500 ms without
 * one. On timeout BrakeApplied() returns false so the hardwired brake switch
 * and regen-based logic remain the fallback. */
#define IBST_TIMEOUT_TICKS    5

uint8_t IBooster::timeoutTicks = 0;
bool IBooster::brakeApplied = false;

void IBooster::RegisterCanMessages(CanHardware* can)
{
    switch (Param::GetInt(Param::iBooster))
    {
    case IBOOSTER_BOSCH:
        can->RegisterUserMessage(IBST_BOSCH_ID);
        break;
    case IBOOSTER_TESLA:
        can->RegisterUserMessage(IBST_TESLA_ID);
        break;
    default:
        break;
    }
}

void IBooster::DecodeCAN(uint32_t id, uint32_t data[2])
{
    uint8_t* bytes = (uint8_t*)data;

    switch (Param::GetInt(Param::iBooster))
    {
    case IBOOSTER_BOSCH:
        if (id == IBST_BOSCH_ID)
        {
            brakeApplied = (bytes[0] & 0x01) != 0;
            timeoutTicks = IBST_TIMEOUT_TICKS;
        }
        break;
    case IBOOSTER_TESLA:
        if (id == IBST_TESLA_ID)
        {
            brakeApplied = (bytes[5] & 0x03) == 2;
            timeoutTicks = IBST_TIMEOUT_TICKS;
        }
        break;
    default:
        break;
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
