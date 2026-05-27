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

#ifndef EMUSBMS_H
#define EMUSBMS_H

#include "bms.h"
#include "canhardware.h"
#include <stdint.h>

class EmusBMS: public BMS
{
   public:
      void SetCanInterface(CanHardware* c) override;
      void DecodeCAN(int id, uint8_t * data) override;
      float MaxChargeCurrent() override;
      void Task100Ms() override;
      bool DischargeAllowed() override;
      float DischargeReductionLevel() override;
      int ProtectionSource() override { return 1; };

   private:
      bool BMSDataValid();
      bool ChargeAllowed();

      int timeoutCounter = 0;
      float minCellV = 0;
      float maxCellV = 0;
      float minTempC = 0;
      float maxTempC = 0;
      float avgTempC = 0;
      float stateOfCharge = 0;
      float remainingKWh = 0;
      float packVoltage = 0;
      float packCurrent = 0;

      // Full 32-bit protection flags and reduction byte from frame 0x307
      uint32_t protectionFlags = 0;
      uint8_t  reductionFlags  = 0;

      // Legacy byte accessors kept for ChargeAllowed() compatibility
      uint8_t diagByte0 = 0;
      uint8_t diagByte1 = 0;

      // J1939 charger mimic state (frames 0x1806E5F4 / 0x18FF50E5)
      float   j1939ReqVoltage = 0;
      float   j1939ReqCurrent = 0;
      bool    j1939StopBit    = false;
      bool    j1939Active     = false;
      int     j1939TxCounter  = 0;

      // Charge-stop debounce — updated only in Task100Ms() at a fixed 100 ms rate.
      // MaxChargeCurrent() reads chargeCurrentAllowed rather than calling ChargeAllowed()
      // directly, preventing the counter from incrementing more than once per 100 ms
      // tick (it would otherwise also increment from the stm32_vcu.cpp 200 ms check).
      int     chargeStopCounter     = 0;
      bool    chargeCurrentAllowed  = true;

      // EMUS configuration thresholds queried at startup via 0x380 (Base+80h).
      // param 0x0008: Low Cell Voltage Reduction Activate Value
      // param 0x0004: Cell Under-Voltage Protection Activate Value
      // Both encoded as uint8, 0.01 V/lsb, offset +200 (e.g. raw=80 → 2.80 V)
      float lowCellVReductionThreshold = 0.0f;
      float cellUVProtectionThreshold  = 0.0f;

      // Startup query state machine — queries 0x0008 then 0x0004 once at power-on.
      enum QueryState { QS_LOW_CELL_RED = 0, QS_UV_PROT, QS_DONE };
      QueryState queryState    = QS_LOW_CELL_RED;
      int        queryTimer    = 0; // counts 100 ms ticks between retries
};

#endif // EMUSBMS_H
