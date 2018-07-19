# mbed-os


## How to use mbed-os

### Requirements
- [GCC ARM compiler](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
- [srecord](http://srecord.sourceforge.net/download.html)

### File Tree
- `mbed-os` must be one directory above your program files
- `mbed_config.h` must be in the **same** directory as `mbed-os` repo
- `Makefile` must be in the inside your project folder directory
- `Project` folder must be in the same directory as `mbed-os`

#### Example Tree
- Modkit
  - mbed-os
  - project-one
    - main.cpp
    - Makefile
  - project-two
      - main.cpp
      - Makefile
  - mbed_config.h

### Building and Compiling

To build and compile your program, `cd` into your working directory where your program files and `Makefile` should be.

Calling `make` from this directory will build and compile your program. It will create another `mbed-os` directory with all the mbed object files. It will also create a `BUILD` directory to store your hex and bin files.

The hex file that is generated to flash to your board is called `nrf5...-flash.hex`.

An example for a successful nrf51 build should look like this:
```
$ make
.
.
.
Compile: main.cpp
link: nrf51.elf
'arm-none-eabi-objcopy' -O binary nrf51.elf nrf51.bin
'arm-none-eabi-objcopy' -O ihex nrf51.elf nrf51.hex
NOTE: the srec_cat binary is required to be present in your PATH. Please see http://srecord.sourceforge.net/ for more information.
srec_cat ../.././mbed-os/targets/TARGET_NORDIC/TARGET_MCU_NRF51822/Lib/s110_nrf51822_8_0_0/s110_nrf51822_8.0.0_softdevice.hex  -intel nrf51.hex -intel -o nrf51-flash.hex -intel --line-length=44
===== hex file ready to flash: BUILD/nrf51-flash.hex =====
```


### EXTRAS
Refer back to [How to use mbed-os](#how-to-use-mbed-os) to figure out where these files are placed.

`mbed_config_nrf51.h` contains configuration data for the NRF51 board

`mbed_config_nrf52.h` contains configuration data for the NRF52 board with OS 5 features included

`mbed_config.h` contains configuration data for both the NRF51 and NRF52 boards

`Makefile_nrf52` contains the rules to build the mbed-os library and your program files with the NRF52 board

`Makefile_nrf51` contains the rules to build the mbed-os library and your program files with the NRF51 board

`nrf52` contains a blinky sketch for the `nrf52840_dk` board with its corresponding `Makefile`

`nrf51` contains a blinky sketch for the `nrf51822` board with its corresponding `Makefile`

**NOTE:**
- If you want to use the either the `mbed_config_nrf51.h` or `mbed_config_nrf52.h`, you must change the file name to be `mbed_config.h`.
- The `Makefile` for NRF51 and NRF52 are different, so make sure you use the right one. Check inside the `Makefile`for `PROJECT := ...` to see which `Makefile` you are using. Make sure you also change the `Makefile_...` you are using to `Makefile`.
