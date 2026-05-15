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
 *
 * ChargeAllowed() uses the EMUS diagnostic codes frame (0x307) as the
 * single source of truth for protection limits, so thresholds only need
 * to be configured in the EMUS Control Panel and not duplicated on the VCU.
 *
 * J1939 charger mimic: the VCU sends 0x18FF50E5 every second to make the EMUS
 * think a J1939 charger is present. The EMUS responds with 0x1806E5F4 containing
 * its requested charge voltage and current. MaxChargeCurrent() returns that value
 * directly, so the Tesla Gen2 charger's power setpoint tracks the EMUS CC/CV
 * profile rather than being a fixed on/off signal.
 *
 * Diagnostic codes frame 0x307 byte layout (all bits active-high):
 *   Byte 0: bit0=CellUnderVolt, bit1=CellOverVolt, bit2=DchrgOvrCurr,
 *            bit3=ChrgOvrCurr,   bit4=CellModOverHeat, bit5=Leakage,
 *            bit6=NoCellComm,    bit7=MS_ConfError
 *   Byte 1: bit0=MSIntCANError,  bit1=MSCommCANErr, bit2=ChargerConnected,
 *            bit3=CellOverHeat,  bit4=NoCurrentSensor, bit5=PackUnderVolt,
 *            bit6=PackOverVolt,  bit7=CellUnderHeat
 */

void EmusBMS::SetCanInterface(CanHardware* c)
{
   can = c;
   can->RegisterUserMessage(0x301); // Battery Voltage Overall Parameters (Base+1)
   can->RegisterUserMessage(0x305); // State of Charge Parameters (Base+5)
   can->RegisterUserMessage(0x306); // Energy Parameters (Base+6)
   can->RegisterUserMessage(0x307);     // Diagnostic Codes (Base+7)
   can->RegisterUserMessage(0x308);     // Cell Temperature Overall Parameters (Base+8)
   can->RegisterUserMessage(0x1806E5F4); // J1939: BMS→Charger (EMUS sends in response to 0x18FF50E5)
}

bool EmusBMS::BMSDataValid() {
   // Return false if primary BMS is not sending data.
   if(timeoutCounter < 1) return false;
   return true;
}

// Return whether charging is currently permitted.
// Uses EMUS diagnostic flags as the source of truth — no need to duplicate
// voltage/temperature thresholds on the VCU side.
bool EmusBMS::ChargeAllowed()
{
   if(!BMSDataValid()) return false;

   // Stop charging on any cell/pack voltage or temperature protection event.
   // Byte 0 flags:
   if(diagByte0 & 0x02) return false; // CellOverVoltage
   if(diagByte0 & 0x08) return false; // ChrgOverCurrent
   if(diagByte0 & 0x10) return false; // CellModOverHeat
   if(diagByte0 & 0x20) return false; // Leakage (insulation fault)
   // Byte 1 flags:
   if(diagByte1 & 0x08) return false; // CellOverHeat
   if(diagByte1 & 0x40) return false; // PackOverVoltage
   if(diagByte1 & 0x80) return false; // CellUnderHeat (too cold to charge)

   return true;
}

// Return the maximum charge current allowed by the BMS.
// In J1939 mode (once 0x1806E5F4 has been received), returns the EMUS-requested
// current directly so the Tesla charger tracks the EMUS CC/CV taper profile.
// Falls back to 9998 (unlimited) if J1939 mode is not active.
float EmusBMS::MaxChargeCurrent()
{
   if(!ChargeAllowed()) return 0;
   if(!j1939Active)    return 9998.0f; // J1939 not active, pass through unlimited
   if(j1939StopBit)    return 0;       // EMUS requests charger stop
   return j1939ReqCurrent;
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
   else if (id == 0x305) // State of Charge Parameters (Base+5)
   {
      // Pack current: int16 MSB-first, 0.1 A/lsb (negative = discharging)
      packCurrent = (float)(int16_t)((data[0] << 8) | data[1]) / 10.0f;

      // User SOC: uint16 MSB-first, 1 %/lsb (EMUS sends 0-100, not 0-10000)
      // Note: bytes 5-6, not byte 6 alone. Requires user SOC range = 0-100 % in EMUS Control Panel.
      stateOfCharge = (float)(((uint16_t)data[5] << 8) | data[6]);

      // Reset timeout counter to the full timeout value
      timeoutCounter = Param::GetInt(Param::BMS_Timeout) * 10;
   }
   else if (id == 0x306) // Energy Parameters (Base+6)
   {
      // Remaining energy: uint16 MSB-first, 10 Wh/lsb → divide by 100 for kWh
      remainingKWh = (float)((data[2] << 8) | data[3]) / 100.0f;
   }
   else if (id == 0x1806E5F4) // J1939 BMS→Charger: EMUS requested charge limits
   {
      // Bytes 0-1: max allowable charge voltage, uint16 MSB-first, 0.1 V/lsb
      j1939ReqVoltage = (float)((uint16_t)(data[0] << 8) | data[1]) / 10.0f;
      // Bytes 2-3: max allowable charge current, uint16 MSB-first, 0.1 A/lsb
      j1939ReqCurrent = (float)((uint16_t)(data[2] << 8) | data[3]) / 10.0f;
      // Byte 4 bit 0: 0 = start charging, 1 = stop charging
      j1939StopBit    = (data[4] & 0x01) != 0;
      j1939Active     = true;
   }
   else if (id == 0x307) // Diagnostic Codes (Base+7)
   {
      // Store raw flag bytes for use in ChargeAllowed().
      // See file header comment for bit layout.
      diagByte0 = data[0];
      diagByte1 = data[1];
   }
   else if (id == 0x308) // Cell Temperature Overall Parameters (Base+8)
   {
      // Min/max/avg cell temperature: uint8, 1 °C/lsb, basis -100 °C
      minTempC = (float)(data[0]) - 100.0f;
      maxTempC = (float)(data[1]) - 100.0f;
      avgTempC = (float)(data[2]) - 100.0f;
   }
}

