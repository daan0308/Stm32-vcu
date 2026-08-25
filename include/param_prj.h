/*
 * This file is part of the ZombieVerter project.
 *
 * Copyright (C) 2011-2019 Johannes Huebner <dev@johanneshuebner.com>
 * Copyright (C) 2019-2022 Damien Maguire <info@evbmw.com>
 * Changes by Tom de Bree <tom@voltinflux.com> 2024
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

#define VER 2.30.A.DAALPO.3


/* Entries must be ordered as follows:
   1. Saveable parameters (id != 0)
   2. Temporary parameters (id = 0)
   3. Display values
 */
//Next param id (increase when adding new parameter!): 140
/*              category     name         unit       min     max     default id */
#define PARAM_LIST \
    PARAM_ENTRY(CAT_SETUP,     Inverter,     INVMODES, 0,      8,      0,      5  ) /* inverter type */ \
    PARAM_ENTRY(CAT_SETUP,     Vehicle,      VEHMODES, 0,      8,      0,      6  ) /* vehicle platform */ \
    PARAM_ENTRY(CAT_SETUP,     GearLvr,      SHIFTERS, 0,      4,      0,      108 ) /* gear lever/shifter type */ \
    PARAM_ENTRY(CAT_SETUP,     Transmission, TRNMODES, 0,      1,      0,      78 ) /* transmission type */ \
    PARAM_ENTRY(CAT_SETUP,     interface,    CHGINT,    0,     4,      0,      39 ) /* EVSE pilot/charge interface type */ \
    PARAM_ENTRY(CAT_SETUP,     chargemodes,  CHGMODS,   0,     6,      0,      37 ) /* onboard charger type */ \
    PARAM_ENTRY(CAT_SETUP,     BMS_Mode,    BMSMODES,  0,      6,      0,      90 ) /* selects active BMS driver */ \
    PARAM_ENTRY(CAT_SETUP,     ShuntType,   SHNTYPE,   0,      3,      0,      88 ) /* current shunt module type */ \
    PARAM_ENTRY(CAT_SETUP,     InverterCan,  CAN_DEV,  0,      1,      0,      70 ) /* CAN bus for inverter */ \
    PARAM_ENTRY(CAT_SETUP,     VehicleCan,   CAN_DEV,  0,      1,      1,      71 ) /* CAN bus for vehicle */ \
    PARAM_ENTRY(CAT_SETUP,     ShuntCan,     CAN_DEV,  0,      1,      0,      72 ) /* CAN bus for current shunt */ \
    PARAM_ENTRY(CAT_SETUP,     LimCan,       CAN_DEV,  0,      1,      0,      73 ) /* CAN bus for current limiter */ \
    PARAM_ENTRY(CAT_SETUP,     ChargerCan,   CAN_DEV,  0,      1,      1,      74 ) /* CAN bus for charger */ \
    PARAM_ENTRY(CAT_SETUP,     BMSCan,       CAN_DEV,  0,      1,      1,      89 ) /* CAN bus for BMS */ \
    PARAM_ENTRY(CAT_SETUP,     OBD2Can,      CAN_DEV,  0,      1,      0,      96 ) /* CAN bus for OBD2 */ \
    PARAM_ENTRY(CAT_SETUP,     DisplayTx,    DISPTX,   0,      2,      0,      143 ) /* P911 display frames 0x64-0x68 */ \
    PARAM_ENTRY(CAT_SETUP,     CanMapCan,    CAN_DEV,  0,      1,      0,      97 ) /* CAN bus for CAN mapper */ \
    PARAM_ENTRY(CAT_SETUP,     DCDCCan,      CAN_DEV,  0,      1,      1,      107 ) /* CAN bus for DC-DC converter */ \
    PARAM_ENTRY(CAT_SETUP,     HeaterCan,    CAN_DEV,  0,      1,      1,      138 ) /* CAN bus for heater */ \
    PARAM_ENTRY(CAT_SETUP,     MotActive,    MotorsAct,0,      3,      0,      129 ) /* active motor selection (MG1, MG2 or both) */ \
    PARAM_ENTRY(CAT_SETUP,     SpeedFactor, "RevInv/RevWheel", 0, 500, 84,     900 ) \
    PARAM_ENTRY(CAT_SETUP,     IMD_Type,     IMDTYPE,  0,      2,      0,      901)  \
    PARAM_ENTRY(CAT_THROTTLE,  potmin,      "dig",     0,      4095,   0,      7  ) /* throttle pot ADC minimum */ \
    PARAM_ENTRY(CAT_THROTTLE,  potmax,      "dig",     0,      4095,   4095,   8  ) /* throttle pot ADC maximum */ \
    PARAM_ENTRY(CAT_THROTTLE,  pot2min,     "dig",     0,      4095,   4095,   9  ) /* second throttle pot ADC minimum (dual-channel) */ \
    PARAM_ENTRY(CAT_THROTTLE,  pot2max,     "dig",     0,      4095,   4095,   10 ) /* second throttle pot ADC maximum (dual-channel) */ \
    PARAM_ENTRY(CAT_THROTTLE,  regenrpm,    "rpm",    100,      10000,    1500,     60 ) /* RPM above which regen activates */ \
    PARAM_ENTRY(CAT_THROTTLE,  regenendrpm,"rpm",     100,      10000,  100,  126 ) /* RPM below which regen disengages */ \
    PARAM_ENTRY(CAT_THROTTLE,  regenmax,     "%",     -35,   0,     -10,     61 ) /* maximum regen torque % */ \
    PARAM_ENTRY(CAT_THROTTLE,  regenBrake,    "%",    -35,   0,     -10,     122 ) /* regen torque when brake is pressed */ \
    PARAM_ENTRY(CAT_THROTTLE,  regenramp,   "%/10ms",  0.1,    100,    1,    68 ) /* regen ramp rate */ \
    PARAM_ENTRY(CAT_THROTTLE,  potmode,     POTMODES,  0,      1,      0,      11 ) /* single or dual-channel throttle pot */ \
    PARAM_ENTRY(CAT_THROTTLE,  dirmode,     DIRMODES,  0,      4,      1,      12 ) /* direction input method */ \
    PARAM_ENTRY(CAT_THROTTLE,  reversemotor,  ONOFF,   0,      1,      0,      127 ) /* invert motor rotation direction */ \
    PARAM_ENTRY(CAT_THROTTLE,  throtramp,   "%/10ms",  1,    100,    10,    13 ) /* forward throttle ramp rate */ \
    PARAM_ENTRY(CAT_THROTTLE,  throtramprpm,"rpm",     0,      20000,  20000,  14 ) /* RPM above which throtramp is bypassed */ \
    PARAM_ENTRY(CAT_THROTTLE,  revlim,      "rpm",     0,      20000,  6000,   15 ) /* motor RPM limit */ \
    PARAM_ENTRY(CAT_THROTTLE,  revRegen,     ONOFF,    0,       1,      0,      137 ) /* enable regen in reverse direction */ \
    PARAM_ENTRY(CAT_THROTTLE,  udcmin,      "V",       0,      1000,   450,    19 ) /* DC bus voltage below which torque is derated */ \
    PARAM_ENTRY(CAT_THROTTLE,  udclim,      "V",       0,      1000,   520,    20 ) /* DC bus voltage above which regen is limited */ \
    PARAM_ENTRY(CAT_THROTTLE,  idcmax,      "A",       0,      5000,   5000,   21 ) /* DC current above which torque is derated */ \
    PARAM_ENTRY(CAT_THROTTLE,  idcmin,      "A",      -5000,   0,     -5000,   22 ) /* DC current below which regen is limited */ \
    PARAM_ENTRY(CAT_THROTTLE,  tmphsmax,    "°C",      50,     150,    85,     23 ) /* heatsink temperature limit for derate */ \
    PARAM_ENTRY(CAT_THROTTLE,  tmpmmax,     "°C",      70,     300,    300,    24 ) /* motor temperature limit for derate */ \
    PARAM_ENTRY(CAT_THROTTLE,  throtmax,    "%",       0,      100,    100,    25 ) /* maximum forward throttle % */ \
    PARAM_ENTRY(CAT_THROTTLE,  throtmin,    "%",      -100,    0,     -100,    26 ) /* maximum regen throttle % */ \
    PARAM_ENTRY(CAT_THROTTLE,  throtmaxRev,    "%",       0,      100,    30,    123 ) /* maximum reverse throttle % */ \
    PARAM_ENTRY(CAT_THROTTLE,  throtdead,   "%",       0,      50,     10,     76 ) /* throttle deadband around zero */ \
    PARAM_ENTRY(CAT_THROTTLE,  RegenBrakeLight,   "%",    -100,     0,     -15,      128 ) /* throttle % below which brake light activates */ \
    PARAM_ENTRY(CAT_THROTTLE,  throtrpmfilt,   "rpm/10ms",  0.1,    200,    15,    131 ) /* RPM filter time constant for throttle */ \
    PARAM_ENTRY(CAT_LEXUS,     Gear,        LOWHIGH,   0,      3,      0,      27 ) /* Lexus/GS450H gearbox gear selection */ \
    PARAM_ENTRY(CAT_LEXUS,     OilPump,     "%",       0,      100,    50,     28 ) /* Lexus/GS450H oil pump duty cycle */ \
    PARAM_ENTRY(CAT_CRUISE,    cruisestep,  "rpm",     1,      1000,   200,    29 ) /* cruise control RPM increment per button press */ \
    PARAM_ENTRY(CAT_CRUISE,    cruiseramp,  "rpm/100ms",1,     1000,   20,     30 ) /* cruise control ramp rate */ \
    PARAM_ENTRY(CAT_CRUISE,    regenlevel,  "",        0,      3,      2,      31 ) /* regen level when cruise is active */ \
    PARAM_ENTRY(CAT_CONTACT,   udcsw,       "V",       0,      1000,   330,    32 ) /* DC voltage threshold to close main contactors */ \
    PARAM_ENTRY(CAT_CONTACT,   cruiselight, ONOFF,     0,      1,      0,      33 ) /* enable cruise control indicator output */ \
    PARAM_ENTRY(CAT_CONTACT,   errlights,   ERRLIGHTS, 0,      255,    0,      34 ) /* error indicator light output bitmask */ \
    PARAM_ENTRY(CAT_COMM,      CAN3Speed,   CAN3SPD,   0,      2,      0,      77 ) /* third CAN bus speed */ \
    PARAM_ENTRY(CAT_CHARGER,   BattCap,     "kWh",     0.1,    250,    22,     38 ) /* battery pack capacity for energy tracking */ \
    PARAM_ENTRY(CAT_CHARGER,   Voltspnt,    "V",       0,      1000,   395,    40 ) /* target charge voltage */ \
    PARAM_ENTRY(CAT_CHARGER,   Pwrspnt,     "W",       0,      12000,  1500,   41 ) /* charge power setpoint */ \
    PARAM_ENTRY(CAT_CHARGER,   IdcTerm,     "A",       0,      150,    0,      56 ) /* charge termination current (CC/CV transition) */ \
    PARAM_ENTRY(CAT_CHARGER,   CCS_ICmd,    "A",       0,      150,    0,      42 ) /* CCS charge current command */ \
    PARAM_ENTRY(CAT_CHARGER,   CCS_ILim,    "A",       0,      350,    100,    43 ) /* CCS maximum current limit */ \
    PARAM_ENTRY(CAT_CHARGER,   CCS_SOCLim,  "%",       0,      100,    80,     44 ) /* CCS charge SOC limit */ \
    PARAM_ENTRY(CAT_CHARGER,   SOCFC,       "%",       0,      100,    50,     79 ) /* SOC at which full charge is indicated */ \
    PARAM_ENTRY(CAT_CHARGER,   Chgctrl,     CHGCTRL,   0,      2,      0,      45 ) /* charge control mode */ \
    PARAM_ENTRY(CAT_CHARGER,   ChgAcVolt,   "Vac",     0,      250,   240,     120 ) /* AC supply voltage for efficiency calculation */ \
    PARAM_ENTRY(CAT_CHARGER,   ChgEff,     "%",       0,      100,   90,      121) /* charger efficiency % for power calculation */ \
    PARAM_ENTRY(CAT_CHARGER,   ConfigFoccci,  ONOFF,     0,      1,      0,     133) /* enable Foccci EVSE communication */ \
    PARAM_ENTRY(CAT_DCDC,      DCdc_Type,   DCDCTYPES, 0,      1,      0,      105 ) /* DC-DC converter type */ \
    PARAM_ENTRY(CAT_DCDC,      DCSetPnt,    "V",       9,      15,     14,     106 ) /* DC-DC output voltage setpoint */ \
    PARAM_ENTRY(CAT_BMS,       BMS_Timeout,  "sec",    1,      120,    10,     91 ) /* seconds without BMS frame before timeout */ \
    PARAM_ENTRY(CAT_BMS,       BMS_VminLimit, "V",     0,      10,     3.0,    92 ) /* minimum cell voltage limit (non-EMUS BMS) */ \
    PARAM_ENTRY(CAT_BMS,       BMS_VmaxLimit, "V",     0,      10,     4.2,    93 ) /* maximum cell voltage limit (non-EMUS BMS) */ \
    PARAM_ENTRY(CAT_BMS,       BMS_TminLimit, "°C",    -100,   100,    5,      94 ) /* minimum cell temperature limit (non-EMUS BMS) */ \
    PARAM_ENTRY(CAT_BMS,       BMS_TmaxLimit, "°C",    -100,   100,    50,     95 ) /* maximum cell temperature limit (non-EMUS BMS) */ \
    PARAM_ENTRY(CAT_HEATER,    Heater,      HTTYPE,    0,      3,      0,      57 ) /* heater module type */ \
    PARAM_ENTRY(CAT_HEATER,    Control,     HTCTRL,    0,      2,      0,      58 ) /* heater control mode */ \
    PARAM_ENTRY(CAT_HEATER,    HeatPwr,     "W",       0,      6500,   0,      59 ) /* heater power setpoint */ \
    PARAM_ENTRY(CAT_HEATER,    HeatPercnt,  "%",       0,      100,    0,      124 ) /* heater duty cycle % */ \
    PARAM_ENTRY(CAT_CLOCK,     Set_Day,     DOW,       0,      6,      0,      46 ) /* set current day of week */ \
    PARAM_ENTRY(CAT_CLOCK,     Set_Hour,    "Hours",   0,      23,     0,      47 ) /* set current hour */ \
    PARAM_ENTRY(CAT_CLOCK,     Set_Min,     "Mins",    0,      59,     0,      48 ) /* set current minute */ \
    PARAM_ENTRY(CAT_CLOCK,     Set_Sec,     "Secs",    0,      59,     0,      49 ) /* set current second */ \
    PARAM_ENTRY(CAT_CLOCK,     Chg_Hrs,     "Hours",   0,      23,     0,      50 ) /* scheduled charge start hour */ \
    PARAM_ENTRY(CAT_CLOCK,     Chg_Min,     "Mins",    0,      59,     0,      51 ) /* scheduled charge start minute */ \
    PARAM_ENTRY(CAT_CLOCK,     Chg_Dur,     "Mins",    0,      600,    0,      52 ) /* scheduled charge duration */ \
    PARAM_ENTRY(CAT_CLOCK,     Pre_Hrs,     "Hours",   0,      59,     0,      53 ) /* preheat start hour */ \
    PARAM_ENTRY(CAT_CLOCK,     Pre_Min,     "Mins",    0,      59,     0,      54 ) /* preheat start minute */ \
    PARAM_ENTRY(CAT_CLOCK,     Pre_Dur,     "Mins",    0,      60,     0,      55 ) /* preheat duration */ \
    PARAM_ENTRY(CAT_IOPINS,    PumpPWM,    PumpOutType,0,      1,      0,      135 ) /* coolant pump output type */ \
    PARAM_ENTRY(CAT_IOPINS,    Out1Func,    PINFUNCS,  0,      22,     6,      80 ) /* output pin 1 function */ \
    PARAM_ENTRY(CAT_IOPINS,    Out2Func,    PINFUNCS,  0,      22,     7,      81 ) /* output pin 2 function */ \
    PARAM_ENTRY(CAT_IOPINS,    Out3Func,    PINFUNCS,  0,      22,     3,      82 ) /* output pin 3 function */ \
    PARAM_ENTRY(CAT_IOPINS,    SL1Func,     PINFUNCS,  0,      22,     0,      83 ) /* signal light 1 function */ \
    PARAM_ENTRY(CAT_IOPINS,    SL2Func,     PINFUNCS,  0,      22,     0,      84 ) /* signal light 2 function */ \
    PARAM_ENTRY(CAT_IOPINS,    PWM1Func,    PINFUNCS,  0,      22,     0,      85 ) /* PWM output 1 function */ \
    PARAM_ENTRY(CAT_IOPINS,    PWM2Func,    PINFUNCS,  0,      22,     4,      86 ) /* PWM output 2 function */ \
    PARAM_ENTRY(CAT_IOPINS,    PWM3Func,    PINFUNCS,  0,      22,     2,      87 ) /* PWM output 3 function */ \
    PARAM_ENTRY(CAT_IOPINS,    GP12VInFunc, PINFUNCS,  0,      13,     12,     98 ) /* 12V general-purpose digital input function */ \
    PARAM_ENTRY(CAT_IOPINS,    HVReqFunc,   PINFUNCS,  0,      13,     12,     99 ) /* HV request input function */ \
    PARAM_ENTRY(CAT_IOPINS,    PB1InFunc,   PINFUNCS,  0,      13,     12,     140 ) /* pushbutton 1 input function */ \
    PARAM_ENTRY(CAT_IOPINS,    PB2InFunc,   PINFUNCS,  0,      13,     12,     141 ) /* pushbutton 2 input function */ \
    PARAM_ENTRY(CAT_IOPINS,    PB3InFunc,   PINFUNCS,  0,      13,     12,     142 ) /* pushbutton 3 input function */ \
    PARAM_ENTRY(CAT_IOPINS,    GPA1Func,    APINFUNCS, 0,      2,      0,      110 ) /* analog input 1 function */ \
    PARAM_ENTRY(CAT_IOPINS,    GPA2Func,    APINFUNCS, 0,      2,      0,      111 ) /* analog input 2 function */ \
    PARAM_ENTRY(CAT_IOPINS,    ppthresh,    "dig",     0,      4095,   2500,   114 ) /* proximity pilot ADC threshold */ \
    PARAM_ENTRY(CAT_IOPINS,    BrkVacThresh,"dig",     0,      4095,   2500,   115 ) /* brake vacuum sensor ADC threshold */ \
    PARAM_ENTRY(CAT_IOPINS,    BrkVacHyst,  "dig",     0,      4095,   2500,   116 ) /* brake vacuum sensor hysteresis */ \
    PARAM_ENTRY(CAT_IOPINS,    DigiPot1Step,"dig",     0,      255,    0,      117 ) /* digital potentiometer 1 step value */ \
    PARAM_ENTRY(CAT_IOPINS,    DigiPot2Step,"dig",     0,      255,    0,      118 ) /* digital potentiometer 2 step value */ \
    PARAM_ENTRY(CAT_IOPINS,    FanTemp,     "°C",      0,      100,    40,     134 ) /* temperature to activate cooling fan */ \
    PARAM_ENTRY(CAT_IOPINS,    TachoPPR,    "PPR",     0,      100,    2,      136 ) /* tacho pulses per revolution */ \
    PARAM_ENTRY(CAT_SHUNT,     IsaInit,     ONOFF,     0,      1,      0,      75 ) /* trigger ISA shunt module initialization */ \
    PARAM_ENTRY(CAT_PWM,       Tim3_Presc,  "",        1,      72000,  719,    100 ) /* Timer 3 prescaler */ \
    PARAM_ENTRY(CAT_PWM,       Tim3_Period, "",        1,      100000, 7200,   101 ) /* Timer 3 period */ \
    PARAM_ENTRY(CAT_PWM,       Tim3_1_OC,   "",        1,      100000, 3600,   102 ) /* Timer 3 channel 1 compare value */ \
    PARAM_ENTRY(CAT_PWM,       Tim3_2_OC,   "",        1,      100000, 3600,   103 ) /* Timer 3 channel 2 compare value */ \
    PARAM_ENTRY(CAT_PWM,       Tim3_3_OC,   "",        1,      100000, 3600,   104 ) /* Timer 3 channel 3 compare value */ \
    PARAM_ENTRY(CAT_PWM,       CP_PWM,   "",        1,      100, 10,   132 ) /* charge pilot PWM duty cycle */ \
    VALUE_ENTRY(version,       VERSTR,              2000 ) /* firmware version string */ \
    VALUE_ENTRY(opmode,        OPMODES,             2002 ) /* current operating mode */ \
    VALUE_ENTRY(chgtyp,        CHGTYPS,             2003 ) /* active charge type */ \
    VALUE_ENTRY(lasterr,       errorListString,     2004 ) /* last error message */ \
    VALUE_ENTRY(status,        STATUS,              2005 ) /* system status bitmask */ \
	VALUE_ENTRY(CanAct,        ONOFF,               2107 ) /* CAN bus active */ \
    VALUE_ENTRY(TorqDerate,    LIMITREASON,         2102 ) /* torque derating reason bitmask */ \
    VALUE_ENTRY(udc,           "V",                 2006 ) /* main DC bus voltage */ \
    VALUE_ENTRY(udc2,          "V",                 2007 ) /* BMS pack voltage */ \
    VALUE_ENTRY(udc3,          "V",                 2008 ) /* auxiliary DC bus voltage */ \
    VALUE_ENTRY(deltaV,        "V",                 2009 ) /* voltage difference between udc sources */ \
    VALUE_ENTRY(INVudc,        "V",                 2010 ) /* inverter-reported DC bus voltage */ \
    VALUE_ENTRY(power,         "kW",                2011 ) /* drive power (positive=discharge, negative=charge) */ \
    VALUE_ENTRY(idc,           "A",                 2012 ) /* DC bus current */ \
    VALUE_ENTRY(KWh,           "kwh",               2013 ) /* remaining energy */ \
    VALUE_ENTRY(range,         "km",                9002 ) /* estimated remaining range */ \
    VALUE_ENTRY(consumption,   "Wh/km",             9003 ) /* energy consumption */ \
    VALUE_ENTRY(AMPh,          "Ah",                2014 ) /* amp-hours consumed */ \
    VALUE_ENTRY(SOC,           "%",                 2015 ) /* state of charge */ \
    VALUE_ENTRY(BMS_Vmin,      "V",                 2084 ) /* minimum cell voltage */ \
    VALUE_ENTRY(BMS_Vmax,      "V",                 2085 ) /* maximum cell voltage */ \
    VALUE_ENTRY(BMS_Tavg,      "°C",                2103 ) /* average cell temperature */ \
    VALUE_ENTRY(BMS_Tmin,      "°C",                2086 ) /* minimum cell temperature */ \
    VALUE_ENTRY(BMS_Tmax,      "°C",                2087 ) /* maximum cell temperature */ \
    VALUE_ENTRY(BMS_ChargeLim, "A",                 2088 ) /* maximum charge current from BMS */ \
    VALUE_ENTRY(BMS_MaxInput,  "kW",                2105 ) /* maximum input power from BMS */ \
    VALUE_ENTRY(BMS_MaxOutput, "kW",                2106 ) /* maximum output power from BMS */ \
    VALUE_ENTRY(BMS_MaxCharge, "W",                 2101 ) /* maximum charge power */ \
    VALUE_ENTRY(BMS_Isolation, "Ohm",               2104 ) /* isolation resistance */ \
    VALUE_ENTRY(BMS_IsoMeas,   "mV",                2099 ) /* isolation measurement voltage */ \
    VALUE_ENTRY(BMS_DischargeOk,   ONOFF,           9005 ) /* 1=discharge allowed, 0=protection flag active BMS */ \
    VALUE_ENTRY(BMS_TorqRedPct,    "%",             9006 ) /* drive torque reduction factor: 100=full torque, 0=fully reduced */ \
    VALUE_ENTRY(BMS_UVProtThr,     "V",             9007 ) /* cell under-voltage protection threshold queried from BMS at startup */ \
    VALUE_ENTRY(BMS_LowVRedThr,    "V",             9008 ) /* low cell voltage reduction threshold queried from BMS at startup */ \
    VALUE_ENTRY(BMS_ProtSrc,       BMSPROTSRC,      9009 ) /* active (dis)charge protection source */ \
    VALUE_ENTRY(BMS_J1939Cur,      "A",             9010 ) /* raw J1939 charge current requested by EMUS (before debounce) */ \
    VALUE_ENTRY(BMS_J1939Stop,     ONOFF,           9011 ) /* J1939 stop bit from EMUS: 1=EMUS commanding charge stop */ \
    VALUE_ENTRY(BMS_ProtFlags,     "",              9012 ) /* EMUS protection flags lower 16 bits from frame 0x307 */ \
    VALUE_ENTRY(BMS_ChgStopCnt,    "",              9013 ) /* charge stop debounce counter (0-50 ticks at 100ms, 50=stop confirmed) */ \
    VALUE_ENTRY(BMS_J1939Act,      ONOFF,           9014 ) /* J1939 active: 1=EMUS has responded to charger mimic */ \
    VALUE_ENTRY(speed,         "rpm",               2016 ) /* motor RPM */ \
    VALUE_ENTRY(Veh_Speed,     "kph",               2017 ) /* vehicle speed */ \
    VALUE_ENTRY(torque,        "dig",               2018 ) /* torque setpoint */ \
    VALUE_ENTRY(pot,           "dig",               2019 ) /* throttle pot 1 ADC value */ \
    VALUE_ENTRY(pot2,          "dig",               2020 ) /* throttle pot 2 ADC value */ \
    VALUE_ENTRY(potbrake,      "dig",               2021 ) /* brake pot ADC value */ \
    VALUE_ENTRY(brakepressure, "dig",               2022 ) /* brake pressure ADC value */ \
    VALUE_ENTRY(potnom,        "%",                 2023 ) /* normalized throttle % after all limiting */ \
    VALUE_ENTRY(dir,           DIRS,                2024 ) /* drive direction */ \
    VALUE_ENTRY(tmphs,         "°C",                2028 ) /* heatsink temperature */ \
    VALUE_ENTRY(tmpm,          "°C",                2029 ) /* motor temperature */ \
    VALUE_ENTRY(tmpaux,        "°C",                2030 ) /* auxiliary temperature sensor */ \
    VALUE_ENTRY(uaux,          "V",                 2031 ) /* auxiliary voltage */ \
    VALUE_ENTRY(canio,         CANIOS,              2032 ) /* CAN digital I/O state bitmask */ \
    VALUE_ENTRY(FrontRearBal,  "%",                 2082 ) /* front/rear torque balance */ \
    VALUE_ENTRY(cruisespeed,   "rpm",               2033 ) /* cruise control target speed */ \
    VALUE_ENTRY(cruisestt,     CRUISESTATES,        2034 ) /* cruise control state */ \
    VALUE_ENTRY(din_cruise,    ONOFF,               2035 ) /* cruise control digital input */ \
    VALUE_ENTRY(din_start,     ONOFF,               2036 ) /* start digital input */ \
    VALUE_ENTRY(din_brake,     ONOFF,               2037 ) /* brake digital input */ \
    VALUE_ENTRY(din_forward,   ONOFF,               2038 ) /* forward direction digital input */ \
    VALUE_ENTRY(din_reverse,   ONOFF,               2039 ) /* reverse direction digital input */ \
    VALUE_ENTRY(din_bms,       ONOFF,               2040 ) /* BMS digital input */ \
    VALUE_ENTRY(din_12Vgp,     ONOFF,               2071 ) /* 12V general-purpose digital input */ \
    VALUE_ENTRY(handbrk,       ONOFF,               2041 ) /* handbrake input */ \
    VALUE_ENTRY(Gear1,         ONOFF,               2042 ) /* gear selector position 1 */ \
    VALUE_ENTRY(Gear2,         ONOFF,               2043 ) /* gear selector position 2 */ \
    VALUE_ENTRY(Gear3,         ONOFF,               2044 ) /* gear selector position 3 */ \
    VALUE_ENTRY(T15Stat,       ONOFF,               2045 ) /* terminal 15 (ignition) status */ \
    VALUE_ENTRY(InvStat,       ONOFF,               2046 ) /* inverter ready status */ \
    VALUE_ENTRY(GearFB,        LOWHIGH,             2047 ) /* gearbox feedback position */ \
    VALUE_ENTRY(CableLim,      "A",                 2048 ) /* charge cable current limit (from pilot) */ \
    VALUE_ENTRY(PilotLim,      "A",                 2049 ) /* charge pilot current limit */ \
    VALUE_ENTRY(PlugDet,       ONOFF,               2050 ) /* charge plug detection */ \
    VALUE_ENTRY(PilotTyp,      PLTMODES,            2051 ) /* charge pilot type detected */ \
    VALUE_ENTRY(CCS_I_Avail,   "A",                 2052 ) /* CCS available current */ \
    VALUE_ENTRY(CCS_V_Avail,   "V",                 2053 ) /* CCS available voltage */ \
    VALUE_ENTRY(CCS_I,         "A",                 2054 ) /* CCS actual charge current */ \
    VALUE_ENTRY(CCS_Ireq,      "A",                 2068 ) /* CCS requested current */ \
    VALUE_ENTRY(CCS_V,         "V",                 2055 ) /* CCS actual charge voltage */ \
    VALUE_ENTRY(CCS_V_Min,     "V",                 2056 ) /* CCS minimum voltage */ \
    VALUE_ENTRY(CCS_V_Con,     "V",                 2057 ) /* CCS contract voltage */ \
    VALUE_ENTRY(hvChg,         ONOFF,               2058 ) /* HV charging active */ \
    VALUE_ENTRY(CCS_COND,      CCS_STATUS,          2059 ) /* CCS condition/status */ \
    VALUE_ENTRY(CCS_State,     "s",                 2060 ) /* CCS state machine timer */ \
    VALUE_ENTRY(CP_DOOR,       DMODES,              2061 ) /* charge port door state */ \
    VALUE_ENTRY(CCS_Contactor, ONOFF,               2062 ) /* CCS contactor state */ \
    VALUE_ENTRY(Day,           DOW,                 2064 ) /* current day of week */ \
    VALUE_ENTRY(Hour,          "H",                 2065 ) /* current hour */ \
    VALUE_ENTRY(Min,           "M",                 2066 ) /* current minute */ \
    VALUE_ENTRY(Sec,           "S",                 2067 ) /* current second */ \
    VALUE_ENTRY(ChgT,          "M",                 2090 ) /* remaining charge time */ \
    VALUE_ENTRY(HeatReq,       ONOFF,               2069 ) /* heater request active */ \
    VALUE_ENTRY(U12V,          "V",                 2070 ) /* 12V supply voltage */ \
    VALUE_ENTRY(I12V,          "A",                 2083 ) /* 12V supply current */ \
    VALUE_ENTRY(ChgTemp,       "°C",                2078 ) /* charger temperature */ \
    VALUE_ENTRY(AC_Volts,      "V",                 2079 ) /* AC supply voltage */ \
    VALUE_ENTRY(AC_Amps,       "A",                 2089 ) /* AC supply current */ \
    VALUE_ENTRY(canctr,        "dig",               2091 ) /* CAN error counter */ \
    VALUE_ENTRY(cpuload,       "%",                 2063 ) /* CPU load */ \
    VALUE_ENTRY(PPVal,         "dig",               2094 ) /* proximity pilot ADC value */ \
    VALUE_ENTRY(BrkVacVal,     "dig",               2095 ) /* brake vacuum sensor ADC value */ \
    VALUE_ENTRY(tmpheater,     "°C",                2096 ) /* heater temperature */ \
    VALUE_ENTRY(udcheater,     "V",                 2097 ) /* heater supply voltage */ \
    VALUE_ENTRY(powerheater,   "W",                 2098 ) /* heater power */ \
    VALUE_ENTRY(VehLockSt,     ONOFF,               2100 ) /* vehicle lock state */\
    VALUE_ENTRY(ImdFault,      ONOFF,               9000 ) \
    VALUE_ENTRY(ImdState,      IMDSTATE,            9001 ) \
