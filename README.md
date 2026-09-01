# Gesture-Controlled 3D-Printed Robotic Arm

A 3-DOF robotic arm controlled in real time by hand gestures, using two ESP32
microcontrollers communicating over **ESP-NOW**. One ESP32 lives on a sensor-equipped
glove; the other drives the arm. Total build cost: **~£152**.

<img width="600" height="400" alt="Movieon16-01-2025at4 50PM-ezgif com-video-to-gif-converter (1)" src="https://github.com/user-attachments/assets/f53afcea-b1a7-4f0c-840f-20f0d88e2ccd" />

## What it does

| Gesture | Arm motion |
|---|---|
| Tilt hand left / right | X-axis rotation (base) |
| Tilt hand up / down | Y-axis elevation |
| Bend middle finger | Gripper open / close |
| Press thumb limit switch | Arm extension |

<img width="1920" height="1080" alt="Untitled Project (4)" src="https://github.com/user-attachments/assets/0975df20-d94e-4f47-a21f-0c3df7944651" />
<img width="1920" height="1080" alt="Untitled Project (3)" src="https://github.com/user-attachments/assets/aa6fd756-fc72-4bce-b925-f3bc3d84f328" />
<img width="1080" height="1080" alt="Untitled Project (8)" src="https://github.com/user-attachments/assets/cb691bb6-a457-4203-9c79-f6447e448688" />
<img width="1920" height="1080" alt="!extend" src="https://github.com/user-attachments/assets/c2db1893-0659-4b31-ab1a-acfdec760e80" />



The glove reads hand orientation from an MPU6050 and finger bend from a flex sensor,
packs the values into a struct, and sends it to the arm's MAC address over ESP-NOW.
The arm's ESP32 unpacks the struct and drives three stepper motors (via L298N drivers)
and one servo.

---

## Hardware

### Glove
| Component | Qty | Notes |
|---|---|---|
| ESP32 DevKit V4 | 1 | Sender |
| MPU6050 | 1 | I²C, gyro + accelerometer |
| SparkFun flex sensor (25 kΩ) | 1 | Mounted on middle finger |
| SPDT limit switch (3-pin) | 1 | Mounted near thumb |

### Arm
| Component | Qty | Notes |
|---|---|---|
| ESP32 DevKit V4 | 1 | Receiver |
| 42BYGH40 stepper motor | 3 | 12 V, 1.8° step angle, ≤1.5 A/phase |
| L298N motor driver | 3 | Dual H-bridge |
| Tower Pro MG90S servo | 1 | Gripper, 1.8 kg·cm |
| 12 V bench supply | 1 | ~84 W peak with all motors running |

