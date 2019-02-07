Few people have been asking for the source code of my Arduino music player, so I created this GitHub project. The player is able to play MOD/S3M/XM/IT music files that are stored in the MCU program memory and has been developed so that it can run within very limited memory and performance constraints while still producing decent sound. I originally developed the player for Arduino Uno, which has only 2KB of RAM, 32KB of flash memory and 8-bit MCU running at 16MHz. Below is a video showing the player in action (running on Teensy 3.6)

[![Arduino Music Player on Teensy 3.6](https://img.youtube.com/vi/FbUc1X3T-MU/0.jpg)](https://youtu.be/FbUc1X3T-MU)

## Basic Installation Instructions
Once you have downloaded the project, open **pmf_player.ino** in Arduino IDE and compile the project for your target platform (if you have compilation issues, check the "Issues" section). For Teensy and MKR you can just connect DAC0 & Ground (GND) pins to an amplifier line-in to listen to the music. For Arduino AVR you'll need to build an 8-bit resistor DAC (e.g. [resistor ladder](https://en.wikipedia.org/wiki/Resistor_ladder)) connected to data pins 0-7. For other platforms you'll need to port the player to the platform (see "Porting to a New Platform" section) and output to the associated [DAC](https://en.wikipedia.org/wiki/Digital-to-analog_converter).

The player comes with an existing music file (**aryx.s3m** by Karsten Koch) that should fit any Arduino device with at least 32Kb of flash memory. You can see Aryx playing below on Arduino Uno so you know what to expect from the birth cry of your player.

[![Arduino Uno Playing 12Chl S3M @ 37KHz](https://img.youtube.com/vi/b_QbBE_fXZs/0.jpg)](https://youtu.be/b_QbBE_fXZs)

## Custom Music Files
Good place to look for music is [The Mod Archive](https://modarchive.org), and particularly the [chiptune section](https://modarchive.org/index.php?query=54&request=search&search_type=genre) as they are more likely to fit into the MCU flash memory. Arduino MCU's with larger flash memory can of course fit in larger music files, e.g. Teensy 3.6 has 1MB of flash which is enough for most mod music files. To play the music files on Windows PC, you can use [OpenMPT](https://openmpt.org).

In order to use custom music files you'll need to use PMF Converter that's a Windows command line executable in **pmf_converter/bin/pmf_converter.exe**. This converter will convert MOD/S3M/XM/IT files to PMF files that you can embed to the player program. PMF format is specifically designed for small memory devices by compressing the music data, and for small music files in particular you can often see significant reduction in the file size compared to MOD/S3M/XM/IT files. The format is designed so that it can be efficiently played back without any load-time processing (e.g. sample decompression) and just loaded as a data chunk to memory for playback. The data in the format is also properly aligned to avoid need for any unaligned memory accesses.

The PMF music data is stored in **music.h** file of the project, which you can create simply by running the following console command on your Windows PC:
```
pmf_converter -hex -o ../../pmf_player/music.h -i <mod/s3m/xm/it file>
```
After the PMF conversion you just need to compile the sketch again and upload the program to the MCU.

The converter can also output 32-bit DWORD hex codes instead of 8-bit BYTE hex codes by using *-hexd* switch instead of *-hex* switch. Use of DWORDs can substantially improve the project compilation times, particularly for large music files. When using DWORDs, you need to change *uint8_t* to *uint32_t* in the following line in **pmf_player.ino**:
```
static const uint8_t PROGMEM s_pmf_file[]=
```
Note that DWORD arrays aren't supported on Arduino platforms that doesn't natively support 32-bit types, which is why this isn't the default setting.

## Making Electronic Instruments
The player supports controlling individual audio channels from code to enable creation of electronic instruments. You can override the data for note, instrument/sample, volume and audio effect programmatically for each row & channel as the music advances. **pmf_player.ino** has a simple example which adds an extra audio channel for the music playback and adds a drum hit programmatically every 8th row (see *row_callback_test()* function and commented-out setup in *setup()* function).

For instruments without any pre-recorded music playing on the background, you can create a MOD/S3M/IT/XM with one empty pattern long playlist and the instruments you like to use, for example using [OpenMPT](https://openmpt.org). However, when converting the file to PMF, **pmf_converter** strips out all unreferenced instruments and eliminates empty channels. To avoid this use command-line argument "*-dro*" for the converter (v0.42+):
```
pmf_converter -hex -dro -o ../../pmf_player/music.h -i <mod/s3m/xm/it file>
```
This will keep all the instrument data intact and available for programmatic playback while not playing any sounds by itself.

## Porting to a New Platform
If the Arduino platform you try to compile the project for isn't supported, you'll need to implement some of the functions for the platform. Most of the code is platform agnostic, but few of the pmf_player functions require special implementations, namely:
- get_sampling_freq(uint32_t sampling_freq_)
- start_playback(uint32_t sampling_freq_)
- stop_playback()
- mix_buffer(pmf_mixer_buffer &buf_, unsigned num_samples_)
- get_mixer_buffer()

*get_sampling_freq()* returns the closest supported sampling frequency matching the requested frequency. The MCU isn't likely able to reproduce exactly the requested frequency so this function is used to adjust the player to match the actual supported frequency. It's fine to return the requested frequency from the function since the pitch error should be pretty minor and other errors in playback probably hide this anyway (e.g. the used 8.8fp sample step).

*start_playback()* is probably the most challenging to implement since it needs to setup an interrupt to run at given frequency, and the interrupt function to feed data to the DAC. *pmf_audio_buffer* can be used for the master audio buffer implementation and to fetch audio data in the interrupt in given bit depth to be fed to the DAC. 

*stop_playback()* just need to stop the interrupt from running.

*mix_buffer()* mixes all active audio channel samples with given sampling rate and volume to the master buffer. There's a reference implementation *mix_buffer_impl()* that can be used for the initial implementation. However, for more optimal implementation, this function can be written in hand optimized platform specific assembly language (see the AVR implementation for example). This function is the most performance intensive part of the player and dominates how many audio channels you can mix at most and in what kind of frequency, or how much spare computing resources you have left for doing other things while the music is playing.

*get_mixer_buffer()* just returns the master audio buffer to the player for some processing.

## Issues
- If you compile the project for a device with very limited RAM (like 2KB on Arduino Uno) the sketch compilation may fail because of insufficient RAM. You can easily reduce the RAM usage by reducing the number of supported audio channels (32 by default) to something like 16. The number of supported channels is defined in **pmf_player.h** file with *pmfplayer_max_channels* value. The number of channels the player needs to have at minimum depends on the music file, which is shown in "Channels" in the beginning of **music.h** (e.g. 12 for aryx.s3m). If you define less channels than is required by the music file, the player will just ignore the extra channels.

- Another potential problem is that when you try to compile the project you'll get "undefined reference" errors for functions *get_sampling_freq()*, *start_playback()*, *stop_playback()*, *mix_buffer()* and *get_mixer_buffer()*. This is because these functions are not implemented for the device you are compiling the project for. Check out "Porting to a New Platform" for to address this issue. 

- The music playback may not sound the same as it does with OpenMPT and there are many potentially reasons for it. One potential reason is that the effects used in the music file are not supported by the player, or are not implemented to the spec (which are very vague). There are also potential bugs in the player/converter that can cause playback issues. Also check for potential warnings upon the music file conversion.

## Closing Words
If you find bugs or make other improvements to the player don't hesitate to drop me a line! This is just a hobby project of mine so I work on it pretty irregularly, but I'm happy to get updates and share the work with other Arduino music enthusiasts.


Cheers, Jarkko