//Next value Id: 2108  (next EMUS-specific value Id: 9015)

//Dead params
/*
    PARAM_ENTRY(CAT_THROTTLE,  bmslimhigh,  "%",       0,      100,    50,     17 ) \
    PARAM_ENTRY(CAT_THROTTLE,  bmslimlow,   "%",      -100,    0,     -1,      18 ) \
*/
//////////

#define VERSTR STRINGIFY(4=VER)
#define PINFUNCS     "0=None, 1=ChaDeMoAlw, 2=OBCEnable, 3=HeaterEnable, 4=RunIndication, 5=WarnIndication," \
                     "6=CoolantPump, 7=NegContactor, 8=BrakeLight, 9=ReverseLight, 10=HeatReq, 11=HVRequest," \
                     "12=DCFCRequest, 13=BrakeVacPump, 14=CoolingFan, 15=HvActive, 16=PwmTim3, 17=CpSpoof,"\
                     "18=GS450pump, 19=IMD_Ok, 20=CoolantPumpBattery, 21=AlwaysOn, 22=BattTempGt15"
#define APINFUNCS    "0=None, 1=ProxPilot, 2=BrakeVacSensor, 3=HeaterPot"
#define IMDTYPE      "0=None, 1=SimpleBender"
#define SHIFTERS     "0=None, 1=BMW_F30, 2=JLR_G1, 3=JLR_G2, 4=BMW_E65"
#define SHNTYPE      "0=None, 1=ISA, 2=SBOX, 3=VAG"
#define DMODES       "0=CLOSED, 1=OPEN, 2=ERROR, 3=INVALID"
#define POTMODES     "0=SingleChannel, 1=DualChannel"
#define BTNSWITCH    "0=Button, 1=Switch, 2=CAN"
#define DIRMODES     "0=Button, 1=Switch, 2=ButtonReversed, 3=SwitchReversed, 4=DefaultForward"
#define INVMODES     "0=None, 1=Leaf_Gen1, 2=GS450H, 3=UserCAN, 4=OpenI, 5=Prius_Gen3, 6=Outlander, 7=GS300H, 8=RearOutlander"
#define PLTMODES     "0=Absent, 1=ACStd, 2=ACchg, 3=Error, 4=CCS_Not_Rdy, 5=CCS_Rdy, 6=Static"
#define VEHMODES     "0=BMW_E46, 1=BMW_E6x+, 2=Classic, 3=None, 5=BMW_E39, 6=VAG, 7=Subaru, 8=BMW_E31"
#define BMSMODES     "0=Off, 1=SimpBMS, 2=TiDaisychainSingle, 3=TiDaisychainDual, 4=LeafBms, 5=RenaultKangoo33, 6=EMUS"
#define BMSPROTSRC   "0=VCU, 1=BMS"
#define OPMODES      "0=Off, 1=Run, 2=Precharge, 3=PchFail, 4=Charge"
#define DOW          "0=Sun, 1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat"
#define CHGTYPS      "0=Off, 1=AC, 2=DCFC"
#define DCDCTYPES    "0=NoDCDC, 1=TeslaG2"
#define STATUS       "0=None, 1=UdcLow, 2=UdcHigh, 4=UdcBelowUdcSw, 8=UdcLim, 16=EmcyStop, 32=MProt, 64=PotPressed, 128=TmpHs, 256=WaitStart"
#define CCS_STATUS   "0=NotRdy, 1=ready, 2=SWoff, 3=interruption, 4=Prech, 5=insulmon, 6=estop, 7=malfunction, 15=invalid"
#define DIRS         "-1=Reverse, 0=Neutral, 1=Drive, 2=Park"
#define ONOFF        "0=Off, 1=On, 2=na"
#define LOWHIGH      "0=LOW, 1=HIGH, 2=AUTO, 3=HIGHFWDLOWREV"
#define OKERR        "0=Error, 1=Ok, 2=na"
#define CANSPEEDS    "0=125k, 1=250k, 2=500k, 3=800k, 4=1M"
#define CANIOS       "1=Cruise, 2=Start, 4=Brake, 8=Fwd, 16=Rev, 32=Bms"
#define CANPERIODS   "0=100ms, 1=10ms"
#define ERRLIGHTS    "0=Off, 4=EPC, 8=engine"
#define CRUISESTATES "0=None, 1=On, 2=Disable, 4=Set, 8=Resume"
#define CDMSTAT      "1=Charging, 2=Malfunction, 4=ConnLock, 8=BatIncomp, 16=SystemMalfunction, 32=Stop"
#define HTTYPE       "0=None, 1=Ampera, 2=VW, 3=OutlanderCan"
#define HTCTRL       "0=Disable, 1=Enable, 2=Timer"
#define CHGMODS      "0=Off, 1=EXT_DIGI, 2=Volt_Ampera, 3=Leaf_PDM, 4=TeslaOI, 5=Out_lander, 6=Elcon"
#define CHGCTRL      "0=Enable, 1=Disable, 2=Timer"
#define CHGINT       "0=Unused, 1=i3LIM, 2=Chademo, 3=CPC, 4=Foccci"
#define CAN3SPD      "0=k33.3, 1=k500, 2=k100"
#define TRNMODES     "0=Manual, 1=Auto"
#define CAN_DEV      "0=CAN1, 1=CAN2"
#define DISPTX       "0=Off, 1=CAN1, 2=CAN2"
#define CAT_THROTTLE "Throttle"
#define CAT_POWER    "Power Limit"
#define CAT_CONTACT  "Contactor Control"
#define CAT_TEST     "Testing"
#define CAT_COMM     "Communication"
#define CAT_SETUP    "General Setup"
#define CAT_CLOCK    "RTC Module"
#define CAT_HEATER   "Heater Module"
#define CAT_BMS      "Battery Management"
#define CAT_CRUISE   "Cruise Control"
#define CAT_LEXUS    "Gearbox Control"
#define CAT_CHARGER  "Charger Control"
#define CAT_DCDC     "DC-DC Converter"
#define CAT_SHUNT    "ISA Shunt Control"
#define CAT_IOPINS   "General Purpose I/O"
#define CAT_PWM      "PWM Control"
#define MotorsAct    "0=Mg1and2, 1=Mg1, 2=Mg2, 3=BlendingMG2and1"
#define PumpOutType  "0=GS450hOil, 1=TachoOut"
#define LIMITREASON  "0=None, 1=UDClimLow, 2=UDClimHigh, 4=IDClimLow, 8=IDClimHigh, 16=TempLim"
#define IMDSTATE     "0=IMD_OFF, 1=IMD_NORMAL, 2=IMD_MEASUREMENT_ERROR"

