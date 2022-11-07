#include <Arduino.h>
#include "audio.h"
#include <SPI.h>
#include <SD.h>

File            audiofile;
static uint32_t f_startPos = 44;  //audio data start position
static uint32_t f_readPos = 0;    //read audio data position
static uint32_t f_endPos = 0;     //audio data end position

#define PRINT_SOUND_INFO    0
#define WAV_HEAD_LEN 320
typedef struct wavhead_s
{
  /*RIFF块*/
  char RIFF_BUF[4];       //（RIFF）  0x46464952
  uint32_t wavFileLen;    //Data size, including header size and audio file size (total file size - 8)
  /*Format Chunk*/
  char WAVEfmt_BUF[8];    //format type（"WAVE"）   0x45564157   //"fmt "  0X20746D66
  uint32_t fmtLen;        //Sizeof(WAVEFORMATEX)
  uint16_t code;          //Encoding format  WAVE_FORMAT_PCM、WAVEFORMAT_ADPCM
  uint16_t channels;      //Number of channels, 1 for mono, 2 for stereo
  uint32_t sampleRate;     //Sampling frequency
  uint32_t bytePerSecond;    //data volume per second
  uint16_t doubleBitsPerSample;    //
  uint16_t singleBitsPerSample;   //
  char DATA_BUF[4];          //"data"
  uint32_t audioDataSize;   //size of audio data
  float playTime;
  uint32_t fileSize;
} wavhead_t;
wavhead_t g_wav_head;
//---------------------------------------------------------------------------

void AMP_init(void)
{
  if (!AMP_Left.begin(AMP_LEFT_ADDRESS))
  {
    Serial.printf("TAS2560 left init failed\r\n");
  }
  AMP_Left.set_pcm_channel(LeftMode);
  //  AMP_Left.set_gain(8);
  if (!AMP_Right.begin(AMP_RIGTT_ADDRESS))
  {
    Serial.printf("TAS2560 rigth init failed\r\n");
  }
  AMP_Right.set_pcm_channel(RightMode);
  //  AMP_Right.set_gain(8);
  rak_audio.setVolume(1);
}

void set_AMP_mute()
{
  AMP_Left.set_mute();
  AMP_Right.set_mute();
}
void set_AMP_unmute()
{
  AMP_Left.set_unmute();
  AMP_Right.set_unmute();
}
void SD_init(void)
{
  SD_CS_low();  //enable the SPI CS
  if (!SD.begin(SS, SPI, 8000000, "/sd", 5)) { // Start access to the SD.
    Serial.println("Card Mount Failed");
  }
  else
  {
    memset(playList, 0, sizeof(playList));
    listDir("/", 0);
    printMusicList();
  }
  SD_CS_high(); //disable the SPI CS
}

