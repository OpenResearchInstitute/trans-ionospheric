# Trans-Ionospheric Radio Peripheral and Badge Firmware

Code for the Trans-Ionospheric electronic badge and
radio peripheral from Phase 4 Ground and [Open Research Institute](https://openresearch.institute/badge/).

This is based on the unofficial JoCo Cruise 2018 pirate monkey badge,
which was in turn based on the excellent work that the AND!XOR team
produced for the DEFCON 25 Bender Badge.

All code files are licensed under the Apache license 2.0 unless
superceded by notices in individual files or directories.

## Getting Started

Some resources must be installed outside of this source tree.
One of these is the Nordic SDK.

NOTE: If you're already set up for JoCo firmware builds, you don't need
to do any of this setup over again.

Choose a place to install these on your file system. For example,
we'll use "/src/badge-support/".

Download the version 14.2.0 of the Nordic SDK
[from here](https://developer.nordicsemi.com/nRF5_SDK/)
and unzip it into the directory you chose.

It's known that later versions of the SDK are not compatible with the
existing badge code. We may work on fixing that, which is why we don't
install the SDK in the source tree for the project.

Now edit your shell init or rc file to set the following environment
variable when you create a shell, substituting your chosen path:

```
export SDK_ROOT=/src/badge-support/nRF5_SDK_14.1.0_1dda907/
```

### Ubuntu Installation (non-ARM host)

Download the GNU ARM Embedded Toolchain. Choose one of these methods:

* Download the latest from <https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads>
and unpack it into its own directory in `/usr/local`. Or,

* Download the version that Nordic used to test their SDK, 4.9 2015q3,
from <https://launchpad.net/gcc-arm-embedded/+milestone/4.9-2015-q3-update>
and unpack it into its own directory in `/usr/local`. Or,

* `sudo apt-get install gcc-arm-none-eabi` and take your chances on
whatever version is current for your Linux. This may be your only
option if you're running Linux on a non-x86 platform.

Then edit `$SDK_ROOT/components/toolchain/gcc/Makefile.posix` and change
the value for `GNU_INSTALL_ROOT` to the directory where your toolchain
was installed.

If you plan to hook up hardware for programming and debugging, you'll
need the Segger JLink software, or equivalent. If you're running Ubuntu
on an Intel PC, you can just:

`sudo dpkg -i ubuntu-dependencies/JLink_Linux_V618b_x86_64.deb`

You'll also need a couple of command line tools. Install these in a
location outside this project, like the Nordic SDK. For example using
the location "/src/badge-support", you can do the following:

```
mkdir -p /src/badge-support/bin
cd /src/badge-support/bin
tar xvf <path-to-badge-project>/ubuntu-dependencies/nRF5x-Command-Line-Tools_9_7_0_Linux-x86_64.tar
```

Now edit your shell init or rc file to set the following environment
variable when you create a shell, substituting your chosen path:

```
export NRFJ_BIN=/src/badge-support/bin
```

#### Ubuntu on an ARM Host

If you want to develop on an ARM-based host, most of the components are available.

The Nordic SDK is the same.

You should be able to `sudo apt-get install gcc-arm-none-eabi` to get the toolchain.

You can get JLink software from <https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack>

The command line tools nrfjprog and mergehex are not currently available
for ARM hosts. You can use the JLink tools to program the device, so you
can probably work around nrfjprog. We still need to come up with an
alternative for mergehex, but that should be simple.

### Build the code

`cd firmware/src`

`make`

`cd ..`

`./update.sh`

### Flashing the target

Two ways, either:

`cd firmware`

`./provision.sh`

or

`cd firmware/src`

`make flash_softdevice`

`make flash`

The `make flash_softdevice` can be omitted after the first time.
