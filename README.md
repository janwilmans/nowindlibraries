# nowindlibraries
Support libraries for MSX Nowind Interface and developer emulation environment

# Status
Here's an overview of the current status of the project and some known issues. Finally, some future plans (that may or may never be carried out) are listed. This page is also a kind of TODO list during development.

# What works

- disk images of different sizes (including hard disk images in DOS2)
- BASIC device now0..3 enables accessing files on the host in MSX BASIC
- ROM drive (currently 360 kB)
- write protect (read-only attribute is respected)
- tested on MSX, MSX2, Turbo-R
- commandline host application works on Windows, Linux 32bit and 64bit kernels and MacOS Leopard
# Known issues

- none at the moment

# Usage / Documentation

- Write a proper manual, any volunteers?
- Create a proper GUI usbhost application for multiple platforms
- Eliminate need for root-privileges non-windows platforms
- Create a tool to insert rom-disk-images without recompiling the firmware (or at least without the need of knowlegde of compilers)

# Future development

- virtual drive (access the host file system, without using disk images) this could also increase speed (seek times) a LOT
- real drive access; use a PC drive as a normal msx diskdrive (_format will require special attention here) Note: this might already be possible on mac and linux by hosting /dev/fd0 (untested)
- extending ROM drive size
- RS232 support (MSX can connect to RS232 devices on the host)
- Symbos driver
- support random I/O for (now:) devices
- use compression for ROM drive
- support compressed images (.zip)
- map virtual drive to ftp site
- support disk images located on an internet site