#define CAN_PERIOD_100MS    0
#define CAN_PERIOD_10MS     1

#define FIRST_IO_PARAM Param::Out1Func
#define SEC_IO_PARAM Param::PB1InFunc
#define FIRST_AI_PARAM Param::GPA1Func
enum modes
{
    MOD_OFF = 0,
    MOD_RUN,
    MOD_PRECHARGE,
    MOD_PCHFAIL,
    MOD_CHARGE,
    MOD_LAST
};

enum ctyps
{
    OFF = 0,
    AC,
    DCFC
};

enum _tripmodes
{
    TRIP_ALLOFF = 0,
    TRIP_DCSWON,
    TRIP_PRECHARGEON
};

enum _dirmodes
{
    DIR_BUTTON = 0,
    DIR_SWITCH = 1,
    DIR_REVERSED = 2,
    DIR_DEFAULTFORWARD = 4
};

enum InvModes
{
    NoInv =0,
    Leaf_Gen1 = 1,
    GS450H = 2,
    UserCAN = 3,
    OpenI = 4,
    Prius_Gen3 = 5,
    Outlander = 6,
    GS300H = 7,
    RearOutlander = 8
};

enum ChargeModes
{
    Off = 0,
    EXT_DIGI = 1,
    Volt_Ampera = 2,
    Leaf_PDM = 3,
    TeslaOI = 4,
    Out_lander = 5,
    Elcon = 6
};

