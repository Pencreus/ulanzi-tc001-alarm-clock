# Ulanzi TC001 alarm clock (ESPHome)

Custom [ESPHome](https://esphome.io) firmware that turns the Ulanzi TC001 pixel clock into a
scheduled alarm clock while keeping what the stock firmware did: clock, date, indoor
temperature/humidity, outdoor weather, ambient-light auto brightness and the three buttons.

- **Alarm schedule** — separate Mon–Fri and Sat–Sun wake times and on/off switches, edited on the
  clock's own web page (`http://<name>.local`) or in Home Assistant. Default 7:10 and 9:00.
- **Any of the three buttons dismisses** the alarm. It stops by itself after 30 minutes.
- **Screens** — time (45 s) with a calendar card and a blue PM dot, then date, indoor
  temp/humidity, outdoor temp + condition (from Home Assistant), next alarm (bell icon), and a
  random animation (matrix rain, rain, rainbow, plasma, fire, rainbow scroller). Left/right buttons
  change screen, middle tap turns the display off/on, middle hold (1 s) cycles brightness.
- **Web UI** (ESPHome `web_server` v3, served from the clock) — wake times, switches, alarm pitch,
  Test Alarm, Dismiss, animation selector, sensor readings, and a firmware upload box.
- Works without Home Assistant (time comes from NTP; only the outdoor screen needs HA).

## Use

1. `cp secrets.yaml.example secrets.yaml` and fill it in.
2. `cp example-clock.yaml my-clock.yaml`, set the name and API key.
3. First flash over USB (see below), then update over Wi-Fi: `esphome run my-clock.yaml --device <ip>`.

## Hardware facts (verified on a real TC001)

| Thing | Pin |
|---|---|
| 32x8 WS2812 matrix | GPIO32, GRB, **row-serpentine** (even rows left→right, odd rows right→left) |
| Buzzer (passive) | GPIO15 |
| Buttons left / middle / right | GPIO26 / GPIO27 / GPIO14 (active low) |
| Light sensor (LDR) / battery ADC | GPIO35 / GPIO34 |
| I²C | SDA GPIO21, SCL GPIO22 — SHT3x at 0x44, DS1307 RTC at 0x68 |

The SHT3x reads several degrees high (heat from LEDs/battery): calibrate `temp_offset_c`.
The buzzer is small; a 50 % duty square wave is the loudest it can get. The alarm sweeps the pitch
so it always crosses the buzzer's resonance. For a louder alarm add an external speaker.

## Flashing notes — read before you start

The TC001's USB serial (CH340) is flaky, especially from macOS: the link can drop every minute or
two mid-transfer, after which the macOS port stays stuck until the cable is physically replugged.

- Use 115200 baud (never 460800+).
- Back up the stock flash first: `esptool read-flash 0x0 0x400000 backup.bin` (115200 baud).
- `flash-clock.sh` writes the image in 64 KB chunks and resumes after each dropout;
  `verify-fix.sh` checks every chunk and rewrites bad ones. Run them from a normal terminal.
  `./flash-clock.sh firmware/<name>.factory.bin`
- After the first successful flash, **update over Wi-Fi only**.
- The stock firmware has a Wi-Fi `/update` page; uploading the ESPHome `.ota.bin` there should avoid
  USB entirely. *Not yet verified.*

## Status

Works on one TC001 (ESP32, 4 MB). Display layout and animations are tuned by eye.
