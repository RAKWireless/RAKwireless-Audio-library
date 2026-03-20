# WisBlock Audio Example: Audio Hardware Module Usage Table (Statistics by xxx `.ino`)

> Note: This table only counts audio hardware modules (`18003/18030/18031/18040/18060/18080`). 
>
> Summary: Determined by whether the corresponding hardware interface is "explicitly used/declared" in the example `.ino`;

## Module legend

| Module ID | Meaning |
|---|---|
| 18003 | RAK18003 (TPT29555 IO expander) |
| 18030 | RAK18030 (PDM, 16kHz) |
| 18031 | RAK18031 (PDM, high-rate) |
| 18040 | RAK18040 (NAU85L40B analog mic ADC) |
| 18060 | RAK18060 (TAS2560 I2S amplifier) |
| 18080 | RAK18080 (CDSpotter / DSPG voice recognition) |

## RAK3112

| Example (.ino) | Audio hardware modules (IDs) |
|---|---|
| `AnalogMicAudio_Alarm.ino` | 18040 |
| `AnalogMicRecordToSD.ino` | 18003,18040 |
| `AnalogMicSerialPlotter.ino` | 18040 |
| `AnalogMicSerialPlotterFFT.ino` | 18040 |
| `Audio_Alarm.ino` | 18030 |
| `Audio_Alarm_RAK18003.ino` | 18003,18030 |
| `HighRatePDMSerialPlotterFFT.ino` | 18031 |
| `HighRatePDMSerialPlotterFFT_RAK18003.ino` | 18003,18031 |
| `HighRatePDMWiFiClient.ino` | 18031 |
| `HighRatePDMWiFiClient_RAK18003.ino` | 18003,18031 |
| `PDMSerialPlotter.ino` | 18030 |
| `PDMSerialPlotter_RAK18003.ino` | 18003,18030 |
| `PDMSerialPlotterFFT.ino` | 18030 |
| `PDMSerialPlotterFFT_RAK18003.ino` | 18003,18030 |
| `PlayBack.ino` | 18060 |
| `PlayBack22K.ino` | 18060 |
| `PlayBack48KHz.ino` | 18060 |
| `PlayBackFromSD.ino` | 18003,18060 |
| `RAK18003_example.ino` | 18003 |
| `RAK1806X_test.ino` | 18060 |
| `RAK1806X_test_RAK18003.ino` | 18003,18060 |
| `RecordToSD.ino` | 18003,18030 |

## RAK4631

| Example (.ino) | Audio hardware modules (IDs) |
|---|---|
| `AnalogMicAudio_Alarm.ino` | 18040 |
| `AnalogMicPlayBack.ino` | 18040,18060 |
| `AnalogMicRecordToSD.ino` | 18003,18040 |
| `AnalogMicSerialPlotter.ino` | 18040 |
| `AnalogMicSerialPlotterFFT.ino` | 18040 |
| `Audio_Alarm.ino` | 18003,18030 |
| `Audio_Alarm_RAK18003.ino` | 18003,18030 |
| `DSPG_PlayBack.ino` | 18003,18060,18080 |
| `HighRatePDMSerialPlotterFFT.ino` | 18031 |
| `HighRatePDMSerialPlotterFFT_RAK18003.ino` | 18003,18031 |
| `LoRaWAN_AnalogMicAudio_Alarm.ino` | 18040 |
| `LoRaWAN_Audio_Alarm.ino` | 18030 |
| `LoRaWAN_Audio_Alarm_HighRate.ino` | 18031 |
| `LoRaWAN_Audio_Alarm_HighRate_RAK18003.ino` | 18003,18031 |
| `LoRaWAN_Audio_Alarm_RAK18003.ino` | 18003,18030 |
| `PDMSerialPlotter.ino` | 18030 |
| `PDMSerialPlotter_RAK18003.ino` | 18003,18030 |
| `PDMSerialPlotterFFT.ino` | 18030 |
| `PDMSerialPlotterFFT_RAK18003.ino` | 18003,18030 |
| `PlayBack.ino` | 18060 |
| `PlayBack22K.ino` | 18060 |
| `PlayBack48K.ino` | 18060 |
| `PlayBackFromSD.ino` | 18003,18060 |
| `RAK18003_example.ino` | 18003 |
| `RAK1806X_test.ino` | 18003,18060 |
| `RAK1806X_test_RAK18003.ino` | 18003,18060 |
| `VoiceConfigWisDM.ino` | 18003,18030,18080 |
| `VoiceControlRGB.ino` | 18003,18030,18080 |
| `VoiceRecognition_DSPG.ino` | 18003,18080 |
| `VoiceRecognitionL1.ino` | 18030,18080 |
| `VoiceRecognitionL1_PlayBack.ino` | 18003,18030,18060,18080 |
| `VoiceRecognitionL1_RAK18003.ino` | 18003,18030,18080 |

