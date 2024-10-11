# A GameBoyColor Emulator
This is a GameBoyColor emulator written in my favorite language `C`. I use [SDL2](https://github.com/libsdl-org/SDL)+[Dear ImGui](https://github.com/ocornut/imgui) for
sounds, graphics and input. So it can run on any platform that supports SDL2. And the GUI part is totally separated from the core emulator, so you can easily
replace it with your own GUI. Check the `gui/gui.h`.

<div style="text-align: center;">
  <img src="https://github.com/user-attachments/assets/b1b2ed18-1986-4619-83d4-64be2433993b" alt="screenshot" width="500"/>
</div>

# [Blargg's test](https://github.com/retrio/gb-test-roms)
<img width="200" alt="Screenshot 2024-10-02 at 09 08 17" src="https://github.com/user-attachments/assets/f7c28897-8fc1-4c48-891f-bc9b9ea366eb">
<img width="200" alt="Screenshot 2024-10-02 at 09 39 40" src="https://github.com/user-attachments/assets/79b92658-bf6e-40ee-8cf3-5009f3f5a331">

| Test | Status |
|----------|----------|
| cgb_sound      | ✅   |
| cpu_instrs     | ✅     |
| instr_timing   | ✅     |
| interrupt_time | ✅   |
| mem_timing     | ❌     |
| mem_timing-2   | ❌     |
| oam_bug        | ❌     |
| halt_bug.gb    | ❌     |


I'll gradually fix the failed tests.

# MBC

| Type | Status | Games I tested |
|----------|----------|----------|
| MBC1      | ✅   | Tetris DX |
| MBC5     | ✅     | Super Mario Bros. Deluxe, The Legend of Zelda: Oracle of Ages |


I'll add more MBCs in the future.

# Build
## macOS
1. Install SDL2 and SDL2_image via Homebrew
```bash
brew install sdl2
```
2. Clone the project
```bash
git clone https://github.com/kstardust/KameBoyColor.git
cd KameBoyColor
git submodule update --init
```
3. Build the project
You may need to edit the `HOMEBREW_PATH` in `CMakeLists.txt` to point to your Homebrew installation path.
```bash
cmake .
make
```

## Linux
I do not have a Linux machine on hand, but it should be similar to macOS.

## Windows
You need MSYS2 to build as I used some POSIX functions.
1. Install [MSYS2](https://www.msys2.org/)
2. Clone the project
```bash
git clone https://github.com/kstardust/KameBoyColor.git
cd KameBoyColor
git submodule update --init
```
3. Open `MSYS2 shell` and install the necessary packages
```bash
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-toolchain mingw-w64-x86_64-SDL2
```
4. Set the `MSYS2_PATH` in `CMakeLists.txt` to your MSYS2 installation path
```cmake
set(MSYS2_PATH "C:\\MyPrograms\\msys2")
```
5. Build the project (`This step is supposed to be done in MSYS2 shell. DO NOT USE Git Bash shell!!`)
```bash
cmake -G 'Unix Makefiles'
make
```

# Controls
Its in the `gui/main_sdl2.cpp` file. You can change it to whatever you like.

| Keyboard | Gameboy |
|-----|--------|
| A   | A      |
| B   | B      |
| Enter | Start |
| S   | Select |
| ↑   | Up     |
| ↓   | Down   |
| ←   | Left   |
| →   | Right  |

