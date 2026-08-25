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
#ifndef DISPLAYCANTX_H
#define DISPLAYCANTX_H

#include "canhardware.h"

/*
 * Periodic transmission of the P911 display frames (0x64/0x65/0x67/0x68).
 *
 * Replaces the per-unit canmap configuration: the frame layout is fixed in
 * firmware so display and VCU stay in sync via git instead of flash config.
 * Enabled via the DisplayTx parameter (0=Off, 1=CAN1, 2=CAN2). When enabling,
 * remove any canmap entries for these IDs to avoid duplicate frames.
 */
class DisplayCanTx
{
public:
   static void Task100Ms(CanHardware* can);
};

#endif // DISPLAYCANTX_H
