# EMUS G1 BMS Integration

## Overview

The EMUS G1 BMS is supported via the `EmusBMS` driver (`src/emusbms.cpp`). Select it by
setting `BMS_Mode = 6` (EmusBMS) in the VCU parameter configuration.

---

## Protection Responsibility

The VCU supports two discharge protection models, selected automatically based on `BMS_Mode`.
The active model is visible in the `BMS_ProtSrc` parameter.

| `BMS_ProtSrc` | Model | Who decides |
|---------------|-------|-------------|
| `0` (VCU) | VCU thresholds | Protection based on `BMS_VminLimit`, `BMS_TminLimit`, `BMS_TmaxLimit` configured in the VCU. Used by all BMS modes except EMUS. |
| `1` (BMS) | BMS-native | Protection based on the EMUS protection and reduction flags from frame `0x307`. Thresholds live in the EMUS Control Panel, not in the VCU. |

In both models the output is the same: `BMS_DischargeOk` and `BMS_DischargeLevel` are written
every 100 ms and consumed by `ProcessThrottle()` to apply derating. The VCU's own derating
parameters (`udcmin`, `udclim`, `tmphsmax`, etc.) always apply on top of either model.

---

## CAN Frames

Default base address: `0x300`. All frame IDs below assume this default.

| ID | Direction | Description |
|----|-----------|-------------|
| `0x301` | EMUS → VCU | Battery Voltage Overall Parameters (min/max/avg cell V, pack V) |
| `0x305` | EMUS → VCU | State of Charge Parameters (pack current, user SOC) |
| `0x306` | EMUS → VCU | Energy Parameters (remaining kWh) |
| `0x307` | EMUS → VCU | Diagnostic Codes (protection flags, reduction flags) |
| `0x308` | EMUS → VCU | Cell Temperature Overall Parameters (min/max/avg) |
| `0x380` | VCU → EMUS | Configuration Parameter request (startup threshold query) |
| `0x380` | EMUS → VCU | Configuration Parameter response |
| `0x18FF50E5` | VCU → EMUS | J1939 charger mimic (sent every 1 s) |
| `0x1806E5F4` | EMUS → VCU | J1939 charge limits (voltage, current, stop bit) |

The VCU polls the EMUS by sending zero-length requests on `0x301`, `0x305`, `0x306`, `0x307`,
and `0x308` every 100 ms. The EMUS responds with current data.

---

## Charging

`ChargeAllowed()` uses the EMUS protection flags from `0x307` as the sole gate for charging.
No cell voltage or temperature thresholds need to be configured in the VCU.

Charging stops on any of the following protection flags:

| Bit | Flag |
|-----|------|
| 1 | CellOverVoltage |
| 3 | ChargeOverCurrent |
| 4 | CellModuleOverHeat |
| 11 | CellOverHeat |
| 14 | PackOverVoltage |
| 15 | CellUnderHeat (too cold to charge) |

### J1939 Charger Mimic

While `opmode == MOD_CHARGE` the VCU sends `0x18FF50E5` every second to mimic a J1939
charger. The EMUS responds with `0x1806E5F4` containing its requested charge voltage and
current for the current CC/CV phase. `MaxChargeCurrent()` returns this value directly, so
the charger's power setpoint tracks the EMUS profile automatically. Any charger that
consumes `MaxChargeCurrent()` will benefit from this — it is not specific to any particular
charger model.

The frame is **not sent outside charge mode**. Sending it unconditionally causes the EMUS
to set the `ChargerConnected` protection flag (frame `0x307` bit 10) during driving, which
is misleading and can interfere with protection logic. The J1939 receive state
(`j1939Active`) is cleared on charge mode exit so stale current limits are not carried over
into the next session.

---

## Drive Derating

Drive derating is two-tiered, matching the EMUS's own protection vs. reduction distinction.
This applies when `BMS_ProtSrc = 1` (BMS-native). When `BMS_ProtSrc = 0` (VCU thresholds),
only the binary protection tier applies (no proportional reduction).

### Startup Threshold Query

At power-on, the VCU queries the EMUS for two configuration parameters via `0x380`:

| Param ID | Name | Used for |
|----------|------|----------|
| `0x0004` | Cell Under-Voltage Protection Activate Value | Hard cutoff threshold |
| `0x0008` | Low Cell Voltage Reduction Activate Value | Proportional derating threshold |

Both are encoded as `uint8_t`, `0.01 V/lsb`, offset `+200` (e.g. raw `80` → `2.80 V`).

Queries are retried every 500 ms until both are answered. Until then, the low-cell-voltage
reduction falls back to a fixed 50 % derating. The received values are visible in
`BMS_UVProtThr` and `BMS_LowVRedThr` (both show `0.00` until the query completes).

### Tier 1 — Protection Flags (hard zero torque)

If any of the following protection flags is set, forward drive torque is immediately zeroed,
`ERR_BMSDISCHARGE` is posted, and `TorqDerate` bit 5 (value 32) is set.

| Bit | Flag |
|-----|------|
| 0 | CellUnderVoltage |
| 2 | DischargeOverCurrent |
| 13 | PackUnderVoltage |

### Tier 2 — Reduction Flags (proportional derating)

