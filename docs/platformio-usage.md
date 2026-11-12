# PlatformIO usage

How to build, upload, test and debug the sand table firmware with PlatformIO.

Commands are run from the project root (the folder containing `platformio.ini`). In VS Code, the same actions are in the PlatformIO sidebar under **Project Tasks → <environment>**, and commands can be typed into a **PlatformIO Core CLI** terminal.

## Environments

Each `[env:...]` section in `platformio.ini` is a separate build. Pick one with `-e <name>`. Without `-e`, `default_envs` (`esp32-a4988`) is used.

| Environment | Builds for | Motor driver |
|---|---|---|
| `esp32-a4988` | ESP32 (default) | A4988, bipolar steppers |
| `esp32-uln2803` | ESP32 | ULN2803, unipolar steppers |
| `native` | Your computer | none, unit tests only using `arduino_mock` |

## Building and uploading firmware

```sh
pio run                              # build the default env
pio run -e esp32-uln2803             # build a specific env
pio run -t upload                    # build and flash over USB
pio run -t upload -t monitor         # flash, then open the serial monitor
pio run -t clean                     # remove build output, forces a full rebuild
```

If more than one serial device is connected, list them and pick the port:

```sh
pio device list
pio run -t upload --upload-port /dev/cu.usbserial-0001
```

Upload troubleshooting:

- **Port busy / access denied:** close any open serial monitor first. The monitor and the uploader can't share the port.
- **Stuck on `Connecting...`:** hold the **BOOT** button on the ESP32 until the upload starts.

## Adding patterns (.thr files)

Patterns are stored on the ESP32's LittleFS flash partition, separate from the firmware.

### File format

Sisyphus style theta-rho, one point per line:

```
# comments start with #
0.000 0.950
0.052 0.941
```

- **theta** is in radians and unwrapped, so it keeps increasing past 2π for multiple rotations.
- **rho** runs from 0.0 (centre) to 1.0 (edge). It is scaled to the table's `AXIS_MIN_R`..`AXIS_MAX_R`, so the same file works on both motor builds.
- Values can be separated by spaces or tabs, and Windows (`\r\n`) line endings are fine.
- Malformed lines are skipped and logged.

Tools that export `.thr`:

- [Sandify](https://sandify.org) lets you design a pattern and export it as **Theta Rho (.thr)**.
- Pattern libraries shared by the Sisyphus and Dune Weaver communities.

### Limits

- **Points:** up to 5000 points per file (`PatternFile::DEFAULT_MAX_POINTS`). Larger files are rejected rather than drawn partially, because the whole pattern is held in memory (8 bytes per point). If a file is rejected, simplify or reduce the points in the tool that made it.
- **Storage:** the default partition table gives the filesystem about 1.4 MB, enough for many patterns.
- **File names:** keep them short, lowercase, with no spaces.

### Uploading

1. Put the files in `data/patterns/`. Everything under `data/` is copied to the root of the filesystem, so `data/patterns/my_pattern.thr` becomes `/patterns/my_pattern.thr` on the ESP32.
2. Upload the filesystem image:

   ```sh
   pio run -t uploadfs                   # default env
   pio run -e esp32-uln2803 -t uploadfs  # specific env
   ```

   In VS Code: **Project Tasks → Platform → Upload Filesystem Image**.

3. Choose the pattern to play by setting `PATTERN_FILE_PATH` in `src/main.cpp`, then upload the firmware (`pio run -t upload`).

Things to know:

- **Firmware and filesystem upload separately.** `upload` doesn't touch your patterns, and `uploadfs` doesn't touch the firmware.
- **`uploadfs` replaces the whole filesystem.** Any file not in `data/` is removed from the ESP32, so keep every pattern you want on the board in `data/`.
- **`pio run -t erase`** wipes all flash, firmware and patterns included.
- **Mount failure at startup:** the log shows `LittleFS mount failed` until a filesystem image has been uploaded at least once. The table still plays the built-in patterns.

## Serial monitor (debugging)

The firmware logs with `printf` at 115200 baud (`monitor_speed` in `platformio.ini`). The log includes referencing progress, each planned move, and pattern file results.

```sh
pio device monitor                                # open the monitor (uses monitor_speed)
pio device monitor -b 115200 -p /dev/cu.usbserial-0001
pio run -t upload -t monitor                      # flash and watch from boot
```

- **Exit** with `Ctrl+C`.
- **Restart the board** by pressing **EN** (reset) to watch it from boot. Output starts again from `setup()`.
- **Messages to look for:**
  - `Pattern file /patterns/x.thr: loaded N points`: the file loaded.
  - `... skipped N malformed line(s), first at line L`: some lines in the file were bad.
  - `... rejected, more than 5000 points`: the file is over the limit.
  - `... could not be opened`: wrong path, or the file isn't uploaded.
- **Crash decoding:** if the board crashes (`Guru Meditation Error` with a backtrace of hex addresses), add this to `[esp32_base]` to turn the backtrace into file and line numbers:

  ```ini
  monitor_filters = esp32_exception_decoder
  ```

- **No output:** check the baud rate matches, and that the monitor is connected to the right port (`pio device list`). If output is still missing, add `Serial.begin(115200);` at the start of `setup()`.

## Unit tests

Tests live in `test/`, one folder per suite:

```
test/
├── native/      # runs on your computer with arduino_mock
│   ├── test_motor_control/
│   └── test_pattern/
└── embedded/    # runs on the ESP32
    └── test_filesystem/
```

`test_filter` in `platformio.ini` keeps the native env to `native/*` and the ESP32 envs to `embedded/*`.

```sh
pio test -e native                             # all native suites
pio test -e native -f native/test_pattern      # one suite
pio test -e native -v                          # verbose, shows printf output
pio test -e esp32-a4988                        # on-device suites, board connected
```

Notes:

- **The ESP32 tests are flashed in place of the firmware.** Run `pio run -t upload` afterwards to put the sand table firmware back.
- **Filesystem test:** `test_filesystem` needs a LittleFS partition that mounts, so run `pio run -t uploadfs` once first. It writes and removes its own temporary file. The `example_rose.thr` check is skipped if that file isn't on the board.
- **Adding a suite:** create a new `test_<name>/` folder under `native/` or `embedded/` with a `.cpp` file that has its own `main()` (native) or `setup()`/`loop()` (embedded), `setUp()` and `tearDown()`.