enum ChargeInterfaces
{
    Unused = 0,
    i3LIM = 1,
    Chademo = 2,
    CPC = 3,
    Foccci = 4
};

enum HeatType
{
    Noheater = 0,
    AmpHeater = 1,
    VW = 2,
        OutlanderHeater = 3
};

enum BMSModes
{
    BMSModeNoBMS = 0,
    BMSModeSimpBMS = 1,
    BMSModeDaisychainSingleBMS = 2,
    BMSModeDaisychainDualBMS = 3,
    BMSModeLeafBMS = 4,
    BMSRenaultKangoo33BMS = 5,
    BMSModeEmusBMS = 6
};

enum DCDCModes
{
    NoDCDC = 0,
    TeslaG2 = 1

};

enum ShifterModes
{
    NoShifter = 0,
    BMWF30 = 1,
    JLRG1 = 2,
    JLRG2 =3,
    BMWE65 =4

};

enum IMDTypes
{
    NOIMD = 0,
    SIMPLE_BENDER_IMD = 1
};

enum ChargeControl
{
    Enable = 0,
    Disable = 1,
    Timer = 2
};

enum Gear
{
    LOW = 0,
    HIGH = 1,
    AUTO = 2
};

enum vehicles
{
    vBMW_E46 = 0,
    vBMW_E65 = 1,
    Classic = 2, //used as a flag
    None = 4,
    vBMW_E39 = 5,
    vVAG = 6,
    vSUBARU = 7,
    vBMW_E31 = 8
};

