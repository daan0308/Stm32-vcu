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
#include "my_math.h"

/*
 * EMUS G1 BMS driver.
 *
 * Periodic CAN frames decoded (default base address 0x300):
 *   0x301  Battery Voltage Overall Parameters — min/max/avg cell V, pack V
 *   0x305  State of Charge Parameters — pack current, user SOC
 *   0x306  Energy Parameters — remaining kWh
 *   0x307  Diagnostic Codes — 32-bit protection flags, reduction flags byte
 *   0x308  Cell Temperature Overall Parameters — min/max/avg cell temp
 *   0x1806E5F4  J1939 BMS→Charger — EMUS requested charge V and I
 *   0x380  Configuration Parameters response — startup threshold query replies
 *
 * Startup threshold query (0x380, Base+80h):
 *   At power-on the driver queries two EMUS configuration parameters once:
 *     0x0008  Low Cell Voltage Reduction Activate Value  → lowCellVReductionThreshold
 *     0x0004  Cell Under-Voltage Protection Activate Value → cellUVProtectionThreshold
 *   These are used to compute a proportional discharge reduction level from the
 *   live BMS_Vmin reading. If the query has not completed yet, a fixed 50 %
 *   fallback is used for the low-cell-voltage reduction flag.
 *
 * Diagnostic codes frame 0x307 byte layout (bits active-high):
 *   Bytes 0-3  PROTECTION FLAGS (LSB … MSB, 32 bits)
 *     bit  0: CellUnderVoltage      bit  1: CellOverVoltage
 *     bit  2: DischargeOverCurrent  bit  3: ChargeOverCurrent
 *     bit  4: CellModuleOverHeat    bit  5: Leakage
 *     bit  6: NoCellComm            bit  7: MS_ConfigError
 *     bit  8: MS_InternalCANError   bit  9: MS_CommonCANError
 *     bit 10: ChargerConnected      bit 11: CellOverHeat
 *     bit 12: NoCurrentSensor       bit 13: PackUnderVoltage
 *     bit 14: PackOverVoltage       bit 15: CellUnderHeat
 *     bits 16-31: extended fault flags (see CAN protocol spec)
 *   Byte 4  REDUCTION FLAGS
 *     bit 0: LowCellVoltage         bit 1: HighDischargeCurrentt
 *     bit 2: HighCellModuleTemp     bit 3: MS_ConfigMismatch
 *     bit 4: MS_CommonBusMalfunction  bit 5: HighCellTemp
 *   Byte 7  BATTERY STATUS FLAGS
 *
 * J1939 charger mimic: while opmode == MOD_CHARGE the VCU sends 0x18FF50E5
 * every second so the EMUS knows a J1939 charger is present. The frame carries
 * the actual charger output: Param::udc (DC bus voltage, 0.1 V/lsb) in bytes
 * 0-1 and Param::idc (charge current, 0.1 A/lsb) in bytes 2-3, both big-endian.
 * The EMUS responds with 0x1806E5F4 containing its requested charge voltage and
 * current. MaxChargeCurrent() returns that value directly, so the charger's
 * power setpoint tracks the EMUS CC/CV profile automatically.
 * The frame is suppressed outside charge mode to prevent the EMUS from setting
 * the ChargerConnected protection flag (0x307 bit 10) during drive.
 */

void EmusBMS::SetCanInterface(CanHardware* c)
{
   can = c;
   can->RegisterUserMessage(0x301);     // Battery Voltage Overall Parameters
   can->RegisterUserMessage(0x305);     // State of Charge Parameters
   can->RegisterUserMessage(0x306);     // Energy Parameters
   can->RegisterUserMessage(0x307);     // Diagnostic Codes
   can->RegisterUserMessage(0x308);     // Cell Temperature Overall Parameters
   can->RegisterUserMessage(0x1806E5F4); // J1939: BMS→Charger
   can->RegisterUserMessage(0x380);     // Configuration Parameters (startup query replies)
}

bool EmusBMS::BMSDataValid()
{
   return timeoutCounter > 0;
}

bool EmusBMS::ChargeAllowed()
{
   if (!BMSDataValid()) return false;

   // Stop charging on any cell/pack voltage or temperature protection event.
   if (protectionFlags & (1 <<  1)) return false; // CellOverVoltage
   if (protectionFlags & (1 <<  3)) return false; // ChargeOverCurrent
   if (protectionFlags & (1 <<  4)) return false; // CellModuleOverHeat
   if (protectionFlags & (1 << 11)) return false; // CellOverHeat
   if (protectionFlags & (1 << 14)) return false; // PackOverVoltage
   if (protectionFlags & (1 << 15)) return false; // CellUnderHeat (too cold to charge)

   return true;
}

