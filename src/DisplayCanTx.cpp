/*
 * This file is part of the ZombieVerter project.
 *
 * Copyright (C) 2026 Allard Posthumus
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
#include "DisplayCanTx.h"
#include "params.h"

/*
 * Frame layout as parsed by P911TouchDisplayP4 (main/CAN/CAN.c). All multi-byte
 * values little endian. Keep both sides in sync when changing anything here.
 *
 * 0x64 VCU status:   opmode u8 | status u16 | dir i8 | T15 u8 | derate u8 | - | -
 * 0x65 values101:    idc i16 (x10 A) | SOC u8 | speed u16 (kph) | tmphs u8 | torque u16
 * 0x67 values103:    range u16 (x10 km) | consumption u16 (Wh/km) |
 *                    BMS_Vmin u16 (x100 V) | BMS_Vmax u16 (x100 V)
 * 0x68 values104:    power i16 (kW) | BMS_Tmin i16 (C) | udc u16 (V) |
 *                    ImdState u8 | BMS_Tmax i8
 */

void DisplayCanTx::Task100Ms(CanHardware* can)
{
   uint8_t data[8];

   // 0x64 — VCU status
   uint16_t status = Param::GetInt(Param::status);
   data[0] = Param::GetInt(Param::opmode);
   data[1] = status & 0xFF;
   data[2] = status >> 8;
   data[3] = (int8_t)Param::GetInt(Param::dir);
   data[4] = Param::GetInt(Param::T15Stat);
   data[5] = Param::GetInt(Param::TorqDerate);
   data[6] = 0;
   data[7] = 0;
   can->Send(0x64, (uint32_t*)data, 8);

   // 0x65 — motor / drive values
   int16_t  idc    = (int16_t)(Param::GetFloat(Param::idc) * 10.0f);
   uint16_t speed  = Param::GetInt(Param::Veh_Speed);
   uint16_t torque = Param::GetInt(Param::torque);
   data[0] = idc & 0xFF;
   data[1] = idc >> 8;
   data[2] = Param::GetInt(Param::SOC);
   data[3] = speed & 0xFF;
   data[4] = speed >> 8;
   data[5] = Param::GetInt(Param::tmphs);
   data[6] = torque & 0xFF;
   data[7] = torque >> 8;
   can->Send(0x65, (uint32_t*)data, 8);

   // 0x67 — range / consumption / cell voltages
   uint16_t range = (uint16_t)(Param::GetFloat(Param::range) * 10.0f);
   uint16_t cons  = (uint16_t)Param::GetFloat(Param::consumption);
   uint16_t vmin  = (uint16_t)(Param::GetFloat(Param::BMS_Vmin) * 100.0f);
   uint16_t vmax  = (uint16_t)(Param::GetFloat(Param::BMS_Vmax) * 100.0f);
   data[0] = range & 0xFF;
   data[1] = range >> 8;
   data[2] = cons & 0xFF;
   data[3] = cons >> 8;
   data[4] = vmin & 0xFF;
   data[5] = vmin >> 8;
   data[6] = vmax & 0xFF;
   data[7] = vmax >> 8;
   can->Send(0x67, (uint32_t*)data, 8);

   // 0x68 — power / battery temperatures / pack voltage / IMD
   int16_t  power = (int16_t)Param::GetFloat(Param::power);
   int16_t  tmin  = (int16_t)Param::GetFloat(Param::BMS_Tmin);
   uint16_t udc   = (uint16_t)Param::GetFloat(Param::udc);
   data[0] = power & 0xFF;
   data[1] = power >> 8;
   data[2] = tmin & 0xFF;
   data[3] = tmin >> 8;
   data[4] = udc & 0xFF;
   data[5] = udc >> 8;
   data[6] = Param::GetInt(Param::ImdState);
   data[7] = (int8_t)Param::GetFloat(Param::BMS_Tmax);
   can->Send(0x68, (uint32_t*)data, 8);
}
