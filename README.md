
# A GameBoyColor Emulator
This is a GameboyColor emulator written in my favorite language `C`. I uses [SDL2](https://github.com/libsdl-org/SDL)+[Dear ImGui](https://github.com/ocornut/imgui) for 
sounds, graphics and input. So it can be runned on any platform that supports SDL2. And the GUI part is totally separated from the core emulator, so you can easily 
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
Currently only `MBC1` is implemented. (Which is enough for... *Tetris*!)

# Build
## macOS
1. Install SDL2 and SDL2_image via Homebrew
```bash
brew install sdl2
```
2. Clone the project
```bash
git clone 
git submodule update --init
```
3. Build the project
You may need to edit the `HOMEBREW_PATH` in `CMakeLists.txt` to point to your Homebrew installation path.
```bash
cmake .
make
```
4. Run the project
```bash
./xgbc -r cartridge [-b boot_rom]
```
Boot rom is optional.

## Linux
I do not have a Linux machine on hand, but it should be similar to macOS.

## Windows
*TO BE WRITTEN*   
*I uses MSYS2 to build the project on my Windows PC. But it's never easy for me to setup the environment on Windows.*

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