bool EmusBMS::DischargeAllowed()
{
   if (!BMSDataValid()) return false;

   if (protectionFlags & (1 <<  0)) return false; // CellUnderVoltage
   if (protectionFlags & (1 <<  2)) return false; // DischargeOverCurrent
   if (protectionFlags & (1 << 13)) return false; // PackUnderVoltage

   return true;
}

/*
 * Returns a torque multiplier in [0.0, 1.0] driven by the EMUS reduction flags.
 *
 * Low Cell Voltage (bit 0): proportional between cellUVProtectionThreshold and
 *   lowCellVReductionThreshold using the live BMS_Vmin reading.  Falls back to
 *   50 % if the startup threshold query has not completed yet.
 *
 * High Discharge Current (bit 1): fixed 75 % — no proportional signal available.
 *
 * High Cell/Module Temperature (bits 2, 5): fixed 50 % — mirrors the behaviour
 *   of the existing TemperatureDerate() function.
 *
 * Multiple flags active simultaneously: the lowest (most restrictive) level wins.
 */
float EmusBMS::DischargeReductionLevel()
{
   if (!BMSDataValid()) return 1.0f;
   if (reductionFlags == 0) return 1.0f;

   float level = 1.0f;

   // Low Cell Voltage reduction — proportional between protection and reduction thresholds
   if (reductionFlags & 0x01)
   {
      if (lowCellVReductionThreshold > 0.0f && cellUVProtectionThreshold > 0.0f)
      {
         float band = lowCellVReductionThreshold - cellUVProtectionThreshold;
         if (band > 0.001f)
         {
            float err = minCellV - cellUVProtectionThreshold;
            level = MIN(level, MAX(0.0f, err / band));
         }
      }
      else
      {
         level = MIN(level, 0.5f); // fallback: thresholds not yet received
      }
   }

   // High Discharge Current — fixed 75 %
   if (reductionFlags & 0x02)
      level = MIN(level, 0.75f);

   // High Cell Module Temperature or High Cell Temperature — fixed 50 %
   if (reductionFlags & 0x24)
      level = MIN(level, 0.5f);

   return level;
}

/*
 * Returns the maximum charge current the EMUS permits, in amps.
 *
 * chargeCurrentAllowed is a debounced flag maintained by Task100Ms() at a fixed
 * 100 ms rate.  MaxChargeCurrent() reads it rather than calling ChargeAllowed()
 * directly; this prevents the debounce counter from being incremented more than
 * once per 100 ms tick (it is also called from the 200 ms task in stm32_vcu.cpp).
 *
 * A zero J1939 current without stop bit is treated as "setpoint not yet established"
 * and returns 9998 (unconstrained), because the EMUS uses the stop bit — not zero
 * current — as the authoritative charge-termination signal.
 */
float EmusBMS::MaxChargeCurrent()
{
   if (!chargeCurrentAllowed) return 0.0f;
   if (!j1939Active)          return 9998.0f; // J1939 not yet established — unconstrained
   if (j1939ReqCurrent < 0.1f) return 9998.0f; // CC/CV setpoint not yet established
   return j1939ReqCurrent;
}

