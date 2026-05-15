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
      uint8_t diagByte0 = 0;        // diagnostic flags byte 0 from frame 0x307
      uint8_t diagByte1 = 0;        // diagnostic flags byte 1 from frame 0x307

      // J1939 charger mimic state (frames 0x1806E5F4 / 0x18FF50E5)
      float   j1939ReqVoltage = 0;  // V,  EMUS requested charge voltage
      float   j1939ReqCurrent = 0;  // A,  EMUS requested charge current
      bool    j1939StopBit    = false; // true = EMUS requests stop charging
      bool    j1939Active     = false; // true once 0x1806E5F4 has been received
      int     j1939TxCounter  = 0;  // 100ms ticks until next 0x18FF50E5 transmit

};
#endif // EMUSBMS_H