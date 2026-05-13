# A Composer's Touch

## Overview
This project transforms an unused decorative wall piece into an interactive musical composition tool. The device, which features multiple musical instruments, is modified with copper touch sensors so that each instrument can be played through physical interaction. In addition, the device allows the user to record a short 3-second snippet of sound to layer with the instruments. 

The system uses an ESP32 to detect touch input and communicates with a laptop-based program via wireless bluetooth connection to trigger corresponding sound effects. A secondary touch sensor functions as a mode switch, allowing users to toggle between discrete playback, looping composition, and recording modes.

## Materials and Components

### Hardware
- LILYGO TTGO T1 ESP32
- Copper tape
- Breadboard
- Solid core wire 
- Decorative object (used as interface)
- USB cable
- LiPo battery
- Soldering tools

### Software
- Arduino development environment - PlatformIO on VS Code recommended
- Bluetooth Low Energy (BLE) library - NimBLE-Arduino library recommended
- Computer for recording and audio playback


## System Architecture
1. Copper tape pads act as touch sensors connected to ESP32 touch pins through the breadboard.  
2. ESP32 detects touch events and sends signals via wireless Bluetooth communication.
3. A computer connected to the ESP32 via BLE receives the signals and maps them to audio playback.
4. Mode selection determines whether sounds are played once or looped, or if the computer should record sound to be looped.  

## Installation and Setup

### 1. Hardware Assembly
- Attach copper tape to each place where you want a sensor on the decorative object  
- Connect each copper sensor to a capacitive touch pin on the ESP32  
- Solder wires to the copper tape 
- Connect all components through a breadboard

![description](images/wire_setup.jpg)
*Detailed Wire Setup*  

### 2. Software Setup
- Create a new PlatformIO project for the TTGO T1 board
- Copy the provided `main.cpp` into the `/src` folder of the project  
- Upload the code to the ESP32 via USB
- Install dependencies:
`pip install bleak`
`brew install sox`
- Place `play_bt.py` on your computer and ensure it has access to the audio files used for playback  
- Run `play_bt.py` to start the program; Ctrl+C to quit


## Usage
1. Connect the LiPo battery to the ESP32
2. Start the `play_bt.py` program; wait for confirmation that the Bluetooth connection starts
3. Interact with the copper touch sensors:
   - Touch a copper sensor to trigger its sound
   - Short-press the mode switch to toggle between single-play and loop modes
   - Long-press the mode switch to record 3 seconds of sound to loop, then play with adding the other instruments!

![description](images/mode_switch.jpg)
*Mode Switch Sensor*

## Reproducibility Notes
- Touch sensitivity may vary depending on the size of the copper tape sensors; adding extra solder near spots where the sensors are less effective is recommended
- Required touch threshold may vary depending on power source; increase/decrease as necessary in `main.cpp`
- Commented-out touch debug code is included at the bottom of main.cpp — uncomment and upload it to read raw sensor values and calibrate TOUCH_THRESHOLD for your hardware.