void EmusBMS::DecodeCAN(int id, uint8_t *data)
{
   if (id == 0x301) // Battery Voltage Overall Parameters
   {
      // Min/avg/max cell voltage: uint8, 0.01 V/lsb, basis 2.00 V
      minCellV = (float)(data[0]) / 100.0f + 2.00f;
      maxCellV = (float)(data[1]) / 100.0f + 2.00f;
      // data[2] = average cell voltage (same encoding) — not stored separately

      // Total pack voltage: uint16 MSB-first at bytes 3-4, 0.01 V/lsb.
      // Verified against live capture: bytes[3,4] BE gives ~300 V which matches
      // actual pack voltage. The scrambled uint32 order from spec v3.1.0 does not
      // match actual EMUS firmware output and produced ~76000 V.
      uint16_t rawV = ((uint16_t)data[3] << 8) | data[4];
      packVoltage = rawV / 100.0f;
   }
   else if (id == 0x305) // State of Charge Parameters
   {
      // Pack current: int16 MSB-first, 0.1 A/lsb (negative = discharging)
      packCurrent = (float)(int16_t)((data[0] << 8) | data[1]) / 10.0f;

      // User SOC: uint16 MSB-first, 1 %/lsb
      stateOfCharge = (float)(((uint16_t)data[5] << 8) | data[6]);

      timeoutCounter = Param::GetInt(Param::BMS_Timeout) * 10;
   }
   else if (id == 0x306) // Energy Parameters
   {
      remainingKWh = (float)((data[2] << 8) | data[3]) / 100.0f;
   }
   else if (id == 0x307) // Diagnostic Codes
   {
      // Full 32-bit protection flags, LSB-first across bytes 0-3
      protectionFlags = ((uint32_t)data[3] << 24) | ((uint32_t)data[2] << 16) |
                        ((uint32_t)data[1] <<  8) |  (uint32_t)data[0];
      reductionFlags  = data[4];

      // Keep legacy byte aliases so ChargeAllowed() comparisons remain readable
      diagByte0 = data[0];
      diagByte1 = data[1];
   }
   else if (id == 0x308) // Cell Temperature Overall Parameters
   {
      // Min/max/avg cell temperature: uint8, 1 °C/lsb, basis -100 °C
      minTempC = (float)(data[0]) - 100.0f;
      maxTempC = (float)(data[1]) - 100.0f;
      avgTempC = (float)(data[2]) - 100.0f;
   }
   else if (id == 0x1806E5F4) // J1939 BMS→Charger
   {
      j1939ReqVoltage = (float)((uint16_t)(data[0] << 8) | data[1]) / 10.0f;
      j1939ReqCurrent = (float)((uint16_t)(data[2] << 8) | data[3]) / 10.0f;
      j1939StopBit    = (data[4] & 0x01) != 0;
      j1939Active     = true;
   }
   else if (id == 0x380) // Configuration Parameters response
   {
      // Response layout: [paramID_MSB, paramID_LSB, size, data_MSB, data_3rd, data_2nd, data_LSB, reserved]
      // All threshold params are uint8_t (size=1), value is in data[6] (LSB).
      // Encoding: value_V = (raw_byte + 200) * 0.01
      uint16_t paramId = ((uint16_t)data[0] << 8) | data[1];

      if (paramId == 0x0008 && queryState == QS_LOW_CELL_RED)
      {
         lowCellVReductionThreshold = (data[6] + 200) * 0.01f;
         queryState = QS_UV_PROT;
      }
      else if (paramId == 0x0004 && queryState == QS_UV_PROT)
      {
         cellUVProtectionThreshold = (data[6] + 200) * 0.01f;
         queryState = QS_DONE;
      }
   }
}

