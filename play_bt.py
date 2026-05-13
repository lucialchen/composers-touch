import asyncio
import subprocess
import sys
import os
import threading
from bleak import BleakScanner, BleakClient

DEVICE_NAME = "InstrumentPanel"
CHARACTERISTIC_UUID = "abcd1234-ab12-ab12-ab12-abcdef123456"
SOUNDS_DIR = './sounds'
RECORDING_PATH = '/tmp/voice_loop.mp3'
RECORD_SECONDS = 3

looping_processes = {}
is_recording = False
current_mode = 0


def play_once(inst):
    path = f"{SOUNDS_DIR}/{inst}.mp3"
    if os.path.exists(path):
        subprocess.Popen(["afplay", path])
    else:
        print(f"Missing sound file: {path}")


def start_loop(inst, path=None):
    if path is None:
        path = f"{SOUNDS_DIR}/{inst}.mp3"
    if os.path.exists(path) and inst not in looping_processes:
        p = subprocess.Popen(
            f'while true; do afplay "{path}"; done',
            shell=True
        )
        looping_processes[inst] = p
        print(f"  Started loop: {inst}")


def stop_loop(inst):
    if inst in looping_processes:
        looping_processes[inst].kill()
        del looping_processes[inst]
        print(f"Stopped loop: {inst}")


def stop_all_loops():
    for inst in list(looping_processes.keys()):
        stop_loop(inst)


def _record():
    global is_recording, current_mode
    if os.path.exists(RECORDING_PATH):
        os.remove(RECORDING_PATH)
    try:
        result = subprocess.run(
            ["rec", "-r", "44100", "-c", "1", RECORDING_PATH,
             "trim", "0", str(RECORD_SECONDS)],
            timeout=RECORD_SECONDS + 5, capture_output=True,
        )
        if result.returncode != 0:
            print(f"  ERROR: {result.stderr.decode().strip()}")
            is_recording = False
            return
    except FileNotFoundError:
        print("  ERROR: 'rec' not found, install sox: brew install sox")
        is_recording = False
        return
    print("  Recorded. Switching to loop mode.")
    is_recording = False
    current_mode = 1
    start_loop("__voice__", path=RECORDING_PATH)


def enter_record_mode():
    global is_recording
    if is_recording:
        return
    is_recording = True
    stop_all_loops()
    print(f"\n  RECORD MODE: recording for {RECORD_SECONDS} seconds...")
    threading.Thread(target=_record, daemon=True).start()


def handle_notification(sender, data):
    global current_mode
    line = data.decode('utf-8', errors='ignore').strip()
    if not line:
        return
    print(f"Received: {line}")

    if line.startswith("MODE:"):
        value = line.split(":")[1]
        if not value:
            return
        if value == "RECORD":
            current_mode = 2
            enter_record_mode()
        else:
            current_mode = int(value)
            print(f"Switched to Mode {current_mode}")
            stop_all_loops()
        return

    parts = line.split(",")
    if len(parts) == 2:
        inst = parts[1]
        if current_mode == 0:
            play_once(inst)
        elif current_mode == 1:
            if inst in looping_processes:
                stop_loop(inst)
            else:
                start_loop(inst)
        elif current_mode == 2:
            play_once(inst)


async def main():
    print(f"Scanning for '{DEVICE_NAME}'...")
    device = await BleakScanner.find_device_by_name(DEVICE_NAME, timeout=15)
    if not device:
        print("Device not found.")
        sys.exit(1)

    print(f"Connecting to {device.address}...")
    async with BleakClient(device) as client:
        print("Connected. Listening for touch events...")
        print("Mode 0 = single hit | Mode 1 = loop | Long-press mode sensor = record")
        await client.start_notify(CHARACTERISTIC_UUID, handle_notification)
        try:
            while True:
                await asyncio.sleep(0.1)
        except KeyboardInterrupt:
            print("\nQuitting...")
            stop_all_loops()


asyncio.run(main())