void SD_CS_high()
{
  Expander2.digitalWrite(2, 1);
}
void SD_CS_low()
{
  Expander2.digitalWrite(2, 0);
  delay(10);//vTaskDelay(20);//delay(20);
}
bool endsWith (const char* base, const char* str)
{
  //fb
  int slen = strlen(str) - 1;
  const char *p = base + strlen(base) - 1;
  while (p > base && isspace(*p)) p--; // rtrim
  p -= slen;
  if (p < base) return false;
  return (strncmp(p, str, slen) == 0);
}
int lastIndexOf(const char* haystack, const char* needle)
{
  //fb
  int nlen = strlen(needle);
  if (nlen == 0) return -1;
  const char *p = haystack - nlen + strlen(haystack);
  while (p >= haystack) {
    int i = 0;
    while (needle[i] == p[i])
      if (++i == nlen) return p - haystack;
    p--;
  }
  return -1;
}
void read_file_header(uint8_t *wav_head, struct wavhead_s *a)
{
  if (NULL == wav_head || NULL == a)
  {
    Serial.printf("%s %d parameter NULL\n", __FUNCTION__, __LINE__);
    return;
  }
  a->RIFF_BUF[0] = wav_head[0];//'R'
  a->RIFF_BUF[1] = wav_head[1];//'I'
  a->RIFF_BUF[2] = wav_head[2];//'F'
  a->RIFF_BUF[3] = wav_head[3];//'F'
#if(PRINT_SOUND_INFO > 2)
  {
    Serial.printf("RIFF_BUF = %c%c%c%c \n", a->RIFF_BUF[0], a->RIFF_BUF[1],
                  a->RIFF_BUF[2], a->RIFF_BUF[3]);
  }
#endif

  uint32_t ntemp = 0;
  ntemp = wav_head[7] & 0xff;
  ntemp = (ntemp << 8) | wav_head[6];
  ntemp = (ntemp << 8) | wav_head[5];
  ntemp = (ntemp << 8) | wav_head[4];
  a->wavFileLen = ntemp;
  f_endPos = ntemp + 8;
  a->fileSize = ntemp + 8;
#if(PRINT_SOUND_INFO > 2)
  {
    Serial.printf("wavFileLen = %d\n", a->wavFileLen);
  }
#endif
  a->WAVEfmt_BUF[0] = wav_head[8];//'W'
  a->WAVEfmt_BUF[1] = wav_head[9];//'A'
  a->WAVEfmt_BUF[2] = wav_head[10];//'V'
  a->WAVEfmt_BUF[3] = wav_head[11];//'E'
  a->WAVEfmt_BUF[4] = wav_head[12];//'f'
  a->WAVEfmt_BUF[5] = wav_head[13];//'m'
  a->WAVEfmt_BUF[6] = wav_head[14];//'t'
  a->WAVEfmt_BUF[7] = wav_head[15];//''
#if(PRINT_SOUND_INFO > 2)
  {
    Serial.printf("WAVEfmt_BUF = %c%c%c%c%c%c%c%c \n", a->WAVEfmt_BUF[0], a->WAVEfmt_BUF[1],
                  a->WAVEfmt_BUF[2], a->WAVEfmt_BUF[3],
                  a->WAVEfmt_BUF[4], a->WAVEfmt_BUF[5],
                  a->WAVEfmt_BUF[6], a->WAVEfmt_BUF[7]);
  }
#endif
  ntemp = 0;
  ntemp = wav_head[19] & 0xff;
  ntemp = (ntemp << 8) | wav_head[18];
  ntemp = (ntemp << 8) | wav_head[17];
  ntemp = (ntemp << 8) | wav_head[16];
  a->fmtLen = ntemp;
#if(PRINT_SOUND_INFO > 1)
  {
    Serial.printf("fmtLen = %d\n", a->fmtLen);
  }
#endif
  //1:PCM 6:G711A 7:G711U
  a->code = (wav_head[21] & 0xFF);
  a->code = (a->code << 8) | wav_head[20];
#if(PRINT_SOUND_INFO > 2)
  {
    Serial.printf("code = %d\n", a->code);
  }
#endif
  //Number of channels 1: Mono 2: Stereo
  a->channels = (wav_head[23] & 0xFF);
  a->channels = (a->channels << 8) | wav_head[22];
#if(PRINT_SOUND_INFO > 1)
  {
    Serial.printf("channels = %d\n", a->channels);
  }
#endif
  /*Sample rate For example: 8000 16000 44100 The sample rate (samples per second)
    indicates the playback speed of each channel */
  ntemp = 0;
  ntemp = wav_head[27] & 0xff;
  ntemp = (ntemp << 8) | wav_head[26];
  ntemp = (ntemp << 8) | wav_head[25];
  ntemp = (ntemp << 8) | wav_head[24];
  a->sampleRate = ntemp;
#if(PRINT_SOUND_INFO > 1)
  {
    Serial.printf("sampleRate = %d\n", a->sampleRate);
  }
#endif
  //The amount of audio data per second = sample rate * number of channels * number of samples / 8
  ntemp = 0;
  ntemp = wav_head[31] & 0xff;
  ntemp = (ntemp << 8) | wav_head[30];
  ntemp = (ntemp << 8) | wav_head[29];
  ntemp = (ntemp << 8) | wav_head[28];
  a->bytePerSecond = ntemp;
#if(PRINT_SOUND_INFO > 1)
  {
    Serial.printf("bytePerSecond = %d\n", a->bytePerSecond);
  }
#endif
  //The size of each sample = the number of samples * the number of channels / 88
  a->doubleBitsPerSample = (wav_head[33] & 0xFF);
  a->doubleBitsPerSample = (a->doubleBitsPerSample << 8) | wav_head[32];
#if(PRINT_SOUND_INFO > 2)
  {
    Serial.printf("doubleBitsPerSample = %d\n", a->doubleBitsPerSample);
  }
#endif
  a->singleBitsPerSample = (wav_head[35] & 0xFF);
  a->singleBitsPerSample = (a->singleBitsPerSample << 8) | wav_head[34];
#if(PRINT_SOUND_INFO > 1)
  {
    Serial.printf("singleBitsPerSample = %d\n", a->singleBitsPerSample);
  }
#endif
  uint16_t data_pos = 35;
  while ((data_pos++) && (data_pos < WAV_HEAD_LEN))
  {
    if ((wav_head[data_pos] == 'd') && (wav_head[data_pos + 1] == 'a') && (wav_head[data_pos + 2] == 't') && (wav_head[data_pos + 3] == 'a'))
    {
      f_startPos = data_pos + 8;
      f_readPos = f_startPos;
      audiofile.seek(f_startPos);
#if(PRINT_SOUND_INFO > 1)
      {
        Serial.printf("data block position:%d\r\n", f_startPos);
      }
#endif
      break;
    }
    //     Serial.printf("data_pos:%d\r\n",data_pos);
  }

  a->DATA_BUF[0] = wav_head[data_pos];  //'d'
  a->DATA_BUF[1] = wav_head[++data_pos];//'a'
  a->DATA_BUF[2] = wav_head[++data_pos];//'t'
  a->DATA_BUF[3] = wav_head[++data_pos];//'a'
#if(PRINT_SOUND_INFO > 2)
  {
    Serial.printf("Chunk = %c%c%c%c \n", a->DATA_BUF[0], a->DATA_BUF[1], a->DATA_BUF[2], a->DATA_BUF[3]);
  }
#endif
  //Length of audio data = total file length - 44 (wav header length)
  ntemp = 0;
  ntemp = wav_head[data_pos + 4] & 0xff;
  ntemp = (ntemp << 8) | wav_head[data_pos + 3];
  ntemp = (ntemp << 8) | wav_head[data_pos + 2];
  ntemp = (ntemp << 8) | wav_head[data_pos + 1];
  a->audioDataSize = ntemp;
  a->playTime = (double)(a->audioDataSize) / (a->bytePerSecond);
#if(PRINT_SOUND_INFO > 1)
  {
    Serial.printf("audioDataSize = %d\n", a->audioDataSize);
    Serial.printf("audio play time = %3.3f S\r\n", a->playTime);
  }
#endif

}
void get_flie_info(void)
{
  uint8_t s_wav_head[WAV_HEAD_LEN] = {0};
  audiofile.seek(0x0);  //read file data from first byte
  audiofile.read(s_wav_head, sizeof(s_wav_head));  //read .wav file head
  read_file_header(s_wav_head, &g_wav_head);
}

