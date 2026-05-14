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

#include "emusbms.h"

/*
 * This module receives messages from EMUS BMS and updates the
 * BMS_MinV, BMS_MaxV, BMS_MinT and BMS_MaxT parameters with the
 * received values. It also implements a timeout to indicate whether
 * the BMS is actively sending data or not. This data can be
 * used to safely stop any charging process if the BMS is not
 * working correctly.
 */

void EmusBMS::SetCanInterface(CanHardware* c)
{
   can = c;
   can->RegisterUserMessage(0x301); // Extended ID for min and max cell voltages
   can->RegisterUserMessage(0x308); // Extended ID for min and max cell temperatures
   can->RegisterUserMessage(0x306); // Extended ID for energy parameters (kWh)
   can->RegisterUserMessage(0x305); // Extended ID for SoC
}

bool EmusBMS::BMSDataValid() {
   // Return false if primary BMS is not sending data.
   if(timeoutCounter < 1) return false;
   return true;
}

// Return whether charging is currently permitted.
bool EmusBMS::ChargeAllowed()
{
   // Refuse to charge if the BMS is not sending data.
   if(!BMSDataValid()) return false;

   // Refuse to charge if the voltage or temperature is out of range.
   if(maxCellV > Param::GetFloat(Param::BMS_VmaxLimit)) return false;
   if(minCellV < Param::GetFloat(Param::BMS_VminLimit)) return false;
   if(maxTempC > Param::GetFloat(Param::BMS_TmaxLimit)) return false;
   if(minTempC < Param::GetFloat(Param::BMS_TminLimit)) return false;

   // Otherwise, charging is permitted.
   return true;
}

// Return the maximum charge current allowed by the BMS.
float EmusBMS::MaxChargeCurrent()
{
   if(!ChargeAllowed()) return 0;
   return 9998.0;
}

// Decode CAN messages from EMUS G1 BMS.
// Frame IDs assume default base address 0x300 (Base + Sub-ID).
// Byte layouts per EMUS G1 BMS CAN Protocol v3.1.0.
void EmusBMS::DecodeCAN(int id, uint8_t *data)
{
   if (id == 0x301) // Battery Voltage Overall Parameters (Base+1)
   {
      // Min/max cell voltage: uint8, 0.01 V/lsb, basis 2.00 V
      minCellV = (float)(data[0]) / 100.0f + 2.00f;
      maxCellV = (float)(data[1]) / 100.0f + 2.00f;

      // Total pack voltage: uint32, 0.01 V/lsb, non-sequential byte order per spec v3.1.0:
      // Data[5]=MSB (bits 31-24), Data[3]=2nd (bits 23-16), Data[6]=3rd (bits 15-8), Data[4]=LSB (bits 7-0)
      uint32_t rawV = ((uint32_t)data[5] << 24) | ((uint32_t)data[3] << 16) |
                      ((uint32_t)data[6] << 8)  |  (uint32_t)data[4];
      packVoltage = rawV / 100.0f;
   }
   else if (id == 0x308) // Cell Temperature Overall Parameters (Base+8)
   {
      // Min/max/avg cell temperature: uint8, 1 °C/lsb, basis -100 °C
      minTempC = (float)(data[0]) - 100.0f;
      maxTempC = (float)(data[1]) - 100.0f;
      avgTempC = (float)(data[2]) - 100.0f;
   }
   else if (id == 0x306) // Energy Parameters (Base+6)
   {
      // Remaining energy: uint16 MSB-first, 10 Wh/lsb → divide by 100 for kWh
      remainingKWh = (float)((data[2] << 8) | data[3]) / 100.0f;
   }
   else if (id == 0x305) // State of Charge Parameters (Base+5)
   {
      // Pack current: int16 MSB-first, 0.1 A/lsb (negative = discharging)
      packCurrent = (float)(int16_t)((data[0] << 8) | data[1]) / 10.0f;

      // User SOC: uint16 MSB-first, 0.01 %/lsb
      // Note: bytes 5-6, not byte 