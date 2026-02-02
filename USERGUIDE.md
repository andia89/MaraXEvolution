# MaraX V2 Custom Espresso Firmware - User Manual

This manual documents the operation, configuration, and API references for the custom ESP32 firmware designed for the Lelit MaraX V2 modification project.

## 1. Getting Started

### Initial Setup (WiFi)

1. On first boot, the machine creates a WiFi Access Point named **`MaraX-Setup`**.

2. Connect to this network with your phone or laptop.

3. A captive portal should open automatically (or visit `192.168.4.1`).

4. Enter your home WiFi credentials.

5. The machine will reboot and connect to your local network.

### Hardware Configuration

**Optional Hardware**
The system supports the following optional add-ons (strongly recommended to unlock the full potential):

* **[MaraXEvolution-Scale](https://github.com/andia89/MaraXEvolution-Scale)** For gravimetric dosing and flow profiling.

* **[MaraXEvolution-HMI](https://github.com/andia89/MaraXEvolution-HMI)** A dedicated display connected via ESP-NOW.

**Setting MQTT Credentials**
To enable remote monitoring, configure your broker details using one of the following methods:

* **Via Telnet:** Connect to Port 23 and use the `set` command.

  * Example: `set mqtt_server=192.168.1.10`

  * Example: `set mqtt_user=myUser`

  * Example: `set mqtt_password=myPassword`

* **Via HMI Screen:** Use the configuration portal on the MaraXEvolution-HMI.

## 2. Machine States & Feedback

The machine operates in a finite state machine. You can monitor the state via MQTT (`espresso/status/state`) or Telnet (`status`).

| State | Description | 
 | ----- | ----- | 
| **INIT** | System initialization (Sensors, WiFi). | 
| **HEATING** | Active heating to reach `tempsetbrew`. | 
| **IDLE** | Target temperature reached and stable. | 
| **BREWING** | Shot in progress (Pump active, Profiling active). | 
| **STEAM_BOOST** | Post-shot steam boost (active for 60s or until cancelled). | 
| **STANDBY** | Low power mode after 15 minutes of inactivity. | 
| **COOLING_FLUSH** | Automated flush to bring HX temperature down. | 
| **CLEANING** | Automated backflush cycle states. | 
| **WATER_EMPTY** | Tank empty detected. Pump/Heater disabled. | 
| **BOILER_EMPTY** | Steam boiler level low. Auto-fill active. | 
| **ERROR** | Critical sensor failure (NTC short/disconnect). Heater disabled. | 

## 3. Telnet Command Reference

Connect to the machine using a Telnet client (e.g., PuTTY) on **Port 23** of the machine's IP address.

### General Commands

| Command | Arguments | Description | 
 | ----- | ----- | ----- | 
| `help` |  | Lists available commands. | 
| `status` |  | Prints full system dashboard (Temps, PID, RSSI). | 
| `reboot` |  | Restarts the ESP32. | 
| `macaddress` |  | Prints the device WiFi MAC address. | 
| `lasterror` |  | Prints the last recorded critical error message. | 
| `flush` | `<ms>` | Runs the pump for X milliseconds (e.g., `flush 2000`). | 
| `debug` |  | Toggles **DEBUG** mode (Unlocks hardware control). | 
| `set` | `<key>=<val>` | Manually set a setting (see Settings Reference). | 

### Scale & Calibration

| Command | Arguments | Description | 
 | ----- | ----- | ----- | 
| `tare_scale` |  | Zeros the scale. | 
| `calibratescale` | `<weight_g>` | Starts calibration wizard. Step 1: Tare. | 
| `calibratenext` | `<weight_g>` | Step 2: Place known weight (arg) and calculate factor. | 

### Debug Hardware (Requires DEBUG Mode)

*Warning: These commands bypass safety checks.*

| Command | Arguments | Description | 
 | ----- | ----- | ----- | 
| `heater` | `on` / `off` | Manually toggle SSR. | 
| `pump` | `on` / `off` | Manually toggle Pump Relay. | 
| `fillvalve` | `on` / `off` | Manually toggle Solenoid. | 
| `dimmer` | `0-100` | Set pump Triac power percentage. | 
| `pidoutput` | `0-100` / `auto` | Force manual heater duty cycle or return to Auto. | 
| `readadc` | `0-3` | Read raw ADS1115 voltage on channel X. | 
| `readweight` |  | Read raw ADS1232 scale data. | 

## 4. MQTT Topic Reference

**Broker Configuration:** Set via the WiFi portal or `set` command.
**Root Topic:** `espresso/`

### Status & Sensors (Read-Only)

| Topic | Payload | Description | 
 | ----- | ----- | ----- | 
| `status/state` | String | Current Machine State. | 
| `status/brew_mode` | `COFFEE`/`STEAM` | Current Mode. | 
| `status/steam_boost` | `true`/`false` | Steam boost active status. | 
| `sensor/boiler_temp` | Float | Main Boiler Temp (°C). | 
| `sensor/hx_temp` | Float | Group/HX Temp (°C). | 
| `sensor/pressure` | Float | Pressure (Bar). | 
| `sensor/weight` | Float | Scale Weight (g). | 
| `sensor/flow_rate` | Float | Flow Rate (g/s). | 
| `sensor/heater` | `ON`/`OFF` | SSR State. | 
| `sensor/pump` | `ON`/`OFF` | Pump Relay State. | 
| `sensor/lever` | `LIFTED`/`DOWN` | Brew Lever State. | 
| `sensor/pidoutput` | Float | Heater Duty Cycle (%). | 
| `sensor/pterm` | Float | PID Proportional Term (Debug). | 
| `sensor/iterm` | Float | PID Integral Term (Debug). | 
| `sensor/dterm` | Float | PID Derivative Term (Debug). | 

### Control & Configuration (Write-Only)

**Topic:** `espresso/settings/set`
**Payload Format:** `key=value`

#### Temperature Settings

* `tempsetbrew=93.0` (Target Brew Temp)

* `tempsetsteam=135.0` (Target Steam Temp)

* `tempsetsteamboost=125.0` (Boost Temp after shot)

* `steamboost=true` (Enable/Disable boost feature)

#### PID Tuning

* `kp_temperature`, `ki_temperature`, `kd_temperature`

* `kp_pressure`, `ki_pressure`, `kd_pressure`

* `kp_flow`, `ki_flow`, `kd_flow`

#### Profiling Configuration

* `profiling_mode`: `manual`, `flat`, or `profile`.

* `profiling_target`: `time` or `weight` (X-axis source).

* `profiling_source`: `pressure` or `flow` (Y-axis control).

* `profiling_flat_value`: Float (e.g., `9.0` for 9 bar flat profile).

#### Operations

* `tare_scale=true`: Tare the scale.

* `start_cleaning=true`: Start the 10-step backflush cycle.

* `request=true`: Request the machine to republish all settings.

## 5. Profiling Data Structure

To upload a complex shot profile, send a JSON payload to `espresso/settings/set` with the key `profile_data`.

**JSON Structure:**

```json
{
  "n": "Profile Name",  // String: Name of profile
  "m": 1,               // Int: 0 = Ramped (Interpolated), 1 = Stepped (Hard jumps)
  "s": [                // Array: List of steps
    [9.0, 5.0],         // [Setpoint, Trigger]
    [6.0, 15.0]
  ]
}
```

**Understanding "Trigger":**

* If `profiling_target` is `time`: Trigger is **duration** of step in seconds.

* If `profiling_target` is `weight`: Trigger is **target weight** in grams.

**Understanding "Setpoint":**

* If `profiling_source` is `pressure`: Setpoint is **Bar**.

* If `profiling_source` is `flow`: Setpoint is **mL/s**.

**Example Payload:**
`profile_data={"n":"Soft Preinfusion","m":1,"s":[[2.0,5.0],[9.0,25.0]]}`
*(2 Bar for 5 seconds, then 9 Bar for 25 seconds)*