bool findWAVfile(const char* name)
{
  if (name == NULL) return false;
  //  Serial.printf("audioName:%s\r\n", name);

  char* afn = NULL;  // audioFileName
  afn = strdup(name);
  uint8_t dotPos = lastIndexOf(afn, ".");
  for (uint8_t i = dotPos + 1; i < strlen(afn); i++)
  {
    afn[i] = toLowerCase(afn[i]);
  }

  if (endsWith(afn, ".wav"))
  { // WAVE section
    free(afn);
    return true;
  } // end WAVE section

  if (afn) free(afn);
  return false;
}
void printMusicList()
{
  Serial.println("the music list:");
  for (int i = 0; i < musicCount; i++)
  {
    Serial.printf("[%d]:%s\r\n", i, playList[i]);
  }
}
int sizeof_pdata(char *pdata)
{
  if (pdata == NULL)
    return 0;
  int i = 0;
  while (pdata[i] != '\0')
  {
    i++;
  }
  return i;
}
void listDir( const char * dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);
  File root = SD.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(file.name(), levels - 1);
      }
    } else {
      Serial.print("File: ");
      Serial.print(file.name());
      Serial.print("  Size: ");
      Serial.println(file.size());

      if (findWAVfile(file.name()))
      {
        if (musicCount < MUSIC_LIST)
        {
          memcpy(playList[musicCount], file.name(), strlen(file.name()) + 1);
          //          Serial.printf("###musicCount:%d  sizeof(pdata):%d  find WAV:%s\r\n", musicCount,  strlen(file.name()), playList[musicCount]);
          musicCount++;
        }
      }
    }
    file = root.openNextFile();
  }
}
bool connectFS(fs::FS &fs, const char* path)
{
  if (strlen(path) > 255) return false;
  char audioName[256];
  memcpy(audioName, path, strlen(path) + 1);
  //  Serial.printf("path:%s\r\n", path);
  if (audioName[0] != '/')
  {
    for (int i = 255; i > 0; i--)
    {
      audioName[i] = audioName[i - 1];
    }
    audioName[0] = '/';
  }
  //  Serial.printf("audioName:%s\r\n", audioName);
  if (fs.exists(audioName))
  {
    audiofile = fs.open(audioName);
  }

  if (!audiofile)
  {
    Serial.println("Failed to open file for reading");
    return false;
  }
  char* afn = NULL;  // audioFileName
#ifdef SDFATFS_USED
  audiofile.getName(chbuf, sizeof(chbuf));
  afn = strdup(chbuf);
#else
  afn = strdup(audiofile.name());
#endif

  uint8_t dotPos = lastIndexOf(afn, ".");
  for (uint8_t i = dotPos + 1; i < strlen(afn); i++)
  {
    afn[i] = toLowerCase(afn[i]);
  }

  if (endsWith(afn, ".wav"))
  { // WAVE section
    free(afn);
    return true;
  } // end WAVE section
  audiofile.close();
  if (afn) free(afn);
  return false;
}

