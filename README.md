# mbed-os

## Table of Contents
1. [Requirements](#requirements)
2. [How to Build and Compile for Mbed-OS](#how-to-build-and-compile-for-mbed-os)
3. [How to Build and Compile Mbed 2 from Mbed-OS](#how-to-build-and-compile-Mbed-2-from-Mbed-os)   
    2. [File Tree](#file-tree)
    3. [Building and Compiling](#building-and-compiling)
    4. [Extras](#extras)
4. [How to Reproduce for Mbed 2](#how-to-reproduce)
    1. [Getting mbed-os Repo](#getting-mbed-os-repo)
    2. [Getting Makefile for NRF52840_DK](#getting-makefile-for-nrf52840_dk)
    3. [Getting Makefile for NRF51_MICROBIT](#getting-makefile-for-nrf51_microbit)
    4. [Target Makefile to mbed-os at One Directory Back](#target-makefile-to-mbed-os-at-one-directory-back)
    5. [Merging Makfiles](#merging-makefiles)
    6. [Changing Makefile](#changing-makfile)


# Requirements
- [GCC ARM compiler](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)
- [srecord](http://srecord.sourceforge.net/download.html)

# How to Build and Compile Mbed 2 from Mbed-OS

### File Tree
- `mbed-os` must be one directory above your program files
- `mbed_config.h` must be in the **same** directory as `mbed-os` repo
- `Makefile` must be in the inside your project folder directory
- `Project` folder must be in the same directory as `mbed-os`

#### Example Tree

```
Modkit
│   README.md   
│   mbed_config.h
│
└───mbed-os
│
└───project-one
│   │   main.cpp
│   │   Makefile
│   │
└───project-two
│   │   main.cpp
│   │   Makefile
```

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
Refer back to [How to Build and Compile Mbed 2 from Mbed-os 5](#how-to-use-build-and-compile-Mbed-2-from-Mbed-os-5) to figure out where these files are placed.

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



# How to Reproduce

### Getting mbed-os Repo
Clone mbed-os in your working directory: `git clone https://github.com/ARMmbed/mbed-os`

### Getting Makefile for NRF52840_DK
1. Import mbed_blinky sketch with mbed-dev library
```
mbed import https://os.mbed.com/teams/mbed/code/mbed_blinky/
cd mbed_blinky
mbed remove mbed
mbed add https://os.mbed.com/users/mbed_official/code/mbed-dev/
```
2. Show hidden files in mbed_blinky (cmd + shift + .) and delete the `tools` folder from `./temp`

3. Go back to your working directory and import an OS 5 project: `mbed import mbed-os-example-blinky`

4. Copy the `tools` folder from `mbed-os-example-blinky/mbed-os` and paste it into `mbed_blinky/.temp`

5. Inside `mbed_blinky./temp/tools/targets/__init__.py` change the `target.json` default location to
```
__targets_json_location_default = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))), 'mbed-dev', 'targets', 'targets.json')
```
6. Inside `mbed_blinky/.temp/tools/targets/target.json` file, remove `"features": ["CRYPTOCELL310"]` from `MCU_NRF52840`

7. Export `mbed_blinky` to generate a `Makefile` for the NRF52840_DK
```
cd mbed_blinky
mbed export -i gcc_arm -m nrf52840_dk
```
Calling `make` here should build and compile and blinky sketch successfully

### Getting Makefile for NRF51_MICROBIT

**NOTE:** To do this without reimporting a blinky sketch, in `mbed_blinky` change the current `Makefile` to `Makefile_nrf52` and the current `mbed_config.h` to `mbed_config_nrf52.h`

1. Export `mbed_blinky` to generate a `Makefile` for the NRF51_MICROBIT
```
cd mbed_blinky
mbed export -i gcc_arm -m nrf51_microbit
```
2. Inside `mbed_blinky/Makefile` remove the S110 soft device line under `$(SREC_CAT)`
```
.././mbed-os/targets/TARGET_NORDIC/TARGET_MCU_NRF51822/Lib/s110_nrf51822_1_0_0/s110_nrf51_1.0.0_softdevice.hex
```
Calling `make` here should build and compile and blinky sketch successfully

### Target Makefile to mbed-os at One Directory Back

**NOTE:** To make organization easier to follow, rename your current `Makefile` and `mbed_config.h` files for the NRF51_MICROBIT to `Makefile_nrf51` and `mbed_config_nrf51.h` respectively.

1. In both `Makefile` add a `../` to all paths to point it one directory back and change `mbed-dev` path to `mbed-os`

  For example:
  ```
  ./mbed-dev/...    -> .././mbed-os
  .././mbed-dev/... -> ../.././mbed-os/
  ../               -> ../../
  .././             -> ../.././
  .././.            -> ../.././.
  ```

  The quickest way to do this is to:

  **Find:** `./mbed-dev`   
  **Replace With:** `.././mbed-os`

  Then add `../` to the remaining paths with just dots and slashes in `INCLUDE_PATHS` and `ASM_FLAGS`

2. Put the `mbed_config_nrf51.h` and `mbed_config_nrf52.h` files one directory back, that is, directly outside `mbed_blinky`

  **Example Tree:**
```
Modkit
│   README.md   
│   mbed_config_nrf51.h
│   mbed_config_nrf52.h
│
└───mbed-os
│
└───mbed_blinky
│   │   main.cpp
│   │   Makefile_nrf51
│   │   Makefile_nrf52
│   │
└───mbed-os-example-blinky
```

3. Inside `mbed-os/targets/TARGET_NORDIC/TARGET_NRF5x` move the files `lp_ticker.c` and `us_ticker.c` to the `TARGET_NRF52` folder

**NOTE:** At this point, you should have all the files you need to build and compile your programs for either board. However, when you are actually using the files, you must change your filenames to `Makefile` and `mbed_config.h` for whichever board you are using.

### Merging Makefiles

To merge the NRF51_MICROBIT and NRF52840_DK `Makefile`, export the `mbed_blinky` sketch from the *Mbed Online Compiler* for the NRF52840_DK board with GCC_ARM. A `mbed_config.h` file should be automatically generated from this export. You can merge this config file with the `mbed_config_nrf51.h` file to create one config file so that you do not have to rename your `mbed_config.h` file each time you change boards.

### Changing Makefile
Inside the Makefile, you are able to change the project name with `PROJECT :=` and the filename of the hex file you can use to flash to your board. The default filename to flash is called `$(PROJECT)-combined.hex`.

```
PROJECT := nrf52
.
.
.
$(PROJECT)-flash.hex
```
