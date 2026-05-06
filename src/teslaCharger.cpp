/*
 * This file is part of the Zombieverter project.
 *
 * Copyright (C) 2023 Damien Maguire
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

#include "teslaCharger.h"
#include "my_math.h"
#include "params.h"

static bool HVreq=false;
static bool ChRun=false;
static uint8_t CurReq = 0;
static uint16_t HVvolts=0;
static uint16_t HVspnt=0;
static uint16_t HVpwr=0;


void teslaCharger::SetCanInterface(CanHardware* c)
{
   can = c;
   can->RegisterUserMessage(0x109);
}

void teslaCharger::DecodeCAN(int id, uint32_t data[8])
{
   uint8_t* bytes = (uint8_t*)data;
   if (id == 0x109)
   {
       if(bytes[5]==0x05) HVreq=true; // This should become high when the PP wire gets plugged in and the charger is enabled
       if(bytes[5]==0x00) HVreq=false;
   }
}

void teslaCharger::Task100Ms()
{
    HVvolts=Param::GetInt(Param::udc);
    HVspnt=Param::GetInt(Param::Voltspnt);
    HVpwr=Param::GetInt(Param::Pwrspnt);

    CurReq = HVpwr / HVvolts; // Calculate requested current without limit

    // Time to limit the charge current...
    CurReq = MIN(CurReq, Param::GetInt(Param::BMS_ChargeLim)); // Take the minimal current if the BMS gives a max charge limit
    CurReq = MIN(CurReq, 45); // Max allowed is 45A

    int opmode = Param::GetInt(Param::opmode);
    if (opmode != MOD_CHARGE) {
        CurReq = 0; // If the contactors have not closed and car is not ready to charge, no current is allowed to flow out
    }

    uint8_t bytes[8];
    bytes[0] = 0x00;
    bytes[1] = (HVspnt&0xFF); // HV voltage lowbyte
    bytes[2] = ((HVspnt&0xFF00)>>8); // HV voltage highbyte
    bytes[3] = CurReq; // HV Current Request
    if(ChRun)bytes[5] = 0x01;  // send Chg enable
    if(!ChRun)bytes[5] = 0x00; // send Chg disable
    bytes[6] = Param::GetInt(Param::SOC);
    bytes[7] = 0x00;
    can->Send(0x102, (uint32_t*)bytes,8);
}

bool teslaCharger::ControlCharge(bool RunCh, bool ACReq)
{
    ChRun = RunCh && ACReq; // Enable the charger if there is a AC request, since this charger is an OBC, it only supports AC charging

    if(HVreq) return true; // If the charger requests the car to get into charge_mod return true
    else return false;
}
