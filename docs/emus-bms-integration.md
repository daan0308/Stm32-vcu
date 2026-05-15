# EMUS G1 BMS Integration

This document describes how the ZombieVerter VCU integrates with the EMUS G1 BMS, what data is exchanged, where display values come from, and what the user needs to configure on both sides.

---

## Overview

The VCU communicates with the EMUS BMS over CAN. The BMS broadcasts cell measurements periodically; the VCU polls for frames it needs, decodes them, and populates display parameters. Charging is controlled by the VCU acting on diagnostic protection flags broadcast by the EMUS — the EMUS's own configured limits are used directly, so voltage and temperature thresholds do **not** need to be duplicated in the VCU web interface.

The default CAN base address is **0x300**. All EMUS frames use the scheme `Base + Sub-ID`. If your BMS is configured with a different base address, the registered CAN IDs in `emusbms.cpp` must be updated to match.

---

## CAN Frames

The VCU polls and decodes the following frames every 100 ms:

| Frame ID | Sub-ID | Description |
|----------|--------|-------------|
| 0x301 | Base+1 | Battery Voltage Overall Parameters |
| 0x305 | Base+5 | State of Charge Parameters |
| 0x306 | Base+6 | Energy Parameters |
| 0x307 | Base+7 | Diagnostic Codes |
| 0x308 | Base+8 | Cell Temperature Overall Parameters |

### 0x301 — Battery Voltage Overall Parameters

| Signal | Bytes | Format | Scaling |
|--------|-------|--------|---------|
| Min cell voltage | 0 | uint8 | 0.01 V/lsb + 2.00 V |
| Max cell voltage | 1 | uint8 | 0.01 V/lsb + 2.00 V |
| Pack voltage | 3,4,5,6 | uint32, non-sequential¹ | 0.01 V/lsb |

¹ Non-sequential byte order per EMUS G1 spec v3.1.0: byte 5 = MSB, byte 3 = 2nd, byte 6 = 3rd, byte 4 = LSB.

### 0x305 — State of Charge Parameters

| Signal | Bytes | Format | Scaling |
|--------|-------|--------|---------|
| Pack current | 0–1 | int16, MSB-first | 0.1 A/lsb (negative = discharging) |
| User SOC | 5–6 | uint16, MSB-first | 1 %/lsb |

**User SOC vs raw SOC:** The VCU uses *User SOC*, which reflects the usable charge window configured in the EMUS Control Panel. If you set the usable range to 20–90 % in the EMUS Control Panel, User SOC will report 0 % at 20 % raw and 100 % at 90 % raw. This is the correct value to display and act on.

The 0x305 frame is also used as the **heartbeat** — receiving it resets the BMS timeout counter. If this frame stops arriving, the VCU considers the BMS offline and stops charging.

### 0x306 — Energy Parameters

| Signal | Bytes | Format | Scaling |
|--------|-------|--------|---------|
| Remaining energy | 2–3 | uint16, MSB-first | 10 Wh/lsb (stored as kWh) |

### 0x307 — Diagnostic Codes

