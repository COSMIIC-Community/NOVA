# NOVA firmware

Application firmware for the COSMIIC Nova module (UM-64NNP-V1, STM32U5A5). It sits above a 32 kB bootloader: the image is linked at `0x08008000` and must not be programmed at `0x08000000`.

## Hardware (Mac)

- **ST-LINK-V3SET** over SWD. On a MacBook, plug it through a **powered USB dongle or hub**. Direct laptop ports drop the probe, enumerate badly, or brown out during flash.
- Board power is **VIN = 5 V**. The debugger can back-feed the MCU. For current measurements, **unplug ST-Link** after flashing.
- Cortex-Debug is the flasher. Do not use the generic C/C++ “`a.out`” launch (F5 on `main.c` will offer that). In Run and Debug pick **Debug (ST-Link GDB server)**.

## Tools

Install [STM32CubeCLT 1.22.0](https://www.st.com/en/development-tools/stm32cubeclt.html) at `/opt/ST/STM32CubeCLT_1.22.0/`. Cursor/VS Code extensions: **Cortex-Debug**, C/C++, STM32 for VSCode.

Put CubeCLT on your PATH (or rely on `.vscode/tasks.json`, which already does):

```bash
export PATH="/opt/ST/STM32CubeCLT_1.22.0/CMake/bin:/opt/ST/STM32CubeCLT_1.22.0/Ninja/bin:/opt/ST/STM32CubeCLT_1.22.0/GNU-tools-for-STM32/bin:$PATH"
```

Use this CMake/Ninja/GCC, not a random Homebrew ARM toolchain. CubeIDE makefiles in `Debug/` still point at another machine; ignore them.

## Where the repo lives

Do **not** keep a working copy in iCloud `Documents`. File-provider stalls `gcc`/`ld` so the preLaunchTask hangs and you can get a 0-byte ELF (`not an ELF file`). Clone to something like `~/src`.

Build output is already redirected off the source tree:

`~/Library/Caches/implantables-nova/Debug/Implantables_Nova_App_main.elf`

## Build

First time (or after changing `CMakePresets.json`):

```bash
cmake --preset Debug
cmake --build --preset Debug
```

Later builds are just `cmake --build --preset Debug` (Cmd-Shift-B / the **CMake Build (Debug)** task). Link should take a fraction of a second. If it sits there, stop the task and kill leftover `ninja` / `arm-none-eabi-gcc` / `ld`.

Sampling mode is compile-time in `Core/App/module/sampling_config.h`: uncomment **exactly one** of `SAMPLING_MODE_ALL_64_2KHZ` or `SAMPLING_MODE_SINGLE_30KHZ`, then rebuild. SMART **Number of Channels** must match (`64` or `1`).

## Flash

1. Powered dongle → ST-LINK-V3SET → SWD on the module. VIN 5 V on the board.
2. Run and Debug → **Debug (ST-Link GDB server)** → start. That builds, then `load` at `0x08008000`.
3. ST’s GDB has no Python; launch.json passes `-nx` so a user `.gdbinit` does not abort the session.

CLI equivalent, after a successful build:

```bash
/opt/ST/STM32CubeCLT_1.22.0/STM32CubeProgrammer/bin/STM32_Programmer_CLI \
  -c port=SWD freq=8000 \
  -w "$HOME/Library/Caches/implantables-nova/Debug/Implantables_Nova_App_main.elf" \
  -rst
```

If flash succeeds then the session dies, retry once: a previous GDB server often still holds the probe.

## Windows

CubeCLT’s default install is `C:\ST\STM32CubeCLT_1.22.0\`. Install it, then the **ST-Link USB driver** it ships (Device Manager should show ST-LINK, not an unknown device). A powered hub is usually unnecessary on Windows; VIN is still **5 V**, and you still **unplug ST-Link** before current measurements.

`CMakePresets.json` writes to a macOS cache path. On Windows, configure a local build directory instead:

```bat
set PATH=C:\ST\STM32CubeCLT_1.22.0\CMake\bin;C:\ST\STM32CubeCLT_1.22.0\Ninja\bin;C:\ST\STM32CubeCLT_1.22.0\GNU-tools-for-STM32\bin;%PATH%

cmake -S . -B build\Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake
cmake --build build\Debug
```

ELF: `build\Debug\Implantables_Nova_App_main.elf`.

**STM32CubeProgrammer GUI:** close any other ST-Link session → Connect (SWD, under reset, 8000 kHz) → Open file → that ELF → Download. The ELF’s load address is already `0x08008000`.

**CLI:**

```bat
"C:\ST\STM32CubeCLT_1.22.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" ^
  -c port=SWD freq=8000 ^
  -w build\Debug\Implantables_Nova_App_main.elf ^
  -rst
```

**Cursor / VS Code:** same extensions. Point `.vscode/launch.json` and `tasks.json` at the Windows CubeCLT binaries (`ST-LINK_gdbserver.exe`, `arm-none-eabi-gdb.exe`, `STM32CubeProgrammer\bin`) and set `executable` / `program` to `build\Debug\Implantables_Nova_App_main.elf`. Still pick **Debug (ST-Link GDB server)**, not C++ `a.out`. Only one of CubeProgrammer, ST-LINK_gdbserver, or CubeIDE can own the probe at a time.

## After flash

`main()` writes `PWR_VOSR_USBPWREN | PWR_VOSR_VDD11USBDIS` after `SystemClock_Config()`. PA11/PA12 are FDCAN but also USB HS DP/DM; driving them high back-powers the USB PHY at ~13 mA. Analog/GPIO/reset do not disconnect it. Holding **NRST** still shows that current because this register write never runs.

Unplug ST-Link before reading VIN current.