enum _potmodes
{
    POTMODE_SINGLECHANNEL = 0,
    POTMODE_DUALCHANNEL,
};

enum _canio
{
    CAN_IO_CRUISE = 1,
    CAN_IO_START = 2,
    CAN_IO_BRAKE = 4,
    CAN_IO_FWD = 8,
    CAN_IO_REV = 16,
    CAN_IO_BMS = 32
};

enum status
{
    STAT_NONE = 0,
    STAT_UDCLOW = 1,
    STAT_UDCHIGH = 2,
    STAT_UDCBELOWUDCSW = 4,
    STAT_UDCLIM = 8,
    STAT_EMCYSTOP = 16,
    STAT_MPROT = 32,
    STAT_POTPRESSED = 64,
    STAT_TMPHS = 128,
    STAT_WAITSTART = 256
};

enum ccs_status
{
    CCS_NOTREADY = 0,
    CCS_READY = 1,
    CCS_SWITCHOFF = 2,
    CCS_INTERRUPTION = 3,
    CCS_PRECHARGE = 4,
    CCS_INSULATION = 5,
    CCS_ESTOP = 6,
    CCS_MALFUNCTION = 7,
    CCS_INVAID = 15
};

enum can_devices
{
    CAN_DEV1 = 0,
    CAN_DEV2 = 1
};





extern const char* errorListString;

