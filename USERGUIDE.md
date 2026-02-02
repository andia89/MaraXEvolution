# MaraX Evolution - User Manual

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

The machine operates in a finite state machine. You can monitor the state via MQTT (`espresso/status/state`) or Telnet (`status`) or the accompanying HMI device. 

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
| `calibratescale` |   | Starts calibration wizard. Step 1: Tare. | 
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
| `status/brew_mode` | `COFFEE`/`STEAM` | Current Brew Mode. | 
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

Notes: 
 1) BrewMode `STEAM` is similar to a "standard" heatexchanger machine. It heats up much faster and is basically a bang-bang controller (it is a bit smarter than that). Expect, however, that the temperature fluctuates more in this mode. In this mode the machine will do occasional cooling flushes (just like the MaraX standard firmware does). Brewmode `COFFEE` uses the PID to turn the heater on off. It ramps to the target temperature slower but keeps the temperature much more stable.
 2) SteamBoost refers to the behaviour after the shot. If enabled the machine will turn on the heater full blast for 15s after a shot which increases the steam pressure. In turn the temperature of the heatexchanger will fluctuate. If you want to to do a single shot with foamed milk, use this mode. If you want to do multiple shots maybe not. 

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

## 6. HomeAssistant integration
If you want to use home assistant integration the following is a working MQTT section in `configuration.yaml` that can be used in HomeAssistant to tie in the smart coffee machine. 

      mqtt:
        sensor:
          - name: "Heatexchanger Temperature"
            unique_id: espresso_machine_hx_temp
            state_topic: "espresso/sensor/hx_temp"
            unit_of_measurement: "°C"
            device_class: "temperature"
            state_class: "measurement"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Boiler Temperature"
            unique_id: espresso_machine_boiler_temp
            state_topic: "espresso/sensor/boiler_temp"
            unit_of_measurement: "°C"
            device_class: "temperature"
            state_class: "measurement"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Temperature PID output"
            unique_id: espresso_machine_temperature_pidoutput
            state_topic: "espresso/sensor/pidoutput"
            state_class: "measurement"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Temperature PID P Integrator"
            unique_id: espresso_machine_temperature_pint
            state_topic: "espresso/sensor/pterm"
            state_class: "measurement"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Temperature PID I Integrator"
            unique_id: espresso_machine_temperature_iint
            state_topic: "espresso/sensor/iterm"
            state_class: "measurement"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Temperature PID D Integrator"
            unique_id: espresso_machine_temperature_dint
            state_topic: "espresso/sensor/dterm"
            state_class: "measurement"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Pressure"
            unique_id: espresso_pressure
            state_topic: "espresso/sensor/pressure"
            unit_of_measurement: "bar"
            device_class: "pressure"
            state_class: "measurement"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Machine State"
            unique_id: espresso_machine_state
            state_topic: "espresso/status/state"
            icon: mdi:coffee
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Weight"
            unique_id: espresso_machine_weight
            state_topic: "espresso/sensor/weight"
            unit_of_measurement: "g"
            device_class: "weight"
            state_class: "measurement"
            icon: mdi:weight-gram
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Flow Rate"
            unique_id: espresso_machine_flow_rate
            state_topic: "espresso/sensor/flow_rate"
            unit_of_measurement: "g/s"
            state_class: "measurement"
            icon: mdi:water-pump
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
        binary_sensor:
          - name: "Heating"
            unique_id: espresso_heating
            state_topic: "espresso/sensor/heater"
            payload_on: "ON"
            payload_off: "OFF"
            device_class: "heat"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Pump"
            unique_id: espresso_pump
            state_topic: "espresso/sensor/pump"
            payload_on: "ON"
            payload_off: "OFF"
            device_class: "running"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Lever"
            unique_id: espresso_lever
            state_topic: "espresso/sensor/lever"
            payload_on: "LIFTED"
            payload_off: "DOWN"
            icon: mdi:electric-switch
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
        select:
          - name: "Brewmode"
            unique_id: espresso_brewmode_select # Use a new unique_id
            state_topic: "espresso/status/brew_mode"
            command_topic: "espresso/settings/set"
            # This template takes the selected option and formats it for your ESP32
            command_template: "brewmode={{ value | lower }}"
            value_template: "{{ value | upper }}"
            # This is the list of options that will appear in the dropdown
            options:
              - "COFFEE"
              - "STEAM"
            icon: mdi:coffee
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Profiling Mode"
            unique_id: espresso_profmode_select # Use a new unique_id
            state_topic: "espresso/settings/status/profiling_mode"
            command_topic: "espresso/settings/set"
            # This template takes the selected option and formats it for your ESP32
            command_template: "profiling_mode={{ value | lower }}"
            value_template: "{{ value | upper }}"
            # This is the list of options that will appear in the dropdown
            options:
              - "MANUAL"
              - "FLAT"
              - "PROFILE"
            icon: mdi:coffee
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Profiling Control"
            unique_id: espresso_profTarget_select # Use a new unique_id
            state_topic: "espresso/settings/status/profiling_target"
            command_topic: "espresso/settings/set"
            # This template takes the selected option and formats it for your ESP32
            command_template: "profiling_target={{ value | lower }}"
            value_template: "{{ value | upper }}"
            # This is the list of options that will appear in the dropdown
            options:
              - "TIME"
              - "WEIGHT"
            icon: mdi:coffee
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Profiling Target"
            unique_id: espresso_profSource_select # Use a new unique_id
            state_topic: "espresso/settings/status/profiling_source"
            command_topic: "espresso/settings/set"
            # This template takes the selected option and formats it for your ESP32
            command_template: "profiling_source={{ value | lower }}"
            value_template: "{{ value | upper }}"
            # This is the list of options that will appear in the dropdown
            options:
              - "PRESSURE"
              - "FLOW"
            icon: mdi:coffee
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
        switch:
          - name: "Steamboost"
            unique_id: espresso_steamboost
            state_topic: "espresso/status/steam_boost"
            payload_on: "true"
            payload_off: "false"
            command_topic: "espresso/settings/set"
            command_template: "{{ 'enablesteamboost=true' if value == 'true' else 'enablesteamboost=false'}}"
            retain: true
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
        button:
          - name: "Start cleaning cycle"
            unique_id: espresso_cleaning
            command_topic: "espresso/settings/set"
            command_template: "start_cleaning=true"
            retain: false
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Tare Scale"
            unique_id: espresso_tare_scale
            command_topic: "espresso/settings/set"
            command_template: "tare_scale=true"
            retain: false
            icon: mdi:scale
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Scale Calibration"
            unique_id: espresso_start_calibration
            command_topic: "espresso/settings/set"
            command_template: >
              {% if states('sensor.lelit_mara_x_v2_machine_state').startswith('CALIBRATION_') %}
                calibration_step={{ states('number.lelit_mara_x_v2_calibrate_scale_test_weight') }}
              {% else %}
                calibratescale=true
              {% endif %}
            retain: false
            icon: mdi:scale-balance
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
        number:
          - name: "Brew Target Temperature"
            unique_id: espresso_tempsetbrew
            state_topic: "espresso/settings/status/tempsetbrew"
            command_topic: "espresso/settings/set"
            command_template: "tempsetbrew={{ value }}"
            min: 80
            max: 100
            step: 0.1
            mode: slider
            unit_of_measurement: "°C"
            device_class: "temperature"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
          - name: "Steam Target Temperature"
            unique_id: espresso_tempsetsteam
            state_topic: "espresso/settings/status/tempsetsteam"
            command_topic: "espresso/settings/set"
            command_template: "tempsetsteam={{ value }}"
            min: 110
            max: 133
            step: 0.1
            mode: slider
            unit_of_measurement: "°C"
            device_class: "temperature"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
          - name: "Steamboost Target Temperature"
            unique_id: espresso_tempsetsteamboost
            state_topic: "espresso/settings/status/tempsetsteamboost"
            command_topic: "espresso/settings/set"
            command_template: "tempsetsteamboost={{ value }}"
            min: 100
            max: 133
            step: 0.1
            mode: slider
            unit_of_measurement: "°C"
            device_class: "temperature"
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
          # --- NEW: Brew PID Settings ---
          - name: "PID Temperature Kp"
            unique_id: espresso_kp_brew
            state_topic: "espresso/settings/status/kp_temperature"
            command_topic: "espresso/settings/set"
            command_template: "kp_temperature={{ value }}"
            min: 0
            max: 10
            step: 0.01
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
          - name: "PID Temperature Ki"
            unique_id: espresso_ki_brew
            state_topic: "espresso/settings/status/ki_temperature"
            command_topic: "espresso/settings/set"
            command_template: "ki_temperature={{ value | float /1000 }}"
            value_template: "{{ value | float *1000}}"
            min: 0
            max: 10
            step: 0.001
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
          - name: "PID Temperature Kd"
            unique_id: espresso_kd_brew
            state_topic: "espresso/settings/status/kd_temperature"
            command_topic: "espresso/settings/set"
            command_template: "kd_temperature={{ value | float *1000 }}"
            value_template: "{{ value | float /1000 }}"
            min: 0
            max: 10
            step: 0.1
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Profiling Flat Target"
            unique_id: espresso_profiling_flat_value
            state_topic: "espresso/settings/status/profiling_flat_value"
            command_topic: "espresso/settings/set"
            command_template: "profiling_flat_value={{ value }}"
            min: 0
            max: 100 # Assuming 100 is max (e.g., pressure or flow)
            step: 0.5
            mode: slider
            icon: mdi:chart-line-variant
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
          # --- PID Pressure ---
          - name: "PID Pressure Kp"
            unique_id: espresso_kp_pressure
            state_topic: "espresso/settings/status/kp_pressure"
            command_topic: "espresso/settings/set"
            command_template: "kp_pressure={{ value }}"
            min: 0
            max: 50
            step: 0.01
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "PID Pressure Ki"
            unique_id: espresso_ki_pressure
            state_topic: "espresso/settings/status/ki_pressure"
            command_topic: "espresso/settings/set"
            command_template: "ki_pressure={{ value }}"
            min: 0
            max: 50
            step: 0.1
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "PID Pressure Kd"
            unique_id: espresso_kd_pressure
            state_topic: "espresso/settings/status/kd_pressure"
            command_topic: "espresso/settings/set"
            command_template: "kd_pressure={{ value }}"
            min: 0
            max: 50
            step: 0.01
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
          # --- PID Flow ---
          - name: "PID Flow Kp"
            unique_id: espresso_kp_flow
            state_topic: "espresso/settings/status/kp_flow"
            command_topic: "espresso/settings/set"
            command_template: "kp_flow={{ value }}"
            min: 0
            max: 10
            step: 0.1
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "PID Flow Ki"
            unique_id: espresso_ki_flow
            state_topic: "espresso/settings/status/ki_flow"
            command_topic: "espresso/settings/set"
            command_template: "ki_flow={{ value }}"
            min: 0
            max: 10
            step: 0.1
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "PID Flow Kd"
            unique_id: espresso_kd_flow
            state_topic: "espresso/settings/status/kd_flow"
            command_topic: "espresso/settings/set"
            command_template: "kd_flow={{ value }}"
            min: 0
            max: 10
            step: 0.1
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
          # --- Kalman Weight ---
          - name: "Kalman Weight Measurement Error"
            unique_id: espresso_weight_kalman_me
            state_topic: "espresso/settings/status/weight_kalman_me"
            command_topic: "espresso/settings/set"
            command_template: "weight_kalman_me={{ value }}"
            min: 0
            max: 20
            step: 0.1
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Kalman Weight Estimate Error"
            unique_id: espresso_weight_kalman_e
            state_topic: "espresso/settings/status/weight_kalman_e"
            command_topic: "espresso/settings/set"
            command_template: "weight_kalman_e={{ value }}"
            min: 0
            max: 20
            step: 0.1
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Kalman Weight Process Noise"
            unique_id: espresso_weight_kalman_q
            state_topic: "espresso/settings/status/weight_kalman_q"
            command_topic: "espresso/settings/set"
            command_template: "weight_kalman_q={{ value }}"
            min: 0
            max: 1
            step: 0.01
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
      
          # --- Kalman Flow ---
          - name: "Kalman Flow Measurement Error"
            unique_id: espresso_flow_kalman_me
            state_topic: "espresso/settings/status/flow_kalman_me"
            command_topic: "espresso/settings/set"
            command_template: "flow_kalman_me={{ value }}"
            min: 0
            max: 50
            step: 0.1
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Kalman Flow Estimate Error"
            unique_id: espresso_flow_kalman_e
            state_topic: "espresso/settings/status/flow_kalman_e"
            command_topic: "espresso/settings/set"
            command_template: "flow_kalman_e={{ value }}"
            min: 0
            max: 50
            step: 0.1
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Kalman Flow Process Noise"
            unique_id: espresso_flow_kalman_q
            state_topic: "espresso/settings/status/flow_kalman_q"
            command_topic: "espresso/settings/set"
            command_template: "flow_kalman_q={{ value }}"
            min: 0
            max: 1
            step: 0.01
            mode: slider
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
          - name: "Calibrate Scale Test Weight"
            unique_id: espress_calibrate_test_weight
            command_topic: "espresso/dummy/variable_set"
            optimistic: true
            min: 1
            max: 750
            step: 0.5
            retain: true
            mode: box
            icon: mdi:counter
            device:
              identifiers: ["espresso_mqtt"]
              name: "Lelit Mara X V2"
              manufacturer: "Lelit"
              model: "Mara X V2"