void play_wav(const char* file)
{
  char audioName[256];
  memcpy(audioName, file, strlen(file) + 1);
  if (file)
  {
    SD_CS_low();  //enable the SPI
    if (connectFS(SD, audioName))
    {
      I2S.end();
      delay(10);
      get_flie_info();
      I2S.setSampleBit(g_wav_head.singleBitsPerSample);
      I2S.begin(g_wav_head.channels, g_wav_head.sampleRate);
      //      Serial.printf("######Config I2S Ch:%d\t BIT:%d\t FS:%d \r\n", g_wav_head.channels, g_wav_head.singleBitsPerSample, g_wav_head.sampleRate);
      play_task();
    }
    else
    {
      SD_CS_high(); //disable the SPI CS
      Serial.println("fail to open the file");
    }
  }
}
void close_file()
{
  if (!I2S.end())
  {
    Serial.println("I2S stop error");
  }
  audiofile.close();
}
void play_music()
{
  int16_t sample[2] = {0};
  int bytes_read = audiofile.read((uint8_t*)&sample, sizeof(sample));
  //  I2S.write((void *) &sample, sizeof(sample));
  uint32_t s32 = rak_audio.Gain(sample); // vosample2lume;
  I2S.write((void *) &s32, sizeof(uint32_t));
  f_readPos = f_readPos + bytes_read;
  if (f_readPos >= f_endPos)
  {
    f_readPos = f_startPos;
    audiofile.seek(f_startPos);
  }
}
void play_task(void)
{
  SD_CS_low();  //enable the SPI
  audiofile.seek(f_startPos);
  f_readPos = f_startPos;
  set_AMP_unmute();
}
void reconfig_i2s(void)
{
  I2S.setSampleBit(g_wav_head.singleBitsPerSample);
  I2S.begin(g_wav_head.channels, g_wav_head.sampleRate);
}
void play_pause(void)
{
  set_AMP_mute();
  I2S.clearDMA();
}
void stop_play(void)
{
  //  SD_CS_low();  //enable the SPI
  //  delay(10);
  //  Serial.println("stop play");
  //  delay(200);
  //  SD_CS_high(); //disable the SPI CS
  set_AMP_mute();
  I2S.clearDMA();
  if (!I2S.end())
  {
    Serial.println("I2S stop error");
  }
  audiofile.close();
  Serial.println("stop play");
}