void EmusBMS::Task100Ms() {
   // Decrement timeout counter.
   if(timeoutCounter > 0) timeoutCounter--;

   if(BMSDataValid()) {
      Param::SetFloat(Param::BMS_Vmin, minCellV);
      Param::SetFloat(Param::BMS_Vmax, maxCellV);
      Param::SetFloat(Param::BMS_Tmin, minTempC);
      Param::SetFloat(Param::BMS_Tmax, maxTempC);
      Param::SetInt(Param::BMS_Tavg, (int)avgTempC);
      Param::SetFloat(Param::idc, packCurrent);
      Param::SetFloat(Param::udc2, packVoltage);
      // Auto-calibrate precharge threshold to actual pack voltage (matches Leaf BMS convention)
      if(packVoltage > 50.0f)
         Param::SetFloat(Param::udcsw, packVoltage - 20.0f);
      // Instantaneous power: positive = charging, negative = discharging
      Param::SetFloat(Param::power, (packVoltage * packCurrent) / 1000.0f);
   }
   else
   {
      Param::SetFloat(Param::BMS_Vmin, 0);
      Param::SetFloat(Param::BMS_Vmax, 0);
      Param::SetFloat(Param::BMS_Tmin, 0);
      Param::SetFloat(Param::BMS_Tmax, 0);
      Param::SetFloat(Param::idc, 0);
      Param::SetFloat(Param::udc2, 0);
   }

   Param::SetFloat(Param::KWh, remainingKWh);
   Param::SetFloat(Param::SOC, stateOfCharge);
   Param::SetInt(Param::BMS_ChargeLim, MaxChargeCurrent());

   // Poll BMS for all required frames
   uint8_t data[8] = {0};
   can->Send((uint32_t) 0x301, data, (uint8_t) 0);
   can->Send((uint32_t) 0x305, data, (uint8_t) 0);
   can->Send((uint32_t) 0x306, data, (uint8_t) 0);
   can->Send((uint32_t) 0x307, data, (uint8_t) 0);
   can->Send((uint32_t) 0x308, data, (uint8_t) 0);

   // J1939 charger mimic: send 0x18FF50E5 every 1 second.
   // The EMUS only broadcasts its requested charge limits (0x1806E5F4) in response
   // to receiving this message. The spec requires it within every 5 seconds.
   // We report actual pack voltage as charger output voltage; current is not
   // available from Tesla charger feedback so we report 0.
   j1939TxCounter++;
   if(j1939TxCounter >= 10)
   {
      j1939TxCounter = 0;
      uint8_t j1939[8] = {0};
      uint16_t reportVoltage = (uint16_t)(packVoltage * 10.0f); // 0.1 V/lsb
      j1939[0] = (reportVoltage >> 8) & 0xFF; // output voltage MSB
      j1939[1] =  reportVoltage       & 0xFF; // output voltage LSB
      j1939[2] = 0;                           // output current MSB (unknown)
      j1939[3] = 0;                           // output current LSB (unknown)
      j1939[4] = 0x00;                        // status: all normal
      j1939[5] = 0; j1939[6] = 0; j1939[7] = 0;
      can->Send(0x18FF50E5, (uint32_t*)j1939, 8);
   }
}
