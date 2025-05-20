# ESPHome External Components Library

This repository contains external components for [ESPHome](https://esphome.io/), focusing on advanced HVAC systems such as VRF air conditioning with Modbus RTU support.

## 📁 Components

### `ac_vrf`

A custom climate controller for VRF air conditioners using Modbus RTU protocol.

#### Features
- Full control over fan coils (on/off, mode, temperature, fan speed)
- System-wide state monitoring and fault detection
- ESPHome `climate` integration with automation support

#### Usage

```yaml
external_components:
  - source: github://your-username/ESPHomeExternalComponentsLib@main
    components: [ac_vrf]