### Mechanical
The arm is the **RobotArm** originally designed by **Florin Tobler**
([ftobler/robotArm](https://github.com/ftobler/robotArm) ·
[Thingiverse](https://www.thingiverse.com/thing:1718984)), built here in the
belt-driven community variant maintained by **20sffactory**
([community_robot_arm](https://github.com/20sffactory/community_robot_arm) ·
[20sffactory.com/robot](https://www.20sffactory.com/robot/about)).

Approx. 20 cm tall, PLA, no part larger than 150 mm, so it prints on compact printers.

> **Mechanical files are not redistributed in this repository.** Tobler's design is
> licensed CC BY-NC — download the STLs from the sources above. Only the control-board
> enclosure in `hardware/case/`, which is original to this project, is included here.

---

## Repository layout

```
.
├── firmware/
│   ├── glove_sender/        # ESP32 on the glove
│   └── arm_receiver/        # ESP32 on the arm
├── tests/                   # Standalone sketches used during bring-up
│   ├── mac_address/         # Print the receiver's MAC
│   ├── servo_test/
│   ├── flex_test/
│   ├── stepper_test/
│   └── mpu6050_test/
├── hardware/
│   ├── schematics/          # Glove + arm circuit diagrams
│   ├── pcb/                 # EasyEDA designs (future work)
│   └── case/                # 3D-printed enclosure for the control board (original)
│                            # NOTE: arm STLs are NOT included — see Mechanical above
├── docs/
│   ├── media/               # Photos, demo GIF
│   └── thesis.pdf           # Full MSc thesis
└── README.md
```

---

## Getting started

**Requirements:** Arduino IDE with ESP32 board support, plus the `Adafruit MPU6050`,
`Adafruit Unified Sensor`, and `ESP32Servo` libraries.

1. Flash `tests/mac_address/` to the **arm** ESP32 and note the MAC address from the
   serial monitor.
2. Paste that MAC into the `broadcastAddress[]` array in `firmware/glove_sender/`.
3. Flash `firmware/glove_sender/` to the glove ESP32 and `firmware/arm_receiver/` to
   the arm ESP32.
4. Power the motor drivers from a 12 V supply. **Do not** power the steppers from the
   ESP32's 5 V rail.
5. Place the glove flat on a surface at power-on — the MPU6050 zeroes its offsets on
   boot and must be stationary.

---

## Calibration notes

These are the measured values from this build; yours may differ.

- **Flex sensor (ADC):** ~2730 straight, ~850 fully bent. Threshold for gripper close: **< 900**.
- **Gyro range:** X axis −6.98 to +9.89 rad/s, Y axis −7.78 to +8.50 rad/s.
- **Stepper steps/revolution:** 200 (datasheet) → 220 (measured, unloaded) → 880 (under
  full arm load, 4:1 gear ratio). Effective precision under load: 0.41°.
- **Recommended motor speed:** 30–40 RPM, 22 steps per commanded movement.

---

## Measured performance

- **ESP-NOW latency:** 3–5 ms. No perceptible lag at 10 m; rated to 220 m line-of-sight.
- **Max reach:** ~25 cm.
- **Gripper payload:** 100 g reliably; >120 g risks damaging the printed gripper.
  Objects should be ≥1.5 cm diameter with a non-smooth surface.
- **Peak power draw:** ~84 W at 12 V.

---

## Known limitations

- Lightweight payloads only.
- The glove ESP32 is tethered for power, which limits operating range.
- L298N drivers overheat above ~4 A sustained, causing the steppers to jerk.
- MPU6050 requires re-calibration on every power cycle.

---

## Future work

- Custom PCBs (glove + arm) to replace the veroboard — EasyEDA designs in `hardware/pcb/`.
- Camera-based gesture recognition using OpenCV + MediaPipe on a Raspberry Pi 4,
  removing the glove entirely.

---

## Acknowledgements

The arm mechanics are the **RobotArm** by **Florin Tobler**
(<https://github.com/ftobler/robotArm> · <https://www.thingiverse.com/thing:1718984>),
licensed CC BY-NC. This build uses the belt-driven community variant maintained by
**20sffactory** (<https://github.com/20sffactory/community_robot_arm>).
Mechanical files are not redistributed here — please obtain them from the sources above.

Inverse kinematics follow the approach described by A. Osman and A. Aldabbagh in
*Enabling STEM Education Through a Remote, Gesture-Controlled Surgical Robot*, which
uses the same arm platform.

Built as a thesis project

If you cite the arm design:

```bibtex
@misc{tobler_robotarm,
  author       = {Tobler, Florin},
  title        = {{RobotArm}: Software for a {3D} Printed Robot Arm},
  year         = {2016},
  howpublished = {\url{https://github.com/ftobler/robotArm}},
  note         = {Licensed CC BY-NC}
}
```

---

## License

This repository contains work under two different licenses:

| Path | License | Notes |
|---|---|---|
| `firmware/`, `tests/` | MIT | Original ESP32 firmware written for this project |
| `hardware/case/`, `hardware/pcb/`, `hardware/schematics/` | CC BY-NC 4.0 | Original to this project; matches the upstream arm's terms |
| `docs/thesis.pdf` | All rights reserved | © <your name>, <year> |
| Arm STL / CAD files | **Not included** | CC BY-NC by Florin Tobler — download from upstream |

The upstream arm design is CC BY-NC, which is not an OSI-approved open-source license.
Nothing derived from it may be relicensed under MIT or used commercially without
permission from the original author. The firmware in this repository is independent
work and is unaffected by that restriction.

<!-- TODO: add a LICENSE file (MIT) at the repo root and fill in your name/year above. -->