Reduction flags indicate a warning condition that warrants reduced — not zero — power.
The VCU computes a multiplier in `[0.0, 1.0]` applied to forward torque. `TorqDerate`
bit 6 (value 64) is set when reduction is active.

| Bit | Flag | Derating method |
|-----|------|-----------------|
| 0 | LowCellVoltage | Proportional between `cellUVProtectionThreshold` and `lowCellVReductionThreshold` using live `BMS_Vmin` |
| 1 | HighDischargeCurrent | Fixed 75 % |
| 2 | HighCellModuleTemperature | Fixed 50 % |
| 5 | HighCellTemperature | Fixed 50 % |

When multiple flags are active simultaneously the most restrictive (lowest) level is applied.

#### Low Cell Voltage — proportional detail

```
band  = lowCellVReductionThreshold − cellUVProtectionThreshold
level = clamp((BMS_Vmin − cellUVProtectionThreshold) / band, 0, 1)
```

At `BMS_Vmin == lowCellVReductionThreshold`: `level = 1.0` (full torque)  
At `BMS_Vmin == cellUVProtectionThreshold`:  `level = 0.0` (zero torque, just before hard cutoff)

### Threshold Hysteresis

Every protection and reduction threshold in the EMUS has a separate activate and deactivate
value, both configurable in the EMUS Control Panel (for example: `CellUnderVoltage` activates
at 2.70 V, deactivates at 2.80 V). The EMUS applies this hysteresis internally before setting
or clearing a flag in frame `0x307`.

The VCU acts on the flags themselves — it does not perform its own voltage or temperature
threshold comparisons for the binary on/off decisions. By the time the VCU sees a flag set
or cleared, the EMUS has already applied its own hysteresis. There is therefore no risk of
jitter at the threshold boundary in the VCU. Adding a second hysteresis layer would be
counterproductive: the VCU could hold discharge blocked even after the EMUS had already
cleared the flag.

The one place where the VCU does its own voltage arithmetic is the proportional low-cell-
voltage derating level shown above. This calculation only runs while the `LowCellVoltage`
reduction flag is already active. Because it produces a continuous smooth output rather than
a binary decision, voltage jitter near the band edges produces smooth torque variation rather
than sudden cut/restore — which is the intended behaviour.

> **Note:** The VCU queries the **activate** thresholds at startup (`0x0004`, `0x0008`). The
> EMUS also has corresponding deactivate thresholds (`0x0005`, `0x0009`) which are not
> queried. The activate values are used as the band boundaries in the proportional calculation.
> The difference between activate and deactivate thresholds is typically ≤ 100 mV, so this
> is a minor approximation.

---

## Observable Parameters

| Parameter | ID | Description |
|-----------|----|-------------|
| `BMS_Vmin` | 2084 | Lowest cell voltage (live, from frame `0x301`) |
| `BMS_Vmax` | 2085 | Highest cell voltage (live, from frame `0x301`) |
| `BMS_Tmin` | 2086 | Lowest cell temperature |
| `BMS_Tmax` | 2087 | Highest cell temperature |
| `BMS_Tavg` | 2103 | Average cell temperature |
| `BMS_ChargeLim` | 2088 | Maximum charge current (from J1939 or 9998 A if J1939 not active) |
| `BMS_DischargeOk` | 9005 | `1` = discharge allowed, `0` = protection active |
| `BMS_DischargeLevel` | 9006 | Discharge torque multiplier in % (100 = full, 0 = none) |
| `BMS_UVProtThr` | 9007 | Cell under-voltage protection threshold read from EMUS at startup (0 = not yet received) |
| `BMS_LowVRedThr` | 9008 | Low cell voltage reduction threshold read from EMUS at startup (0 = not yet received) |
| `BMS_ProtSrc` | 9009 | Active protection source: `0` = VCU thresholds, `1` = BMS-native |
| `TorqDerate` | 2102 | Bitmask of active derating reasons (bit 32 = protection, bit 64 = reduction) |
| `SOC` | 2015 | State of charge (%) from EMUS |
| `KWh` | 2013 | Remaining energy (kWh) from EMUS |

---

## TorqDerate Bitmask Reference

| Bit (value) | Reason |
|-------------|--------|
| 1 | UDC below `udcmin` |
| 2 | UDC above `udclim` (regen limited) |
| 4 | IDC below `idcmin` |
| 8 | IDC above `idcmax` |
| 16 | Heatsink or motor temperature above `tmphsmax`/`tmpmmax` |
| 32 | BMS protection — discharge blocked |
| 64 | BMS reduction — torque proportionally reduced (EMUS only) |

---

## Configuration

| Parameter | Purpose |
|-----------|---------|
| `BMS_Mode` | Set to `6` (EmusBMS) |
| `BMS_Timeout` | Seconds without a frame `0x305` before BMS is considered offline |
| `BMSCan` | CAN interface the EMUS is connected to |

No VCU-side cell voltage or temperature thresholds need to be configured when using the
EMUS BMS (`BMS_ProtSrc = 1`). All thresholds are read from the EMUS at startup.
`BMS_VminLimit`, `BMS_TminLimit`, and `BMS_TmaxLimit` are not used in this mode.
