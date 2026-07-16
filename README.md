| <center><img src="./assets/rakstar.jpg" alt="RAKstar" width=25%></center>  | ![RAKWireless](./assets/RAK-Whirls.png) | [![Build Status](https://github.com/RAKWireless/RAK13800_W5100S/workflows/RAK%20Library%20Build%20CI/badge.svg)](https://github.com/RAKWireless/RAK13800_W5100S/actions) |
| -- | -- | -- |

<img src="assets/rakstar.jpg" style="zoom:50%;" /><img src="assets/RAK-Whirls.png" alt="RAK-Whirls" style="zoom:150%;" />

# RAKwireless Audio library



## Introduction

RAKwireless Audio library is designed for RAK audio module and kit solutions. This library provides customer with abundant examples and solutions on audio field. User can use them for any scene 

- mono/stereo audio mode
- play music
- record voice
- voice recognition
- noise collector
- PDM and analog microphone
- DSPG module for audio data process



## Elements

<img src="assets/stack (2).png" alt="stack (2)" style="zoom:80%;" />

As shown above, audio solutions includes below parts:

- RAK18003: Interposer Board, as a signal switch for wiscore, microphone, amplifier 
- RAK18030: 20-20kHz Microphone  
- RAK18031:100-10kHz Microphone       
- RAK18032: Ultrasonic Microphone
- RAK18033: Stereo Mic Header Board                   
- RAK18040: Analog Mic to I2S
- RAK18060: 5.6W Stereo Amplifier
- RAK18061: 5.6W Amplifier              
- RAK18080: DSPG board, audio process board

At the same time we will release some audio kit

- WisBlock Microphone Interposer Kit                         
- WisBlock Speaker Amplifier SD Kit
- WisBlock Remote Microphone Kit
- WisBlock Stereo Microphone Kit
- Amplifier Microphone & SD Kit          
- WisBlock Voice Processing Kit
- WisBlock Voice Processing Speaker Kit                
- WisBlock Audio Accessories               

All elements can be used separately or together. However, there are two points need to note:

- They all use IO connector
- We advise to use them together and be sure the microphone placed on the top of stack.



## Installation

#### BSP Update

Please update BSP part to the last version, which will support basic driver.

![](assets/BSP.jpg)

#### Rakwireless Audio library

Then install audio library, which contains other part of driver, like amplifier, DSPG. And includes all examples and solutions. Because of the technical problem, it is not merged in IDE now. Please download ZIP from:  https://github.com/RAKWireless/RAKwireless-Audio-library  And then install manually.

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

#### How to general header from WAV

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

## RAK3112 (ESP32-S3) — extended API and fixes

This fork ships a rewritten RAK3112 audio path against raw ESP-IDF
`i2s_common` / `i2s_std` / `i2s_pdm`, so runtime sizing, event callbacks,
and dual-port full duplex all work on a stock Arduino-ESP32 v3.x install.
The rest of this section documents what's new on the RAK3112 board target
and how to use it. RAK4631 / RAK11310 / RAK11200 code paths are
unchanged.

### Per-rate TAS2560 amplifier support

`TAS2560::begin(addr, sample_rate)` picks a rate-specific register table
via `tas2560_dsp_coeffs_for_rate(fs)` and reprograms the chip's PLL and
clock-error detection to match. Validated on RAK18060 hardware at:

| Sample rate | Status | Source of DSP table |
|---|---|---|
| 8 kHz  | clean | PPC3 (ROM Mode 1, 8 Ω, Class-H, 8.6 V / 3 A) |
| 16 kHz | clean | upstream `registers[]` |
| 22 050 Hz | clean | upstream `registers[]` |
| 48 kHz | clean | upstream `registers[]` |
| 96 kHz | rate-supported, hardware validation pending | upstream `registers[]` |

At 8 kHz the fork additionally widens the ESP-IDF STD slot to 32-bit
stereo so BCLK lands at 512 kHz — inside the TAS2560 PLL lock range.
Mono callers see no API change: `I2SEventClass::write()` interleaves each
mono sample into both slots transparently.

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
`i2s_chan_handle_t`s. The IDF assigns PDM_RX to `I2S_NUM_0` (the only
port that supports it on ESP32-S3) and STD I/O to `I2S_NUM_1`, so mic
capture and speaker playback run concurrently:

```cpp
PDM.begin(1, 16000);   // I2S_NUM_0
I2S.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, ...);  // I2S_NUM_1
// both live at the same time
```

Pre-fork behavior was to reconfigure a single shared peripheral, so the
second `begin()` would silently tear down the first. See
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

Bench-verified at 16 kHz mono 16-bit: 2000 events / 30 s (matches
DMA math), zero drops, dispatch latency 11 µs mean / 13 µs max — well
below the 15 ms DMA period. Counters exposed for diagnostics:
`dmaEvents()`, `droppedEvents()`, `userCallbacks()`, `lastIsrMicros()`.

### Runtime buffer sizing, gain, `available()`, safe reads

- **`bool setBufferSize(int bytes)`** (both classes) — must be called
  before `begin()`. Programs the real IDF DMA sizing
  (`i2s_chan_config_t` `dma_desc_num=4`, `dma_frame_num` derived).
  Returns `false` post-`begin()` and leaves the previous request
  unchanged. `bufferSize()` reports the effective bytes.
- **`void PDM.setGain(int gain)`** — Arduino-style 0..80, mapped to a
  1..15 software multiplier applied per-sample in `read()` with int16
  saturation. Software rather than hardware because ESP32-S3 doesn't
  define `SOC_I2S_SUPPORTS_PDM_RX_HP_FILTER`, so the IDF's
  `amplify_num` field literally isn't in `i2s_pdm_rx_slot_config_t` on
  this SoC.
- **`int PDM.available()`** — returns actual staged bytes (not a
  compile-time constant, unlike ESP-IDF's own `available()`). Backed
  by a private 3840-byte staging buffer that non-blocking pulls fill
  on demand.
- **`PDM.read(buf, size)`** honors `size` — the pre-fork version wrote
  a full DMA block (~1920 B) regardless of the caller's buffer
  length. Excess data stays in the IDF ring for the next call, so
  small reads preserve sample order.

### Amp mute discipline

`TAS2560::set_mute()` now zeroes both the class-D power
(`PWR_CTRL_1 = 0x03`) *and* the SPK_CTRL digital gain — caching the
prior gain so `set_unmute()` can restore it. Belt-and-braces coverage
across transient PLL states. Same `volume` cache pattern
`clear_alarm()` already used.

### Example sketches (`examples/RAK3112/`)

Verification sketches added for each phase (in addition to the stock
examples, all of which still work unchanged):

| Sketch | Verifies |
|---|---|
| `AudioSmokeTest/` | Play tone → record mic → print min/max/RMS. Cross-phase regression bar. Configurable SAMPLE_RATE / AMP_VARIANT. |
| `Phase3_PDMAvailableCheck/` | `available()` growth / drain, read-size canary, order-preservation |
| `Phase4_CallbackCheck/` | Sustained 30 s capture, callback counters, dispatch latency, replacement, detach |
| `Phase5_APIParityCheck/` | `setBufferSize()` before/after `begin()`, `setGain()` RMS response |
| `PDMSerialPlotter_RAK18003/` | Original PDM plotter, plus opt-in `FULL_DUPLEX_TEST` for concurrent speaker |

### Build note

`library.properties` no longer sets `precompiled=true` /
`ldflags=-lDSpotterSDK`. No precompiled `.a` shipped for ESP32-S3
anyway, so `precompiled=true` was silently skipping the RAK3112
`.cpp` builds. The DSPG voice-recognition examples
(`VoiceRecognition_DSPG`, `DSPG_PlayBack`) won't link with those
disabled — re-enable when working on DSPG.



















