void EmusBMS::Task100Ms()
{
   if (timeoutCounter > 0) timeoutCounter--;

   if (BMSDataValid())
   {
      Param::SetFloat(Param::BMS_Vmin, minCellV);
      Param::SetFloat(Param::BMS_Vmax, maxCellV);
      Param::SetFloat(Param::BMS_Tmin, minTempC);
      Param::SetFloat(Param::BMS_Tmax, maxTempC);
      Param::SetInt(Param::BMS_Tavg, (int)avgTempC);
      // Auto-calibrate precharge threshold to actual pack voltage
      if (packVoltage > 50.0f)
         Param::SetFloat(Param::udcsw, packVoltage - 20.0f);
      Param::SetFloat(Param::power, (packVoltage * packCurrent) / 1000.0f);
   }
   else
   {
      Param::SetFloat(Param::BMS_Vmin, 0);
      Param::SetFloat(Param::BMS_Vmax, 0);
      Param::SetFloat(Param::BMS_Tmin, 0);
      Param::SetFloat(Param::BMS_Tmax, 0);
   }

   Param::SetFloat(Param::KWh, remainingKWh);
   Param::SetFloat(Param::SOC, stateOfCharge);

   // Debounce charge-stop conditions at the 100 ms task rate.
   // chargeCurrentAllowed is only updated here, so MaxChargeCurrent() can be called
   // from any task without incrementing the counter more than once per 100 ms.
   // Threshold: 50 ticks × 100 ms = 5 s — long enough to outlast CC→CV transients.
   static const int CHARGE_STOP_DEBOUNCE_TICKS = 50;
   if (!ChargeAllowed() || j1939StopBit)
   {
      if (++chargeStopCounter >= CHARGE_STOP_DEBOUNCE_TICKS)
         chargeCurrentAllowed = false; // sustained stop: genuine
   }
   else
   {
      chargeStopCounter    = 0;
      chargeCurrentAllowed = true;
   }

   Param::SetInt(Param::BMS_ChargeLim, MaxChargeCurrent());

   // EMUS-specific threshold visibility (queried at startup via 0x380)
   Param::SetFloat(Param::BMS_UVProtThr, cellUVProtectionThreshold);
   Param::SetFloat(Param::BMS_LowVRedThr, lowCellVReductionThreshold);

   // J1939 and protection-flag debug visibility
   Param::SetFloat(Param::BMS_J1939Cur,  j1939ReqCurrent);
   Param::SetInt(Param::BMS_J1939Stop,   j1939StopBit    ? 1 : 0);
   Param::SetInt(Param::BMS_ProtFlags,   (int)(protectionFlags & 0xFFFF));
   Param::SetInt(Param::BMS_ChgStopCnt,  chargeStopCounter);
   Param::SetInt(Param::BMS_J1939Act,    j1939Active     ? 1 : 0);

   // Poll BMS for all required frames
   uint8_t data[8] = {0};
   can->Send(0x301, (uint32_t*)data, 0);
   can->Send(0x305, (uint32_t*)data, 0);
   can->Send(0x306, (uint32_t*)data, 0);
   can->Send(0x307, (uint32_t*)data, 0);
   can->Send(0x308, (uint32_t*)data, 0);

   // Startup threshold query: send one request every 500 ms until both answered.
   // The EMUS responds on 0x380 and DecodeCAN() advances queryState.
   if (queryState != QS_DONE)
   {
      queryTimer++;
      if (queryTimer >= 5)
      {
         queryTimer = 0;
         uint8_t req[8] = {0};
         if (queryState == QS_LOW_CELL_RED)
         {
            req[0] = 0x00; req[1] = 0x08; req[2] = 0x01; // param 0x0008, size 1
         }
         else // QS_UV_PROT
         {
            req[0] = 0x00; req[1] = 0x04; req[2] = 0x01; // param 0x0004, size 1
         }
         can->Send(0x380, (uint32_t*)req, 3);
      }
   }

   // J1939 charger mimic: send 0x18FF50E5 every second, but only while in charge mode.
   // Sending this frame unconditionally makes the EMUS set the ChargerConnected protection
   // flag (0x307 bit 10) even when not charging. Gating on MOD_CHARGE prevents that.
   if (Param::GetInt(Param::opmode) == MOD_CHARGE)
   {
      j1939TxCounter++;
      if (j1939TxCounter >= 10)
      {
         j1939TxCounter = 0;
         uint8_t j1939[8] = {0};
         // Report actual charger output: udc = DC bus voltage, idc = charge current.
         // idc is positive when charging; clamp to 0 if the measurement reads negative.
         uint16_t reportVoltage = (uint16_t)(Param::GetFloat(Param::udc) * 10.0f);
         float    idcFloat      = Param::GetFloat(Param::idc);
         uint16_t reportCurrent = (idcFloat > 0.0f) ? (uint16_t)(idcFloat * 10.0f) : 0;
         j1939[0] = (reportVoltage >> 8) & 0xFF;
         j1939[1] =  reportVoltage       & 0xFF;
         j1939[2] = (reportCurrent >> 8) & 0xFF;
         j1939[3] =  reportCurrent       & 0xFF;
         j1939[4] = 0; j1939[5] = 0; j1939[6] = 0; j1939[7] = 0;
         can->Send(0x18FF50E5, (uint32_t*)j1939, 8);
      }
   }
   else
   {
      j1939TxCounter    = 0;     // reset so first TX fires promptly when charging starts
      j1939StopBit      = false; // clear any end-of-session stop bit so next session
                                 // is not blocked.  j1939Active and j1939ReqCurrent are
                                 // intentionally kept: resetting j1939Active here causes
                                 // MaxChargeCurrent() to return 0 when the EMUS sends its
                                 // first response before its CC/CV setpoint is established.
      chargeStopCounter    = 0;    // reset debounce so previous session state does not carry over
      chargeCurrentAllowed = true; // ensure next session starts in the allowed state
   }
}