## RAK11200

| Example (.ino) | Audio hardware modules (IDs) |
|---|---|
| `AnalogMicAudio_Alarm.ino` | 18040 |
| `AnalogMicRecordToSD.ino` | 18003,18040 |
| `AnalogMicSerialPlotter.ino` | 18040 |
| `AnalogMicSerialPlotterFFT.ino` | 18040 |
| `Audio_Alarm.ino` | 18003,18030 |
| `Audio_Alarm_RAK18003.ino` | 18003,18030 |
| `DSPG_PlayBack.ino` | 18003,18060,18080 |
| `HighRatePDMSerialPlotterFFT.ino` | 18031 |
| `HighRatePDMSerialPlotterFFT_RAK18003.ino` | 18003,18031 |
| `HighRatePDMWiFiClient.ino` | 18031 |
| `HighRatePDMWiFiClient_RAK18003.ino` | 18003,18031 |
| `PDMSerialPlotter.ino` | 18030 |
| `PDMSerialPlotter_RAK18003.ino` | 18003,18030 |
| `PDMSerialPlotterFFT.ino` | 18030 |
| `PDMSerialPlotterFFT_RAK18003.ino` | 18003,18030 |
| `PlayBack.ino` | 18060 |
| `PlayBack22K.ino` | 18060 |
| `PlayBack48KHz.ino` | 18060 |
| `PlayBackFromSD.ino` | 18003,18060 |
| `RAK18003_example.ino` | 18003 |
| `RAK1806X_test.ino` | 18060 |
| `RAK1806X_test_RAK18003.ino` | 18003,18060 |
| `RecordToSD.ino` | 18003,18030 |
| `VoiceControlRGB.ino` | 18003,18030,18080 |
| `VoiceRecognition_DSPG.ino` | 18003,18080 |
| `VoiceRecognitionL1.ino` | 18030,18080 |
| `VoiceRecognitionL1_PlayBack.ino` | 18003,18030,18060,18080 |
| `VoiceRecognitionL1_RAK18003.ino` | 18003,18030,18080 |
| `VoiceRecognitionPlayMusic.ino` | 18003,18060 |
| `WiFiClient_AnalogMicAudioAlarm.ino` | 18040 |
| `WiFiClient_Audio_Alarm.ino` | 18030 |
| `WiFiClient_Audio_Alarm_RAK18003.ino` | 18003,18030 |

## RAK11300

| Example (.ino) | Audio hardware modules (IDs) |
|---|---|
| `AnalogMicAudio_Alarm.ino` | 18040 |
| `AnalogMicSerialPlotter.ino` | 18040 |
| `AnalogMicSerialPlotterFFT.ino` | 18040 |
| `Audio_Alarm.ino` | 18003,18030 |
| `Audio_Alarm_RAK18003.ino` | 18003,18030 |
| `DSPG_VoiceRecognition.ino` | 18003,18080 |
| `HighRatePDMSerialPlotterFFT.ino` | 18031 |
| `HighRatePDMSerialPlotterFFT_RAK18003.ino` | 18003,18031 |
| `LoRaWAN_AnalogMicAudio_Alarm.ino` | 18040 |
| `LoRaWAN_Audio_Alarm.ino` | 18030 |
| `LoRaWAN_Audio_Alarm_HighRate.ino` | 18031 |
| `LoRaWAN_Audio_Alarm_HighRate_RAK18003.ino` | 18003,18031 |
| `LoRaWAN_Audio_Alarm_RAK18003.ino` | 18003,18030 |
| `PDMSerialPlotter.ino` | 18030 |
| `PDMSerialPlotter_RAK18003.ino` | 18003,18030 |
| `PDMSerialPlotterFFT.ino` | 18030 |
| `PDMSerialPlotterFFT_RAK18003.ino` | 18003,18030 |
| `PlayBack.ino` | 18060 |
| `PlayBack22K.ino` | 18060 |
| `PlayBack48.ino` | 18060 |
| `RAK18003_example.ino` | 18003 |
| `RAK1806X_test.ino` | 18003,18060 |
| `RAK1806X_test_RAK18003.ino` | 18003,18060 |
| `VoiceConfigWisDM.ino` | 18003,18030,18080 |
| `VoiceControlRGB.ino` | 18003,18030,18080 |
| `VoiceRecognition_DSPG.ino` | 18003,18080 |
| `VoiceRecognitionL0.ino` | 18030,18080 |
| `VoiceRecognitionL0_RAK18003.ino` | 18003,18030,18080 |

