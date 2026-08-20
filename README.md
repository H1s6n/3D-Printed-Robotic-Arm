# Gesture-Controlled 3D-Printed Robotic Arm

A 3-DOF robotic arm controlled in real time by hand gestures, using two ESP32
microcontrollers communicating over **ESP-NOW**. One ESP32 lives on a sensor-equipped
glove; the other drives the arm. Total build cost: **~£152**.

<!-- TODO: replace with a GIF of the arm in action — this is the single highest-impact
     thing you can add to this README -->
![Demo](docs/media/demo.gif)

---

## What it does

| Gesture | Arm motion |
|---|---|
| Tilt hand left / right | X-axis rotation (base) |
| Tilt hand up / down | Y-axis elevation |
| Bend middle finger | Gripper open / close |
| Press thumb limit switch | Arm extension |

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
The arm is [FTobler's 3D Printed Belt Driven Robot Arm](https://github.com/ftobler/robotArm) —
approx. 20 cm tall, PLA, no part larger than 150 mm, so it prints on compact printers.
See `hardware/` for the printed enclosure (Fusion 360) designed for this build.

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
│   └── case/                # 3D-printed enclosure for the arm's control board
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

Arm mechanical design by [FTobler](https://github.com/ftobler/robotArm). Built as an
MSc thesis project; the full write-up is in `docs/thesis.pdf`.

## License

<!-- TODO: pick one. MIT is the usual default for firmware. Check FTobler's license
     before redistributing any of the mechanical files. -->
