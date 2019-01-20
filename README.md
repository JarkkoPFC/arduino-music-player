Few people have been asking for the source code of my Arduino music player, so I created this GitHub project. The player is able to play MOD/S3M/XM/IT music files that are stored in the MCU program memory and has been developed so that it can run within very limited memory and performance constraints while still producing decent sound. I originally developed the player for Arduino Uno, which has only 2KB of RAM, 32KB of flash memory and 8-bit MCU running at 16MHz. Below is a video showing the player in action (running on Teensy 3.6)

[![Arduino Music Player on Teensy 3.6](https://img.youtube.com/vi/FbUc1X3T-MU/0.jpg)](https://youtu.be/FbUc1X3T-MU)

## Basic Installation Instructions
Once you have downloaded the project, open **pmf_player.ino** in Arduino IDE and compile the project for your target platform (if you have compilation issues, check the "Issues" section). For Teensy you can just connect DAC0 & Analog Ground (AGND) pins to an amplifier line-in or headphones to listen to the music. For other Arduino devices you'll need to build an 8-bit resistor DAC (e.g. [resistor ladder](https://en.wikipedia.org/wiki/Resistor_ladder)) connected to data pins 0-7. Or if the device has built-in [DAC](https://en.wikipedia.org/wiki/Digital-to-analog_converter), you can also copy & modify **pmf_player_teensy.cpp** for that device to support the DAC.

The player comes with an existing music file (**aryx.s3m** by Karsten Koch) that should fit any Arduino device with at least 32Kb of flash memory. You can see Aryx playing below on Arduino Uno so you know what to expect from the birth cry of your player.

[![Arduino Uno Playing 12Chl S3M @ 37KHz](https://img.youtube.com/vi/b_QbBE_fXZs/0.jpg)](https://youtu.be/b_QbBE_fXZs)

## Custom Music Files
Good place to look for music is [The Mod Archive](https://modarchive.org), and particularly the [chiptune section](https://modarchive.org/index.php?query=54&request=search&search_type=genre) as they are more likely to fit into the MCU flash memory. Arduino MCU's with larger flash memory can of course fit in larger music files, e.g. Teensy 3.6 has 1MB of flash which is enough for most mod music files. To play the music files on Windows PC, you can use [OpenMPT](https://openmpt.org).

In order to use custom music files you'll need to use PMF Converter that's a Windows command line executable in **pmf_converter/bin/pmf_converter.exe**. This converter will convert MOD/S3M/XM/IT files to PMF files that you can embed to the player program. PMF format is specifically designed for small memory devices by compressing the music data, and for small music files in particular you can often see significant reduction in the file size compared to MOD/S3M/XM/IT files.

The PMF music data is stored in **music.h** file of the project, which you can create simply by running the following console command on your Windows PC:
```
pmf_converter -hex -o ../../pmf_player/music.h -i <mod/s3m/xm/it file>
```
After the PMF conversion you just need to compile the sketch again and upload the program to the MCU.

## Making Electronic Instruments
The player supports controlling individual audio channels from code to enable creation of electronic instruments. You can override the data for note, instrument/sample, volume and audio effect programmatically for each row & channel as the music advances. **pmf_player.ino** has a simple example which adds an extra audio channel for the music playback and adds a drum hit programmatically every 8th row (see *row_callback_test()* function and commented-out setup in *setup()* function).

For instruments without any pre-recorded music playing on the background, you can create a MOD/S3M/IT/XM with one empty pattern long playlist and the instruments you like to use, for example using [OpenMPT](https://openmpt.org). However, when converting the file to PMF, **pmf_converter** strips out all unreferenced instruments and eliminates empty channels. To avoid this use command-line argument "*-dro*" for the converter (v0.42+):
```
pmf_converter -hex -dro -o ../../pmf_player/music.h -i <mod/s3m/xm/it file>
```
This will keep all the instrument data intact and available for programmatic playback while not playing any sounds by itself.

## Issues
- If you compile the project for a device with very limited RAM (like 2KB on Arduino Uno) the sketch compilation may fail because of insufficient RAM. You can easily reduce the RAM usage by reducing the number of supported audio channels (32 by default) to something like 16. The number of supported channels is defined in **pmf_player.h** file with *pmfplayer_max_channels* value. The number of channels the player needs to have at minimum depends on the music file, which is shown in "Channels" in the beginning of **music.h** (e.g. 12 for aryx.s3m). If you define less channels than is required by the music file, the player will just ignore the extra channels.

- Another potential problem is that when you try to compile the project you'll get "undefined reference" errors for functions *start_playback()*, *stop_playback()*, *mix_buffer()*, *get_mixer_buffer()* and *visualize_pattern_frame()*. This is because these functions are not defined for the device you are compiling the project for. In file **pmf_player_arduino.cpp** you can try to comment out the two lines with text "*AVR_ATmega328P*" to see if it works. The code in this file is MCU specific and may or may not work, and I have tested it only on Arduino Uno. If it doesn't work, you have to unfortunately program these mixing functions yourself, which can be a bit of a challenge. Fortunately the mixing functions in the Teensy implementation (**pmf_player_teensy.cpp** file) are mostly device agnostic C++, so you may borrow most of the code from those functions.

- The music playback may not sound the same as it does with OpenMPT and there are many potentially reasons for it. One potential reason is that the effects used in the music file are not supported by the player, or are not implemented to the spec (which are very vague). There are also potential bugs in the player/converter that can cause playback issues. Also check for potential warnings upon the music file conversion.

## Closing Words
If you find bugs or make other improvements to the player don't hesitate to drop me a line! This is just a hobby project of mine so I work on it pretty irregularly, but I'm happy to get updates and share the work with other Arduino music enthusiasts.


Cheers, Jarkko
