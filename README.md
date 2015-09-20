Few people have been asking for sources of my Arduino music player so I created this Google Code project. The player is able to play MOD/S3M/XM/IT files on Arduino stored in MCU program memory. Here's an example video: http://www.youtube.com/watch?v=XZfw7l-ZxqE

I started this project to be able to play 4 channel Amiga MOD files on Arduino Uno via 8-bit mono resistor DAC. Later I expanded it to support S3M, XM and IT formats as well for more audio channels and ability to play more music that's available. I also added Teensy 3.0 support and structured the code so that it should be fairly easy to add support for other Arduino platforms & DAC types as well (maybe even non-Arduino platforms).

Good place to look for tunes is The Mod Archive: http://modarchive.org, and particularly the chiptune section as they are more likely to fit into the MCU memory. The player doesn't support all effects/features of the formats currently, so some files may not be played correctly. For example volume track effects used by XM & IT formats are not current supported.

The pattern data is compressed to better fit in the limited program memory of Uno (32kb for code & data), which complicates the code a bit but results the data to be compressed usually down to 20-40% of the original. Also the code is written to operate in very limited amount of RAM (~2kb). The sample data isn't compressed, but unused data outside sample loop range is cut off which may reduce the size a bit. The document about the file format & compression scheme is included in the project.

This project actually contains sources for two projects: The player that gets compiled for Arduino and file converter for PC that converts MOD/S3M/IT/XM formats to unified PMF (Profoundic Music File) format. The converter supports only Windows and depends on Spin-X Platform (http://sourceforge.net/projects/spinxengine), so you would need to download and compile that project first. The project contains compiled Windows exe though.

Note that PMF-format is still fairly volatile and there is no guarantee of PMF backwards compatibility in the player, i.e. older PMF files will not likely work with newer version of the player. So if you convert bunch of MOD/S3M/XM/IT files to PMF, it's better if you can run them through PMF converter easily in case you sync to updated version of the project (e.g. convert a directory with a script calling the converter). The format is designed so that it can be loaded as chunk of data to memory and played in-place (data is properly aligned).

If you find bugs or make improvements (fix bugs, add new features/platforms, etc.) don't hesitate to drop me a line! This is just a hobby project of mine so I work on it pretty irregularly, but I'm happy to get updates and share the work with other Arduino music enthusiasts.

Note that if you are getting the following error while compiling the project: "Can't find a register in class 'POINTER_REGS' while reloading 'asm'", it's because you have an old version of avr-gcc (i.e. the AVR C++ compiler) and need to upgrade it. The latest AVR toolchain which contains the latest gcc for AVR can be downloaded from Atmel website:

Windows: http://www.atmel.com/tools/ATMELAVRTOOLCHAINFORWINDOWS.aspx

Linux: http://www.atmel.com/tools/ATMELAVRTOOLCHAINFORLINUX.aspx

Upgrading the compiler isn't as trivial as I would like and you have to do some manual file copying, but here are some instructions: http://forum.arduino.cc/index.php?PHPSESSID=0e4q4ve6fbd1um0csohadverf2&topic=37965.msg281176#msg281176. While the instructions are for WinAVR, I recall it's pretty much the same for the AVR toolchain.

Cheers, Jarkko
