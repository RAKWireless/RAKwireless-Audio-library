| <center><img src="./assets/rakstar.jpg" alt="RAKstar" width=25%></center>  | ![RAKWireless](./assets/RAK-Whirls.png) | [![Build Status](https://github.com/RAKWireless/RAK13800_W5100S/workflows/RAK%20Library%20Build%20CI/badge.svg)](https://github.com/RAKWireless/RAK13800_W5100S/actions) |
| -- | -- | -- |

<img src="assets/rakstar.jpg" style="zoom:50%;" /><img src="assets/RAK-Whirls.png" alt="RAK-Whirls" style="zoom:150%;" />

# RAKwireless Audio library(just draft)



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
- RAK18030: [20-20kHz Microphone](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/142377158/RAK18030+-+20-20kHz+Microphone)                             
- RAK18031:[100-10kHz Microphone](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/142344614/RAK18031+-+100-10kHz+Microphone)                             
- [RAK18032: Ultrasonic Microphone](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/141329983/RAK18032+-+Ultrasonic+Microphone)
- [RAK18033: Stereo Mic Header Board](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/143097861/RAK18033+-+Stereo+Mic+Header+Board)                                   
- [RAK18040: Analog Mic to I2S](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/142901249/RAK18040+-+Analog+Mic+to+I2S)         
- [RAK18060: 5.6W Stereo Amplifier](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/142672154/RAK18060+-+5.6W+Stereo+Amplifier)           
- [RAK18061: 5.6W Amplifier](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/142606623/RAK18061+-+5.6W+Amplifier)                                                   
- RAK18080: DSPG board, audio process board

At the same time we will release some audio kit

- [WisBlock Microphone Interposer Kit](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/182419457/WisBlock+Microphone+Interposer+Kit)                                             
- [WisBlock Speaker Amplifier SD Kit](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/188645377/WisBlock+Speaker+Amplifier+SD+Kit)                    
- [WisBlock Remote Microphone Kit](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/188612616/WisBlock+Remote+Microphone+Kit)         
- [WisBlock Stereo Microphone Kit](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/188612609/WisBlock+Stereo+Microphone+Kit)                       
- [Amplifier Microphone & SD Kit](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/182223279)                    
- [WisBlock Voice Processing Kit](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/185892865/WisBlock+Voice+Processing+Kit)                    
- [WisBlock Voice Processing Speaker Kit](https://rakwireless.atlassian.net/wiki/spaces/WISAUDIO/pages/185925633/WisBlock+Voice+Processing+Speaker+Kit)                             
- WisBlock Audio Accessories               

All elements can be used separately or together. However, there are two points need to note:

- They all use IO connector
- We advise to use them together and be sure the microphone placed on the top of stack.



## Installation

#### BSP Update

Please update BSP part to the last version, which will support basic driver.

#### Rakwireless Audio library

Then install audio library, which contains other part of driver, like amplifier, DSPG. And includes all examples and solutions.



## Examples & Solutions

No matter which core is used, the examples and solutions are basic same. The test code is just for test and can be ingnored.



- #### Audio Graph

  - Original Data Graph
  - FFT Graph

- #### Aduio Alarm

  - With LoRa
  - With WIFI
  - Local

- #### Voice Recognition

  - WisCore
    - WisDM
    - RGB
    - Music
  - DSPG

- #### Play & Record

  - Play from SD
  - Record to SD





















































