# Bio-Rhythm Interactive Light

![Platform](https://img.shields.io/badge/Platform-Arduino%20UNO-blue.svg)
![Sensors](https://img.shields.io/badge/Sensors-Microphone%20Array%20%7C%20Touch-orange.svg)
![Output](https://img.shields.io/badge/Output-High%20Power%20LED%20%28MOSFET%29-green.svg)
![License](https://img.shields.io/badge/License-CC%20BY--NC%204.0-lightgrey.svg)

## 📖 Introduction

**Bio-Rhythm Interactive Light** is an immersive installation piece designed to synchronize artificial lighting with the natural rhythm of human breathing. 

Unlike standard sound-reactive LEDs that simply flash to volume, this system uses a **5-microphone array** and a custom signal processing algorithm to detect the subtle airflow of a human breath. It translates this input into organic, non-linear brightness changes on a main light panel, creating a "living" light effect that mimics respiratory patterns.

This project was developed through **18 iterative refinements** to perfect the fluidity of the light interaction, serving as a prototype for therapeutic lighting and interactive art.

### ✨ Key Features
* **Airflow Detection Algorithm:** Uses a 5-sensor array to differentiate directed breathing from ambient noise using a dynamic baseline (`noiseRemove`).
* **Organic Fading:** Implements non-linear PWM control via MOSFET to create smooth, natural light transitions rather than robotic on/off states.
* **Multi-Stage Progression:** Features a State Machine (Stages 1-4) where sustained breathing drives the light to higher brightness levels.
* **Visual Feedback:** 3x WS2812B LED strips act as status indicators (White = Idle, Green = Active Stage).
* **Touch Interaction:** Capacitive touch sensor for system reset and manual calibration.

---

## 🛠️ Hardware & Bill of Materials (BOM)

Based on the system design and power requirements, the following components were used:

| Component | Type | Function |
| :--- | :--- | :--- |
| **Microcontroller** | Arduino UNO R3 | Central Processing Unit |
| **Sensors** | 5x Analog Sound Sensors (e.g., KY-038) | Microphone Array for airflow detection |
| **Input** | 1x Capacitive Touch Sensor (TTP223) | User interaction/Reset |
| **Main Light Driver** | 2x MOSFET Modules (IRF520/540) | PWM control for high-power LED panels |
| **Main Light** | 4x High-Power LED Matrix Panels | The primary breathing light source |
| **Indicators** | 3x WS2812B LED Strip (Single pixel) | Stage/Status indicators |
| **Power Step-Down** | 2x Buck Converters (LM2596) | Converts 24V -> 5V (for Arduino/Sensors) & 24V -> 12V (if needed) |
| **Power Supply** | 24V DC Power Supply | Main power source for LED panels |
| **Wiring** | Breadboard & Jumpers | Circuit assembly |

---

## 🔌 Circuit Diagram

The following Fritzing diagram illustrates the connections between the microphone array, power management modules, MOSFET drivers, and the Arduino.

<img width="13508" height="9643" alt="xi_bb2" src="https://github.com/user-attachments/assets/ec117aca-d77d-4921-b792-49b9482d023d" />


### Pin Configuration
* **Microphones (5x):** A0, A1, A2, A3, A4
* **Touch Sensor:** Digital Pin 4
* **MOSFET (Main Light):** Digital Pin 3 (PWM)
* **Indicator LEDs:** Pin 5, 6, 7

---

## 💻 Algorithmic Logic

The core challenge of this project was creating a "natural" feel. The firmware implements the following logic:

1.  **Noise Removal:** On startup, the system samples background noise for 1 second to establish a dynamic baseline (`signalMax`/`signalMin`).
2.  **Weighted Averaging:** Reads all 5 microphones, filters outliers, and calculates a weighted average `airflowValue`.
3.  **Thresholding:** Automatically determines the breath detection threshold based on signal intensity.
4.  **State Machine (FSM):**
    * **Idle:** Light is off or dim.
    * **Exhale Detected:** Light fades IN (PWM increases non-linearly).
    * **Hold:** If breath is sustained, light holds at stage peak (85 / 170 / 250 PWM).
    * **Release:** Light fades OUT smoothly if breath stops.

---

## ©️ Intellectual Property & License

**Copyright © 2025 Chen Junxu. All Rights Reserved.**

### ⚠️ Disclaimer (Prototype Use Only)
This repository contains **prototype firmware and schematics** developed for educational and demonstration purposes. 
* It **does NOT** represent the final mass-produced commercial product of any associated client. 
* Specific proprietary parameters and commercial industrial designs have been **excluded** to protect intellectual property.

### 📜 Usage Policy (CC BY-NC 4.0)
This project is licensed under the **Creative Commons Attribution-NonCommercial 4.0 International License**.

1.  **Non-Commercial Use:** You are free to use, modify, and study this code for **personal learning, academic research, or hobbyist projects**.
2.  **No Commercial Deployment:** You are **strictly prohibited** from using this code or design for commercial products, paid services, or industrial deployment without written permission.
3.  **Attribution:** If you use this project in your work or portfolio, you must credit the author (**Chen Junxu**) and link back to this repository.
