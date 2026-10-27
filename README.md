| <center><img src="./assets/rakstar.jpg" alt="RAKstar" width=25%></center>  | ![RAKWireless](./assets/RAK-Whirls.png) | [![Build Status](https://github.com/RAKWireless/RAK13800_W5100S/workflows/RAK%20Library%20Build%20CI/badge.svg)](https://github.com/RAKWireless/RAK13800_W5100S/actions) |
| -- | -- | -- |

<img src="assets/rakstar.jpg" style="zoom:50%;" /><img src="assets/RAK-Whirls.png" alt="RAK-Whirls" style="zoom:150%;" />

# RAKwireless Audio library



## Introduction

RAKwireless Audio library is designed for RAK audio module and kit solutions. This library provides customer with abundant audio  examples and solutions. User can use them for: 

- mono/stereo audio mode
- playing music
- voice recording
- voice recognition
- noise collector
- PDM and analog microphones
- DSPG module for audio data process



## Elements

<img src="assets/stack (2).png" alt="stack (2)" style="zoom:80%;" />

As shown above, audio solutions includes the following parts:

- RAK18003: Interposer Board, as a signal switch for wiscore, microphone, amplifier 
- RAK18030: 20-20kHz Microphone  
- RAK18031:100-10kHz Microphone       
- RAK18032: Ultrasonic Microphone
- RAK18033: Stereo Mic Header Board                   
- RAK18040: Analog Mic to I2S
- RAK18060: 5.6W Stereo Amplifier
- RAK18061: 5.6W Amplifier              
- RAK18080: DSPG board, audio process board

There are also some audio kits available:

- WisBlock Microphone Interposer Kit                         
- WisBlock Speaker Amplifier SD Kit
- WisBlock Remote Microphone Kit
- WisBlock Stereo Microphone Kit
- Amplifier Microphone & SD Kit          
- WisBlock Voice Processing Kit
- WisBlock Voice Processing Speaker Kit                
- WisBlock Audio Accessories               

All elements can be used separately or together. However, there are two points need to note:

- They all use a WisBlock IO connector
- When using multiple WisBlock Audio modules, be sure the microphone placed on the top of stack.



## Installation

#### BSP Update

Please update BSP to the last version, which will support basic driver.

![](assets/BSP.jpg)

#### Rakwireless Audio library

Then install audio library, which contains other part of driver, like amplifier, DSPG. And includes all examples and solutions. Because of the technical problems, it is not merged in the IDE now. Please download ZIP from:  https://github.com/RAKWireless/RAKwireless-Audio-library  And then install manually.

![](assets/install.jpg)

But if you use Arduino 2.0 released version, you can install it in library manager.

![](assets/2.0.jpg)


## Examples & Solutions

The examples can be separated 4 parts. The test code is just for test and can be ingnored.



- #### Audio Graph

  Audio graph will show the audio wave with rawdata from microphone, including analog or digital. We have supported several kinds of graph, here we will show the difference by two examples 

  - Original Data Graph

    Here we use RAK4631+RAK18030 to show the digital wave, and same as the analog.

    ![](assets/graph.jpg)

  - FFT Graph

    The "Fast Fourier Transform" (FFT) is an important measurement method in the science of audio and acoustics measurement. It converts a signal  into individual spectral components and thereby provides frequency  information about the signal. FFTs are used for fault analysis, quality  control, and condition monitoring of machines or systems. This article  explains how an FFT works, the relevant parameters and their effects on  the measurement result.

    Strictly speaking, the FFT is an optimized algorithm for the  implementation of the "Discrete Fourier Transformation" (DFT). A signal  is sampled over a period of time and divided into its frequency  components. These components are single sinusoidal oscillations at  distinct frequencies each with their own amplitude and phase. This  transformation is illustrated in the following diagram. Over the time  period measured, the signal contains 3 distinct dominant frequencies. 

  

  ![](assets/FFT-Time-Frequency-View-540.png)

  ​		Below is what we get with RAK4631+RAK18032, and of course you can use other wiscore and microphone.

  ![](assets/FFT_square_trend.png)

- #### Aduio Alarm

  We combine the wireless and audio alarm. When the audio rawdata value reach to a threshold, it will report a alarm by LoRaWAN, WIFI. Or you can just use the local alarm. You can modify the threshold in code.

  - With LoRa

    This example use RAK4631+RAK18003+RAK18030, and based on LoRaWAN OTAA. After join the network, the PDM will listen and report a alarm when triggered.

    ​                          ![](assets/Loraalarm1.jpg)  

    ![Loraalarm2](assets/Loraalarm2.jpg)

  - With WIFI

    This example use RAK11200+RAK18003+RAK18030, and based on WIFI. After connection, the PDM will listen and report a alarm to remote server with TCP when triggered.

    ![](assets/wifi1.png)

    ![wifi2](assets/wifi2.jpg)

  - Local

    When use in local, it is just a alarm with a buzzer.

- #### Voice Recognition

  The Cyberon Software provides the ability of RAKwireless customers to enable voice control within their WisBlock System. The Cyberon software allows for the detection of specified voice triggers or commands. These commands are stored in a file which the customer links in when building their code.

  Cyberon provides a set of tools to create these models. When the software is provided to customers RAK will provide one (or more) “standard” models to be used by makers and for evaluation of commercial customers. Currently the Cyberon Software can run on two different “platforms” within the WisBlock Audio:

  1. The WisCore (RAK11200 (ESP), RAK11310 (RP2040), RAK4631 (nRF52840))
  2. The RAK18080 (DSPg DBM10L).

  

  We provide three standard mode now as below, first and second is for local device control and third is for LoRaWAN node.

  | Function       | Trigger         | Commands                                                     |
  | -------------- | --------------- | ------------------------------------------------------------ |
  | Music          | “Hey RAK Star”  | “Play Music”, “Stop Music”, “Previous Song”, “Next song”, “Volume Up”, “Volume Down”, “Stop”, “Pause”, “Next”, “Resume”, “Previous”, “Volume Up”, “Volume Down”, “Increase Volume”, “Decrease Volume” |
  | Light Contrrol | “Hey RAK Star”  | "Turn Lights on”, “Turn Lights off”, “Lights on”, “Lights off”, “Turn lights up”, “Turn lights down”, “Dim lights”, “Brighten lights”, “Lights Red”, “Lights Blue”, “Lights Green”, “Lights Yellow”, “Lights Orange”, “Lights Pink” |
  | WisDM          | “Hey RAK Cloud“ | “Connect”, “Disconnect”, “Report Location”, “Get Status”, “Report Battery“,“Report Environment”, “Reset” |

  

  - WisCore
    - WisDM
    
      This examples use RAK4631+RAK18003+RAK18030. When say "Connect", it will connect to the lora server, and of course, you need to make some configuration, like DEVEUI, APPEUI... If you want to use TTN, same as well.
    
      ![WisDM](assets/WisDM.jpg)
    
    - RGB
    
      This example use RAK19001+RAK4631+RAK18003+RAK14001+RAK18030. When say "Turn Lights on", the RGB will on. And you can test other commands, like  “Lights Pink”, it will be pink.
    
      ![](assets/RGB.jpg)
    
  - DSPG

    - Music
    
       This example use RAK18030/1/2+RAK18003+RAK18060+RAK18080+RAK11200.When say the trigger words wakes up the WisCore then say command words it will print the recognized command and ID number by  serial port.
       
       ![](assets/DSPG_play.jpg)
    
      

- #### Play & Record

  Because of the limitation of hardware, we only support record/play WAV file via SD card on RAK11200.   
  
  - Record to SD
  
  - note：This example need use the battery power for WisBase.
  
    This example use RAK11200+RAK18003+RAK18030+SD card. And you can modify the parameter, like frequence, channel or date bit.
  
    ![](assets/record.png)
  
    Then you will find the record file in the SD card.
  
    ![](assets/record2.png)
  
  - Play from SD
  
  This example use RAK11200+RAK18003+RAK18060+Speaker+SD card. And first store the WAV file in SD card and modify the name to "test".
  
  ![](assets/play.png)
  
  Then download the example and play. It can detect the different format of WAV in the SD card and play "test.wav".
  
  ![](assets/play2.png)
  
  ![play3](assets/play3.png)

#### How to generate a header from WAV

First you need put your WAV file with the WAVconvert.py  in the same folder.

![](assets/convert1.png)

Second, run the CMD console program.

![](assets/convert2.png)

Third, enter the "**python ./WAVconvert.py**" command and run .If your computer not installed the Python environment you can download the python by this link：https://www.python.org/downloads/.

![](assets/convert3.png)

After convert done, the converted .h file name is the same as your  WAV name.

![](assets/convert4.png)

Then, open the .h file and copy the sound buffer to replace the example's sound buffer.

![](assets/convert5.png)

---

## Upgrading from 1.0.x — behaviour changes

Most sketches keep working, but some behaviour changed in ways that
won't show up as a compile error. Check these if a sketch that worked
on 1.0.x misbehaves after upgrading.

### All boards (TAS2560 amplifier, RAK18060 / RAK18061)

- **`set_pcm_channel()` now takes effect.** In 1.0.x it wrote the wrong
  register, so every amp played the mix of both channels, (L+R)/2,
  whatever mode was set. Now `LeftMode` plays only the left slot and
  `RightMode` only the right. If your I²S stream carries audio in one
  slot only (for example an I²S driver set to one channel), the amp
  set to the other slot goes **silent**. Fix: send the audio in both
  slots, or call `set_pcm_channel(LeftRightMode)` to get the 1.0.x
  behaviour back. The RAK4631 and RAK11200 examples that play a mono
  stream now use `LeftRightMode` on both amps (stereo files still get
  one channel per amp), so they behave as in 1.0.x.
- **Alarm recovery keeps the channel.** `clear_alarm()` /
  `clear_interrupt()` reload the register table and now re-apply the
  last `set_pcm_channel()` mode, gain and mute state.
- **ASI word length is set to 16 bits** after every `begin()`. If you
  send 24- or 32-bit samples, call `set_pcm_word_length()` after
  `begin()`.
- **`set_mute()` also sets the gain to 0**, and `set_unmute()`
  restores the gain and the power mode from before the mute. While
  muted, `set_gain()` only records the new gain (the amp stays silent)
  and `get_gain()` reports it; `set_unmute()` applies it.
- **Use `begin(addr, sample_rate)`.** The old `begin(addr)` still
  works and loads the same table as 1.0.x, but only the rate-aware
  version is set up correctly for 8 kHz and 96 kHz.

### RAK3112 (ESP32-S3)

- **`PDM.read()` doesn't wait.** In 1.0.x it blocked until a full
  block (1920 bytes) was ready and could write more than the size you
  passed. Now it returns what's ready, up to `size`, or 0 if nothing
  is. Always use the return value as the byte count; a loop that
  assumes a full buffer each call writes stale data.
- **DMA buffers can be sized.** The default is the same as 1.0.x
  (ESP_I2S's 6 × 240 frames, 90 ms at any rate). Call
  `PDM.setBufferSize()` / `I2S.setBufferSize()` before `begin()` to
  change it, e.g. for slow SD card loops.
- **`PDM` and `I2S` are separate peripherals.** In 1.0.x `PDM` was a
  wrapper around the global `I2S`, so reading the mic through
  `I2S.readBytes()`, or stopping it with `I2S.end()`, worked. Now the
  mic is only reachable through `PDM`, and `I2S.end()` doesn't stop it.
- **The global `I2S` is an `I2SEventClass`, not ESP_I2S's
  `I2SClass`.** It supports standard (Philips) mode only and has
  `setPins`, `begin`, `write`, `readBytes`, `available`, `end`,
  `setBufferSize` and the callbacks. ESP_I2S extras are gone:
  `playWAV`, `recordWAV`, `playMP3`, `configureTX/RX`, `setPinsPdmRx`,
  PDM / TDM modes in `I2S.begin()`, and the `Stream` functions (`read()`,
  `peek()`, `print()`). These give compile errors, not silent failures.
- **Fixed ports.** `PDM` uses `I2S_NUM_0` and `I2S` uses `I2S_NUM_1`. A
  sketch that also creates its own ESP_I2S `I2SClass` won't get a port
  while both are running.
- **Below 16 kHz the I²S output uses 32-bit stereo slots** (BCLK
  512 kHz at 8 kHz), because the TAS2560 PLL needs at least 512 kHz.
  Mono writes are duplicated into both slots for you. This only matters
  if you connect a different I²S device.
- **`PDM.setGain()` is new on RAK3112 and isn't the RAK4631 scale.**
  1.0.x had no mic gain on RAK3112. The new `setGain(0..80)` is a
  ×1–×15 multiplier, while on RAK4631 the same call is −20 to +20 dB
  (40 = 0 dB). A value copied from a RAK4631 sketch gives a different
  level; see the `setGain` entry below.
- **`PDM.begin()` returns 0 on failure.** 1.0.x always returned 1, so
  a sketch that checks the result may now stop where it used to run on
  with no mic data.
- **The `Stereo` macro is gone** from the RAK3112 branch of `audio.h`.
  Use `I2S_SLOT_MODE_STEREO` or `2`.

---

## RAK3112 (ESP32-S3) — extended API and fixes

Version 2.0.0 ships a rewritten RAK3112 audio path against raw ESP-IDF
`i2s_common` / `i2s_std` / `i2s_pdm`, so runtime sizing, event callbacks,
and dual-port full duplex all work on a stock Arduino-ESP32 v3.x install.
The rest of this section documents what's new on the RAK3112 board target
and how to use it. The other boards share only the TAS2560 amplifier
fixes (see "Upgrading from 1.0.x" above).

### Per-rate TAS2560 amplifier support

`TAS2560::begin(addr, sample_rate)` picks a rate-specific register table
via `tas2560_dsp_coeffs_for_rate(fs)` and programs PCM_RATE, the PLL and
a 16-bit word length to match. Validated on RAK18060 hardware with
stereo WAVs (correct speed, correct left/right, no hiss):

| Sample rate | Register table | PCM_RATE | PLL J (PLL_CLK) |
|---|---|---|---|
| 8 kHz | PPC3 (ROM Mode 1, 8 Ω, Class-G, 8.6 V / 3 A) | 0x30 | 96 (49.152 MHz) |
| 16 kHz | stock `registers[]` | 0x32 (48 kHz mode) | 96 (49.152 MHz) |
| 44.1 kHz | stock `registers[]` | 0x32 | 32 (45.158 MHz) |
| 48 kHz | stock `registers[]` | 0x32 | 32 (49.152 MHz) |
| 96 kHz | `registers_96khz[]` ("Mode 2 96k", no DSP patch) | 0x33 | 32 (98.304 MHz) |

22 050 Hz (the `PlayBack` / `PlayBack22K` example rate) uses the
stock table; `PlayBack22K` plays correctly in stereo.

Below 16 kHz the library widens the ESP-IDF STD slot to 32 bits (two slots
per frame, WS width 32), so BCLK at 8 kHz is 512 kHz instead of 256 kHz.
Mono callers see no API change: `I2SEventClass::write()` interleaves each
mono sample into both slots transparently.

At 96 kHz the chip ignores ASI channel select, so
`set_pcm_channel(RightMode)` inverts WCLK on that amp instead. Sketches
call it the same way at every rate.

**Mute before stopping I²S.** Call `set_mute()` on the amps before
`I2S.end()`. Ending I²S with the amps powered leaves the class-D idling
without clock or data, which is audible as hiss.

Sketch pattern (any supported rate):

```cpp
TAS2560 AMP_Left, AMP_Right;
AMP_Left.begin(0x4C, /*sample_rate*/ 8000);   // picks per-rate table
AMP_Left.set_pcm_channel(LeftMode);
AMP_Left.set_gain(5);
I2S.setPins(WB_IO6, WB_IO5, SDA1, SCL1);
I2S.begin(I2S_MODE_STD, 8000, I2S_DATA_BIT_WIDTH_16BIT,
          (i2s_slot_mode_t)1);
```

### Dual-port full duplex — PDM + STD I²S concurrent

`PDMClass` and the library-wide `I2SEventClass I2S` now own separate
`i2s_chan_handle_t`s on fixed ports: PDM_RX on `I2S_NUM_0` (the only
port that supports it on ESP32-S3) and STD I/O on `I2S_NUM_1`, so mic
capture and speaker playback run concurrently:

```cpp
PDM.begin(1, 16000);   // I2S_NUM_0
I2S.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, ...);  // I2S_NUM_1
// both live at the same time
```

Because the ports are fixed, the two can start in either order.
Anything else that
claims `I2S_NUM_0` or `I2S_NUM_1` (such as the core's `ESP_I2S`
`I2SClass`) makes the matching `begin()` fail.

In 1.0.x both shared one peripheral, so the second `begin()` would
silently tear down the first. See
`examples/RAK3112/PDMSerialPlotter_RAK18003` with `FULL_DUPLEX_TEST 1`
for a working reproducer.

### Event callbacks (Arduino nRF52 parity)

Both classes expose `onReceive()` / `onTransmit()` matching the
Arduino nRF52 PDM/I²S API convention. The ISR posts to a FreeRTOS
semaphore; a dispatch task (core 0, priority 5) invokes the user
callback in task context. Arduino-safe APIs including `PDM.read()`
work from inside the callback.

```cpp
void mic_ready() {
  uint8_t buf[512];
  int n = PDM.read(buf, sizeof(buf));
  // ...
}
PDM.begin(1, 16000);
PDM.onReceive(mic_ready);   // called once per DMA descriptor
```

Bench-verified for `PDM.onReceive()` at 16 kHz mono 16-bit: 2000
events / 30 s (matches DMA math), zero drops, dispatch latency 11 µs
mean / 13 µs max — well below the 15 ms DMA period. The `I2S`
callbacks use the same code path but haven't been bench-tested.
Counters exposed for diagnostics: `dmaEvents()`, `droppedEvents()`,
`userCallbacks()`, `lastIsrMicros()`.

### Runtime buffer sizing, gain, `available()`, safe reads

- **`bool setBufferSize(int bytes)`** (both classes) — must be called
  before `begin()`. Programs the real IDF DMA sizing
  (`i2s_chan_config_t` `dma_desc_num=4`, `dma_frame_num` derived).
  Without it the default is ESP_I2S's 6 × 240 frames (90 ms), as in
  1.0.x. Returns `false` post-`begin()` and leaves the previous request
  unchanged. `bufferSize()` reports the effective bytes (0 before
  `begin()` means the default).
- **`void PDM.setGain(int gain)`** — Arduino-style 0..80, mapped to a
  1..15 software multiplier applied per-sample in `read()` with int16
  saturation. Software rather than hardware because ESP32-S3 doesn't
  define `SOC_I2S_SUPPORTS_PDM_RX_HP_FILTER`, so the IDF's
  `amplify_num` field literally isn't in `i2s_pdm_rx_slot_config_t` on
  this SoC. Default ×1. The RAK18030's raw level is low, so recordings
  usually need `setGain(80)` (×15); `RecordToSD` uses that.

  **Same name, different scale on RAK4631.** There the value goes to
  the nRF52 PDM hardware in 0.5 dB steps: 0 = −20 dB, 40 = 0 dB,
  80 = +20 dB. On RAK3112 it is a multiplier, so the same number gives
  a very different level:

  | `setGain()` | RAK4631 (nRF52) | RAK3112 |
  |---|---|---|
  | 20 | −10 dB | ×4 (+12 dB) |
  | 30 | −5 dB | ×6 (+15.6 dB) |
  | 40 | 0 dB | ×8 (+18 dB) |
  | 80 | +20 dB (×10) | ×15 (+23.5 dB) |

  When porting a RAK4631 sketch, re-tune the value on the RAK3112
  rather than copying it.
- **`int PDM.available()`** — returns actual staged bytes (not a
  compile-time constant, unlike ESP-IDF's own `available()`). Backed
  by a private 3840-byte staging buffer that non-blocking pulls fill
  on demand.
- **`PDM.read(buf, size)`** honors `size` — the 1.0.x version wrote
  a full DMA block (~1920 B) regardless of the caller's buffer
  length. Excess data stays in the IDF ring for the next call, so
  small reads preserve sample order.
- **`PDM.read()` doesn't wait.** It returns the bytes that are ready
  (up to `size`), or 0 if none are; the 1.0.x version blocked until
  a full block arrived. Always use the return value as the byte count.
  For long recordings (e.g. to SD), raise `PDM.setBufferSize()` so the
  DMA ring covers the slowest write.

### Amp mute discipline

`TAS2560::set_mute()` powers the class-D down (`PWR_CTRL_1 = 0x03`)
*and* zeroes the SPK_CTRL digital gain, caching the prior gain and
boost mode. `set_unmute()` restores both. Repeated `set_mute()` calls
are safe. Always mute before `I2S.end()` (see above).

### Example sketches (`examples/RAK3112/`)

The stock examples compile unchanged: a compile sweep of all four cores found no failures
that weren't already present in 1.0.9. They haven't all been
run on hardware.

| Sketch | Verifies |
|---|---|
| `AudioSmokeTest_RAK18003/` | Play tone → record mic → print min/max/RMS. Configurable SAMPLE_RATE / AMP_VARIANT. |
| `PDMSerialPlotter_RAK18003/` | Original PDM plotter, plus opt-in `FULL_DUPLEX_TEST` for concurrent speaker |

### Known limitations

- PDM capture is limited by the microphone, not the driver: the
  RAK18032 ultrasonic mic is rated to about 75 kHz, so use 72 kHz at
  most (verified); 96 kHz returns bad data.
- `I2S.available()` returns a buffer-size hint, not the number of bytes
  ready.
- The `I2S.onReceive()` / `onTransmit()` callbacks haven't been
  bench-tested.
- Tested with the RAK18060 stereo amp only; the RAK18061 mono amp is
  untested.
- `RAK4631/AnalogMicRecordToSD` was ported from the ESP32 version and
  `*/VoiceRecognition_DSPG` updated to the current `DSPG::begin()`;
  both compile but haven't been run on hardware.

### Build note

`library.properties` keeps `precompiled=true` / `ldflags=-lDSpotterSDK`:
the CDSpotter voice examples on RAK4631, RAK11200 and RAK11310 need the
bundled `libDSpotterSDK.a`. There is no ESP32-S3 archive, and with
arduino-cli 1.5 (and current Arduino IDE 2.x) the RAK3112 sources compile
from source with the flag set. If library edits ever appear not to take
effect, check the build log for the RAK3112 `.cpp` files and clear the
sketch build cache.



















































