# ECGR 4101/5101 Project 2: Virtual Pet

A Tamagotchi-style virtual pet running on an **STM32 NUCLEO-L496ZG-P** with an **X-NUCLEO-GFX01M2** LCD.
Keep **William Aft Off** fed and happy: its hunger and happiness drain over time, it reacts to the joystick and to
being shaken, it has a mini game, and it remembers its state even after the board is unplugged.

**Project board (Trello):** [ECGR 4101/5101 Project 2: Virtual Pet](https://trello.com/b/t18jJGGF/ecgr-4101-5101-project-2-virtual-pet)

---

## Features

- **Original animated pet**: idle, eating, dancing, backflip, sleeping, sad, low-battery, dying and leaving animations
- **Hunger and happiness meters** (0–100) that drain over time
- **Low-stat alerts**: red **HUNGRY!** / **SAD!** blinks when a stat is at or below 25
- **Consequences**: hunger at 0 → the pet **dies**; happiness at 0 → the pet **runs away**
- **Actions**: feed, dance, shake-to-backflip, and the **Catch the Stars** mini game
- **Sound effects and music** through a speaker for menus, actions, death and runaway
- **Non-volatile save**: the pet's state is stored in flash and restored at power-up
- **Reset** from the menu to start over with a new pet

---

## Controls

| Input | Action |
|---|---|
| Joystick **center** | Open the menu / select the highlighted option |
| Joystick **up / down** | Move through the menu |
| **Shake** the board (accelerometer) | Pet does a **backflip** |
| Joystick **left / right** | Move the catcher in the Catch the Stars mini game |

**Menu options**

| Option | What it does |
|---|---|
| FEED | Hunger +20, eating animation and sound |
| DANCE | Happiness +10, hunger −5, dance animation with music notes |
| STARS | Starts the Catch the Stars mini game. Stats are frozen while playing, and both refill to 100 when it ends |
| RESET | Starts a fresh pet (80 / 80, alive) and saves it |
| CLOSE | Closes the menu |

---

## Pet rules

| Rule | Value |
|---|---|
| Stat range | 0–100, every change is clamped |
| Starting stats | 80 hunger / 80 happiness |
| Hunger drain | −1 every 3 s |
| Happiness drain | −1 every 4 s |
| Low-stat alert | ≤ 25 |
| Death | Hunger reaches 0 |
| Runaway | Happiness reaches 0 |
| Backflip (shake) | Happiness +10, hunger −5, at most once every 2 s |

Once the pet dies or runs away, its stats freeze and actions are disabled until **RESET**.

---

## Hardware

**Parts**
- NUCLEO-L496ZG-P development board
- X-NUCLEO-GFX01M2 display board (2.2" 240×320 ILI9341 LCD + 5-way joystick)
- MPU-9250 / GY-6500 accelerometer (I²C)
- 8 Ω, 3 W speaker with a PAM8302A mono amplifier
- 3.7 V Li-ion battery (amplifier power)
- Breadboard, jumper wires

**Connections**

| Device | Interface | Pins |
|---|---|---|
| LCD (ILI9341) | SPI1 | SCK PA5, MISO PA6, MOSI PA7, CS PA9, DC PB10, RESET PA1 |
| Joystick | GPIO (active low) | Left PB6, Right PB0, Up PC0, Down PB4, Center PC7 |
| MPU-9250 | I²C1 (address 0x68) | SDA PG13, SCL PG14, power enable PG12 |
| Speaker (via PAM8302A) | PWM (TIM2 CH1) | PA0 |


---

## Project structure

| File | Purpose |
|---|---|
| `Core/Src/main.c` | Startup, main loop, joystick input, menu actions, death / runaway polling, shake detection |
| `Core/Src/pet_stats.c/.h` | Hunger / happiness / state (`pet` struct), time-based drain, clamping, action effects, alerts, mini game freeze/refill |
| `Core/Src/pet_flash.c/.h` | Saves and loads the `pet` struct in the last 2 KB page of flash |
| `Core/Src/character.c/.h` | Pet actions (`feed`, `play`, `dance`) and which animation loop to show |
| `Core/Src/graphics_controller.c/.h` | Sprite animation playback, HUD background and status bars |
| `Core/Src/sprite.c/.h` | Pet sprite frames (RGB565 arrays) |
| `Core/Src/menu.c/.h` | Menu drawing and selection |
| `Core/Src/catch_stars.c/.h` | Catch the Stars mini game |
| `Core/Src/mpu9250.c/.h`, `i2c.c/.h` | MPU-9250 setup over I²C, accelerometer reads, shake detection |
| `Core/Src/sounds.c/.h` | Non-blocking PWM melody player and sound effects |
| `Drivers/BSP/GFX01M2/` | ILI9341 LCD driver and font |
| `project2_char_anims/` | Source sprite images (BMP) |
| `convert_bmp_hex_array`, `Utilities/` | Scripts that convert images into C arrays |
| `STM32L496xx_FLASH.ld` | Linker script; the last 2 KB of flash is reserved for the save page |

---

## How it works

**Main loop.** At power-up the board initializes the clock, GPIO, SPI (LCD), I²C (MPU-9250) and the sound timer,
then loads the saved pet from flash. Each pass of the `while (1)` loop:

1. Reads the joystick, updates the sound player and reads the accelerometer
2. Runs the mini game instead, if it is active
3. Handles menu input and actions
4. Updates hunger / happiness on real time (`HAL_GetTick()`)
5. Checks for death, runaway, or a shake (backflip)
6. Redraws the menu, or the HUD and the pet animation

**Responsiveness.** Nothing in the loop waits. Every timer (stat drain, animation frames, alert blinking, sound notes,
autosave) compares `HAL_GetTick()` to its last update and returns right away, so the joystick and animations never freeze.

**Non-volatile save.** The `pet` struct (hunger, happiness, state) is packed into one 8-byte record with a magic number
and inverted check copies, and written to the next empty slot of the last flash page (256 slots). It saves every
30 s only if something changed, and immediately on death, runaway, the end of the mini game, and reset. The page
is only erased when all 256 slots are full, about once every 2 hours of play, which keeps flash wear very low.

---

## Build and flash

**Requirements:** VS Code with the **STM32Cube for Visual Studio Code** extension and **STM32CubeCLT**.

1. Clone the repository and open the project folder (the one containing `CMakeLists.txt`) in VS Code. Trust the folder when asked.
2. Select the **Debug** preset when prompted.
3. Click **Build** in the status bar. The build should end with `exit code 0`.
4. Connect the board through the ST-LINK USB port.
5. **Run → Start Debugging** and choose **STM32Cube: STM32 Launch STLink GDB Server**.
6. When it pauses at `main()`, press **Continue**.

---

## Team and contributions

| Member | GitHub | Contributions |
|---|---|---|
| Jus Liu | JusPSLiu | Animations, animation controller state machine, repository setup and integration |
| Gregory Hagins II | creative-gregory | Design / mock-up, hardware implementation (MPU-9250 accelerometer, speaker and amplifier, sound effects) |
| Lauren Rouse | lrouse9-stack | Catch the Stars mini game, dance option |
| Marly Ashac | mashac136 | Pet stats module (hunger / happiness with time-based drain and clamping), low-stat alerts, death and runaway logic, action effects (feed, dance, mini game freeze / refill), saving the pet to flash memory |

---