The diagnostic codes frame carries protection flags set by the EMUS when any measured value exceeds a limit configured in the **EMUS Control Panel**. The VCU reads these flags directly to make charge/no-charge decisions — see [Charging Control](#charging-control) below.

| Bit | Flag | Relevant for charging |
|-----|------|-----------------------|
| Byte 0, bit 1 | CellOverVoltage | Yes — stops charging |
| Byte 0, bit 3 | ChrgOverCurrent | Yes — stops charging |
| Byte 0, bit 4 | CellModuleOverHeat | Yes — stops charging |
| Byte 0, bit 5 | Leakage (insulation fault) | Yes — stops charging |
| Byte 1, bit 3 | CellOverHeat | Yes — stops charging |
| Byte 1, bit 6 | PackOverVoltage | Yes — stops charging |
| Byte 1, bit 7 | CellUnderHeat (too cold) | Yes — stops charging |

### 0x308 — Cell Temperature Overall Parameters

| Signal | Bytes | Format | Scaling |
|--------|-------|--------|---------|
| Min cell temperature | 0 | uint8 | 1 °C/lsb − 100 °C |
| Max cell temperature | 1 | uint8 | 1 °C/lsb − 100 °C |
| Avg cell temperature | 2 | uint8 | 1 °C/lsb − 100 °C |

---

## VCU Display Parameters

These are read-only values visible in the VCU web interface and available for CAN TX via the CAN map.

| Parameter | Unit | Source |
|-----------|------|--------|
| `BMS_Vmin` | V | Min cell voltage from 0x301 |
| `BMS_Vmax` | V | Max cell voltage from 0x301 |
| `BMS_Tmin` | °C | Min cell temperature from 0x308 |
| `BMS_Tmax` | °C | Max cell temperature from 0x308 |
| `BMS_Tavg` | °C | Avg cell temperature from 0x308 |
| `SOC` | % | User SOC from 0x305 |
| `KWh` | kWh | Remaining energy from 0x306 |
| `idc` | A | Pack current from 0x305 |
| `udc2` | V | Pack voltage from 0x301 |
| `power` | kW | Calculated: pack voltage × pack current / 1000 |
| `range` | km | Predicted remaining range (rolling average, see below) |
| `consumption` | Wh/km | Rolling average energy consumption (see below) |
| `BMS_ChargeLim` | A | 0 when charging blocked; EMUS-requested current when J1939 active; 9998 (unlimited) if J1939 not yet active |

### Precharge threshold auto-calibration

When pack voltage is above 50 V, the VCU automatically sets `udcsw` (the precharge completion threshold) to `packVoltage − 20 V`. This mirrors the convention used by the Leaf BMS integration and means you do not need to manually configure `udcsw` when using the EMUS BMS.

### Predicted range and consumption

The VCU maintains a rolling average of energy consumption in Wh/km using an exponential moving average with a ~30-second time constant. It only updates while the vehicle is moving faster than 5 kph and the motor is drawing power (motoring only — regen and standstill are excluded to avoid skewing the estimate).

- **`range`** = remaining kWh × 1000 / avgConsumption. Updates only when the BMS is reporting valid energy data.
- **`consumption`** = the rolling average in Wh/km. Starts at 150 Wh/km on power-up. Divide by 10 to get kWh/100 km.

Both values are available for display transmission on **CAN message 103** (`VCU_Values103`). Configure the CAN map in the VCU web interface: `range` → bytes 0–1, scale 0.1; `consumption` → bytes 2–3, scale 0.1.

---

## Charging Control

The VCU controls charging via the `BMS_ChargeLim` parameter:
- **EMUS-requested current** — when J1939 charger mimic is active and charging is permitted, `BMS_ChargeLim` tracks the EMUS CC/CV taper profile directly
- **9998 A** — charging permitted but J1939 not yet active (effectively unlimited, the charger's own limits apply)
- **0 A** — charging blocked

Charging is blocked when **any** of the following is true:

1. **BMS timeout** — the 0x305 heartbeat frame has not been received within the configured `BMS_Timeout` period. This protects against a BMS that has gone silent.

2. **EMUS protection flag active** — one of the diagnostic flags in 0x307 indicates a protection event (over-voltage, over-temperature, under-temperature, leakage, or over-current). These flags are raised by the EMUS based on limits configured in the **EMUS Control Panel** — the VCU trusts these directly.

> **Important:** The VCU web interface contains parameters `BMS_VminLimit`, `BMS_VmaxLimit`, `BMS_TminLimit`, and `BMS_TmaxLimit`. These are **not used** by the EMUS BMS driver. They exist for other BMS types (SimpBMS, DaisychainBMS) that do not broadcast diagnostic flags. For EMUS, set your protection limits in the EMUS Control Panel only.

---

## User-Configurable VCU Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `BMS_Timeout` | 10 s | How long to wait without a 0x305 frame before considering the BMS offline and blocking charging. |
| `BMSCan` | CAN1 | Which CAN interface the EMUS is connected to. |

---

## EMUS Control Panel Requirements

The following must be configured in the EMUS G1 Control Panel software for correct VCU integration:

- **User SOC range**: set to 0–100 % unless you intentionally want to restrict the reported range. If set to e.g. 20–80 %, the VCU's `SOC` display and charge cutoff will reflect that window.
- **CAN base address**: must match the IDs registered in the VCU (default 0x300).
- **Protection limits**: configure cell voltage min/max, temperature min/max, and current limits here. The VCU reads the resulting diagnostic flags; it does not duplicate these thresholds.
- **Neuro messages (0x522/0x523)**: not used by this integration. These are intended for TM4/Dana TM4 Neuro inverter setups and are not transmitted by default.

---

## J1939 Charger Mimic

### Overview

The EMUS G1 BMS supports J1939 chargers. When a J1939 charger is present on the bus, the EMUS responds with its requested charge voltage and current, automatically tapering current as the pack approaches full charge (CC/CV profile). The VCU exploits this to get a fine-grained power setpoint rather than a simple on/off signal.

The VCU mimics a J1939 charger on the CAN bus. The EMUS responds with its charge setpoints, and the VCU passes those setpoints directly to the Tesla Gen2 charger via the `BMS_ChargeLim` parameter.

### CAN Frames

| Frame ID | Direction | Description |
|----------|-----------|-------------|
| 0x18FF50E5 | VCU → EMUS | Charger status broadcast (sent every 1 s) |
| 0x1806E5F4 | EMUS → VCU | Requested charge voltage and current |

Both are 29-bit extended CAN frames. The ZombieVerter CAN driver handles extended frames transparently — no special configuration is needed.

#### 0x18FF50E5 — Charger→BMS status

Sent by the VCU every 1 second. The EMUS requires this message within every 5 seconds; without it the EMUS will not transmit 0x1806E5F4.

| Bytes | Content |
|-------|---------|
| 0–1 | Output voltage, uint16 MSB-first, 0.1 V/lsb (VCU reports actual pack voltage) |
| 2–3 | Output current, uint16 MSB-first, 0.1 A/lsb (reported as 0 — not available from Tesla charger feedback) |
| 4 | Status byte: 0x00 = all normal |
| 5–7 | Reserved, 0x00 |

#### 0x1806E5F4 — BMS→Charger charge setpoints

Sent by the EMUS in response to 0x18FF50E5. Contains the EMUS CC/CV profile output.

| Bytes | Signal | Format | Scaling |
|-------|--------|--------|---------|
| 0–1 | Max allowable charge voltage | uint16, MSB-first | 0.1 V/lsb |
| 2–3 | Max allowable charge current | uint16, MSB-first | 0.1 A/lsb |
| 4 | Control byte: bit 0 = stop charging (1 = stop, 0 = charge) | — | — |

### How BMS_ChargeLim tracks the EMUS taper

Once 0x1806E5F4 has been received at least once (J1939 active), `MaxChargeCurrent()` returns the EMUS-requested current directly. `BMS_ChargeLim` is set from this value every 100 ms.

The Tesla Gen2 charger power setpoint is calculated as:

```
calcBMSpwr = HVvolts × BMS_ChargeLim   [W]
```

As the EMUS reduces its requested current during the CV phase, `BMS_ChargeLim` falls, and the Tesla charger reduces output power proportionally — no VCU-side threshold logic required.

Before 0x1806E5F4 is first received (J1939 not yet active), `BMS_ChargeLim` falls back to 9998 A (unlimited), so charging is not blocked while the EMUS negotiates.

### Verified live values

With the vehicle at rest and BMS connected, the following were observed in Cangaroo on the CANable SLCAN adapter:

- **0x18FF50E5**: visible as a 29-bit extended frame, transmitted every 1 s by the VCU
- **0x1806E5F4**: EMUS response; decoded values:
  - Bytes 0–1 = `0x0CA5` → **323.7 V** requested charge voltage
  - Bytes 2–3 = `0x03C0` → **96.0 A** requested charge current at rest

During active charging the EMUS will taper the requested current as the pack voltage rises toward the target.

### Requirements

**CAN bus speed:** The J1939 standard specifies 250 kbps. If your EMUS is on a 500 kbps bus, check whether the EMUS firmware supports J1939 mode at that speed — refer to the EMUS G1 release notes. The VCU CAN speed is set in the VCU web interface and must match the bus.

**EMUS Control Panel:** J1939 charger support may need to be enabled in the EMUS configuration. Refer to the EMUS G1 Control Panel documentation for the relevant setting. Protection limits (voltage, temperature, current) continue to be configured in the EMUS Control Panel and are enforced via the diagnostic codes frame (0x307) — see [Charging Control](#charging-control).

---

## Timeout Behaviour

The BMS timeout counter decrements every 100 ms. It is reset to `BMS_Timeout × 10` each time a valid 0x305 frame is received. If the counter reaches zero:

- All cell/temperature display values are set to 0
- `idc` and `udc2` are set to 0  
- Charging is blocked (`BMS_ChargeLim` = 0)

This ensures that a silent or disconnected BMS fails safe — charging stops rather than continuing unprotected.
