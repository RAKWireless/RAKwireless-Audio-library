/**
   @file Text_to_Speech.ino
   @author rakwireless.com
   @version 1.2
   @date 2026-07-03
   @copyright Copyright (c) 2026
   @brief Hybrid Text-to-Speech for RAKwireless WisBlock (RAK3112).

   Input text arrives over the USB serial port and is spoken through the
   RAK18060 amplifier. Words and phrases that have a recorded WAV on the SD
   card are played back; anything else is synthesized on-device with
   arduino-espeak-ng. An optional Espressif Chinese TTS engine can be
   compiled in for dedicated Chinese synthesis (the SAYCN command). All
   audio is streamed through one shared I2S output. 
   Note: Chinese text does not use the pre-recorded WAV files.

   PIPELINE

    THREE-TIER HYBRID TTS (highest quality wins)
     Tier 1  phrases  - whole pre-recorded sentences   (/tts/phrases)
     Tier 2  words    - pre-recorded word/number bank   (/tts/words)
     Tier 3  eSpeak   - on-device synthesis fallback    (anything unknown)

     SAY:<text>
       -> preprocess / tokenise / number-expand
       -> resolve into an ordered SEGMENT list:
            FILE  segment  = a recorded WAV (phrase tier or word tier)
            SPEAK segment  = text to synthesize with eSpeak (fallback tier)
          consecutive unknown words are coalesced into one SPEAK segment.
       -> pre-render every SPEAK segment to a PSRAM PCM buffer
       -> open ONE I2S session at 22050 Hz and play all segments back-to-back
          (recorded + synthesized), gapless.

     SAYCN:<text>  (only when ENABLE_ESPRESSIF_TTS is defined)
       -> verify the input contains UTF-8 Chinese characters
       -> render the whole utterance to a PCM buffer via esp_tts_bridge
          (the Espressif Chinese TTS engine)
       -> play the buffer through the same I2S output at the engine's native
          sample rate.
     SAYCN bypasses the three-tier hybrid entirely - it does NOT touch the
     phrase / word / eSpeak paths. When the toggle is off, the SAYCN command,
     its help entry and the Espressif_TTS library are all compiled out; the
     sketch builds and runs without that library present.

   SAMPLE RATE
     The hybrid pipeline (SAY / SPEAK / PLAY + eSpeak) runs at 22050 Hz /
     16-bit / mono. eSpeak outputs 22050 Hz natively; the recorded WAV
     library must be generated at 22050 Hz to match. No resampling, no
     per-segment I2S re-init.
     The optional Espressif Chinese TTS engine has its own native rate
     (typically 16 kHz, depending on the voice); each SAYCN utterance opens
     its own I2S session at that rate.

   eSpeak audio is collected into RAM via a Print-derived sink (PcmCollector)
   and is NOT sent to I2S directly - the RAK audio library's I2S object stays
   the single owner of the I2S peripheral.

   LIMITATIONS
     Playback and synthesis are synchronous (blocking). Once a SAY:/SPEAK:/
     PLAY: command starts producing audio it runs to completion - there is no
     STOP command and a command cannot be interrupted mid-utterance. Any text
     typed during playback is buffered and processed afterwards.

   STATUS LEDS
     GREEN  lit while audio is playing.
     BLUE   lit when a command or operation hits an error; cleared at the
            start of the next command.

    MAP (in source order)
     - setup / loop ............ boot + serial command loop
     - espeak_setup ............ initialise the eSpeak fallback engine
     - Espressif TTS ........... esp_tts_bridge include / stubs and the SAYCN
                                 handler (only when ENABLE_ESPRESSIF_TTS is
                                 defined; otherwise dead code + stubs)
     - command handling ........ printHelp, resolveAudioPath, handleCommand
                                 (SAY, SAYCN, SPEAK, PLAY, VOL, RATE, LIST,
                                  RELOAD, HELP, STATUS)
     - SAY pipeline ............ handleSay (text -> segment list -> playback)
     - eSpeak render/playback .. renderSpeak, stream_pcm_to_i2s, say_segments
                                 (say_segments also runs the ESP-TTS render
                                 branch for espTts segments)
     - library loading ......... manifest parsing + word/phrase indexes
     - text/number processing .. preprocessText, tokenize, expandNumber, ...
     - WAV / SD / amp .......... read_format_only, stream_wav_to_i2s, SD/amp init

   REQUIRED LIBRARIES (git clone into ~/Documents/Arduino/libraries)
     - arduino-espeak-ng     https://github.com/pschatzmann/arduino-espeak-ng
     - arduino-posix-fs      https://github.com/pschatzmann/arduino-posix-fs
     - arduino-audio-tools   https://github.com/pschatzmann/arduino-audio-tools
   eSpeak voice data is served from PROGMEM by arduino-posix-fs (no SD load).

   OPTIONAL LIBRARY (only when ENABLE_ESPRESSIF_TTS is defined)
     - Espressif_TTS   Local library shipped with this project at
                       `lib/Espressif_TTS/` (esp_tts_bridge.{h,cpp}). Wraps
                       the ESP-IDF Chinese TTS component and exposes the
                       espTtsSetup / espTtsContainsChinese / espTtsRenderChinese
                       symbols the SAYCN command uses. Also requires the
                       Espressif TTS voice-data partition scheme - see
                       partitions_esp_tts.csv and BUILD SETTINGS below.
                       With ENABLE_ESPRESSIF_TTS undefined the sketch provides
                       inline no-op stubs for the three symbols above so it
                       compiles even without this library present.

   *** REQUIRED: regenerate the eSpeak voice data ***
   The espeak-ng-data bundled in arduino-espeak-ng is stale and buggy - it
   mis-synthesises any word containing the letter pair "st" (test, best,
   stop, sister, ...), converting "st" to the U+FB06 ST-ligature and spelling
   the word out as garbage. Before this sketch will work correctly you must
   replace that data with current official data and regenerate the PROGMEM
   headers:
       brew install espeak-ng
       cd ~/Documents/Arduino/libraries/arduino-espeak-ng
       rm -rf espeak-ng-data
       cp -R "$(brew --prefix)/share/espeak-ng/espeak-ng-data" espeak-ng-data
       python3 regen_data.py
   regen_data.py is a standalone helper kept in the library root. This step
   must be repeated whenever arduino-espeak-ng is reinstalled or updated.
   See the project README for full details.
   (See Readme.md file for windows instructions)

   REQUIRED BUILD SETTINGS (Arduino IDE)
     - Board:            RAKwireless RAK3112
     - ESP32 core:       3.3.x (mandatory for ESP_I2S.h)
     - Partition Scheme: Default 16MB (6.25MB APP) for the eSpeak-only build.
                         When ENABLE_ESPRESSIF_TTS is defined use "Custom"
                         with partitions_esp_tts.csv (it adds the voice_data
                         partition the Espressif TTS engine loads from).
     - USB CDC On Boot:  Enabled
     - PSRAM:            OPI PSRAM enabled
     - Details:          see README_ARDUINO.md in this folder

   Hardware: RAK3112 + RAK19007 + RAK18003 + RAK18060.
**/
#include "Arduino.h"
#include "audio.h"          // RAKwireless-Audio-library
#include "SPI.h"
#include "SD.h"
#include <vector>
#include <map>

// RAKwireless audio.h defines Mono/Stereo (and similar) as preprocessor
// macros. AudioTools uses those words as enum members
// (e.g. enum class I2SChannelSelect { Stereo, Left, Right, Default }), so the
// macros corrupt the enum and the build fails. Undefine them before pulling
// in AudioTools. This sketch never uses these macros directly - the RAK amp
// channel is set with LeftMode/RightMode, which are not undefined here.
#ifdef Stereo
#undef Stereo
#endif
#ifdef Mono
#undef Mono
#endif
#ifdef Left
#undef Left
#endif
#ifdef Right
#undef Right
#endif

// --- eSpeak / AudioTools stack ---
#include "AudioTools.h"     // required by arduino-espeak-ng
#include "FileSystems.h"    // arduino-posix-fs: eSpeak data from PROGMEM
#include "espeak.h"         // ESpeak class

// Compile-time toggle for the Espressif Chinese TTS engine.
//   defined     -> the Espressif_TTS library is pulled in, the SAYCN command
//                  is available, and the say_segments() render branch calls
//                  esp_tts_bridge for espTts segments.
//   undefined   -> the Espressif_TTS library is not required to be present;
//                  the SAYCN command and its help entry are compiled out and
//                  inline no-op stubs stand in for the bridge API (nothing
//                  ever calls them at runtime because no segment is ever
//                  flagged as espTts). Comment this line out to build without
//                  Espressif TTS support.
#define ENABLE_ESPRESSIF_TTS

#ifdef ENABLE_ESPRESSIF_TTS
  #include "esp_tts_bridge.h" // Espressif Chinese TTS (optional, ENABLE_ESPRESSIF_TTS)
#else
  // ---- ENABLE_ESPRESSIF_TTS is NOT defined ----
  // Provide inline stubs so the rest of the sketch compiles cleanly without
  // the Espressif_TTS library installed. With the library disabled:
  //   - the SAYCN command and its help entry are compiled out (see below),
  //   - no segment ever gets espTts=true, so the branch in say_segments()
  //     that calls the render function is dead code,
  //   - these stubs exist only so those dead references still compile.
  static inline bool espTtsSetup()                        { return false; }
  static inline bool espTtsContainsChinese(const String&) { return false; }
  static inline bool espTtsRenderChinese(const String&, uint8_t**, size_t*, int)
                                                          { return false; }
#endif

// ============================================================
// HARDWARE OBJECTS
// ============================================================

Audio rak_audio;                       // RAK audio library: provides Gain()

#define AMP_LEFT_ADDRESS    0x4c        // I2C address, RAK18060 left  TAS2560
#define AMP_RIGTT_ADDRESS   0x4f        // I2C address, RAK18060 right TAS2560

// The RAK18003 interposer carries two TPT29555 I2C IO-expanders. They gate
// power/enable lines for the audio modules AND the microSD chip-select - the
// SD card's Chip Select is NOT an MCU GPIO, it is Expander2 pin 2 (see SD_CS_low/high).
TPT29555   Expander1(0x23);
TPT29555   Expander2(0x25);

TAS2560 AMP_Left;                       // RAK18060 left  channel amplifier
TAS2560 AMP_Right;                      // RAK18060 right channel amplifier
static bool g_ampLeftOk  = false;       // true after AMP_Left.begin() succeeds
static bool g_ampRightOk = false;       // false when only left speaker is wired

// ============================================================
// WAV FORMAT
// ============================================================

// Number of header bytes scanned when locating a WAV's "data" chunk.
#define WAV_HEAD_LEN 320

// Minimal WAV format info - all this project needs from a WAV header. 
// The header is read by say_segments() so a mismatch can be detected 
// and warned about if it is not 22050 Hz / 16-bit / mono. 
struct WavFormat
{
  uint16_t channels;
  uint32_t sampleRate;
  uint16_t bitsPerSample;
};

// ============================================================
// TTS LIBRARY CONFIG + INDEXES
// ============================================================

static const char* MANIFEST_FILE = "/tts/manifest.json";          // preferred method to find files
static const char* PHRASES_DIR   = "/tts/phrases";                 // fallback method
static const char* SILENCE_FILE  = "/tts/silence/pause_100ms.wav"; // optional
static bool         g_silenceAvailable = false;  // true if SILENCE_FILE exists
static bool         g_manifestLoaded   = false;  // true if manifest.json parsed

// Folders searched (in order) when resolving a word -> WAV file. ONLY used by
// the fallback path findWordFile(); when a manifest is loaded the in-memory
// g_words map is authoritative and these folders are not scanned.
static const char* WORD_FOLDERS[] = {
  "/tts/words/numbers/",  "/tts/words/units/",      "/tts/words/nouns/",
  "/tts/words/verbs/",    "/tts/words/adjectives/", "/tts/words/adverbs/",
  "/tts/words/pronouns/", "/tts/words/determiners/","/tts/words/common/",
  "/tts/words/glue/",
};
static const size_t WORD_FOLDERS_N = sizeof(WORD_FOLDERS) / sizeof(WORD_FOLDERS[0]);

// One phrase-library entry: the phrase text split into lowercase word tokens,
// plus the SD path of the recording. handleSay() does a greedy longest-match
// of the input word stream against these.
struct Phrase
{
  std::vector<String> words;            // e.g. {"battery","is","low"}
  String              path;             // e.g. "/tts/phrases/battery_is_low.wav"
};
std::vector<Phrase> g_phrases;           // phrase index (Tier 1)

// Word index (Tier 2). Key -> WAV path. Each manifest word is inserted under
// TWO keys: its spoken text ("twenty") and its filename stem ("20"), so digit
// tokens from the number expander resolve directly.
std::map<String, String> g_words;

// ============================================================
// eSpeak FALLBACK STATE (Tier 3)
// ============================================================

// PSRAM scratch buffer that the eSpeak sink fills during synthesis.
// 22050 Hz * 2 bytes/sample -> ~44 KB/s. 900 KB ~= 20 s of speech.
#define SPEAK_SCRATCH_CAP  (900u * 1024u)
static uint8_t* g_scratch         = nullptr; // synthesis scratch buffer (PSRAM)
static size_t   g_scratchLen      = 0;       // bytes currently in g_scratch
static bool     g_scratchOverflow = false;   // set if synthesis exceeded the cap

// PcmCollector - the audio sink we hand to the ESpeak object.
// eSpeak writes 16-bit PCM here; we simply append it to g_scratch. eSpeak is
// Synthesis goes to RAM first and is replayed later by say_segments().
class PcmCollector : public Print
{
public:
  // Single-byte sink (rarely used by eSpeak, provided for completeness).
  size_t write(uint8_t b) override
  {
    if (g_scratchLen < SPEAK_SCRATCH_CAP) g_scratch[g_scratchLen++] = b;
    else g_scratchOverflow = true;
    return 1;
  }
  // Block sink: append up to the buffer's remaining capacity. Always reports
  // the full length back to eSpeak so the synthesizer never stalls waiting
  // for a slow consumer. Anything past SPEAK_SCRATCH_CAP is dropped and flags
  // g_scratchOverflow so renderSpeak() can warn about the truncation.
  size_t write(const uint8_t* data, size_t len) override
  {
    size_t room = (g_scratchLen < SPEAK_SCRATCH_CAP)
                  ? (SPEAK_SCRATCH_CAP - g_scratchLen) : 0;
    size_t w = (len < room) ? len : room;
    if (w) { memcpy(g_scratch + g_scratchLen, data, w); g_scratchLen += w; }
    if (w < len) g_scratchOverflow = true;
    return len;   // report full length so eSpeak never stalls
  }
};
PcmCollector g_pcmSink;                  // the sink instance
ESpeak       espeak(g_pcmSink);          // eSpeak engine, writing to g_pcmSink
static bool  g_espeakReady = false;      // true once espeak_setup() succeeds
static bool  g_espTtsReady = false;      // true once espTtsSetup() succeeds
static int   g_espeakRate  = 22050;      // eSpeak output sample rate (Hz)
static int   g_espeakWpm   = 175;        // eSpeak speech rate (80..450 words/min)
static const char* BUILD_TAG = "sentence-dot-token-fix-20260706";

// One resolved piece of an utterance produced by handleSay().
struct Segment
{
  bool     speak;     // true = synthesized, false = recorded WAV file
  bool     espTts;    // SPEAK only: use Espressif TTS (Chinese) instead of eSpeak
  String   data;      // FILE: SD path to a WAV   SPEAK: text to synthesize
  uint8_t* pcm;       // SPEAK only: rendered PCM (malloc/ps_malloc'd) or nullptr
  size_t   pcmLen;    // SPEAK only: length of pcm in bytes
};

// ============================================================
// FORWARD DECLARATIONS
// ============================================================

// Hardware / SD
void   SD_CS_low();
void   SD_CS_high();
void   RAK18003Init(void);
void   SD_init(void);
void   AMP_init(void);
void   set_AMP_mute();
void   set_AMP_unmute();
void   listDir(const char* dirname, uint8_t levels);

// Status LEDs - GREEN = audio playing, BLUE = an error occurred
void   ledGreen(bool on);
void   ledBlue(bool on);

// Serial command interface
void   printHelp();
void   handleCommand(const String& cmd);
String resolveAudioPath(const String& token);

// SAY pipeline + text/number processing
void                 handleSay(const String& text);
String               preprocessText(const String& in);
std::vector<String>  tokenize(const String& text);
bool                 isAsciiToken(const String& s);
bool                 looksLikeNumber(const String& s);
std::vector<String>  expandNumber(const String& num);
void                 expandInteger(long n, std::vector<String>& out);

// WAV streaming
bool   read_format_only(const char* path, WavFormat* out);
bool   stream_wav_to_i2s(const char* path, uint16_t i2sSlots = 1);

// Library loading + lookup
void   loadLibrary();
void   loadManifest();
void   loadPhrasesFromDir();
void   parseManifest(char* buf, size_t len);
void   addPhraseEntry(const String& text, const String& path);
void   addWordEntry(const String& text, const String& path);
String lookupWord(const String& word);
String findWordFile(const String& word);
std::vector<String> splitOnChar(const String& s, char sep);

// eSpeak fallback
void   espeak_setup();
bool   renderSpeak(const String& text, uint8_t** outPcm, size_t* outLen);
void   stream_pcm_to_i2s(const uint8_t* pcm, size_t len);
uint32_t planPlaybackRate(const std::vector<Segment>& segs, uint16_t* outCh,
                          uint16_t* outBits);
void   say_segments(std::vector<Segment>& segs);

// ============================================================
// SETUP / LOOP
// ============================================================

// setup() 
// Runs once at power-on/reset. Powers the WisBlock IO rail, brings up the
// RAK18003 interposer, the RAK18060 amplifiers and the SD card, loads the
// phrase/word library, initialises the eSpeak fallback, and prints the
// command help. 
void setup()
{
  // WB_IO2 powers the WisBlock sensor/IO rail (the audio modules). Set it
  // and give the audio hardware time to come up.
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(500);

  // Open the USB serial console, but wait at most 3 s for a host to attach
  // so the device still boots when running standalone (no PC connected).
  time_t timeout = millis();
  Serial.begin(115200);
  while (!Serial)
  {
    if ((millis() - timeout) < 3000) delay(100);
    else break;
  }

  // Status LEDs start off (idle). GREEN lights during playback, BLUE on error.
  pinMode(LED_GREEN, OUTPUT); ledGreen(false);
  pinMode(LED_BLUE,  OUTPUT); ledBlue(false);

  Serial.println("======= RAK3112 Text-to-Speech =======");
  Serial.printf("Build: %s\n", BUILD_TAG);
  RAK18003Init();          // I2C IO-expanders + presence checks
  AMP_init();              // bring up both TAS2560 amplifiers
  rak_audio.setVolume(5);  // 0..21
  set_AMP_mute();          // start muted; unmute only around playback
  SD_init();               // mount the microSD card

  // Probe for the optional inter-segment silence clip.
  SD_CS_low(); delay(10);
  g_silenceAvailable = SD.exists(SILENCE_FILE);
  SD_CS_high();
  Serial.printf("Inter-segment silence: %s\n",
                g_silenceAvailable ? "ENABLED" : "disabled (file missing)");

  loadLibrary();           // build phrase + word indexes (Tiers 1 & 2)
  espeak_setup();          // initialise the eSpeak fallback (Tier 3)
  g_espTtsReady = espTtsSetup(); // Espressif Chinese TTS (optional build)

  Serial.println("===================================");
  printHelp();
  Serial.println("> ready");
}

// loop() - main loop. Reads one newline-terminated command from the serial
// console (if any) and dispatches it. NOTE: command handling is synchronous -
// while a command plays audio this loop is blocked, so commands typed during
// playback are buffered by the serial driver and run when it finishes.
void loop()
{
  if (Serial.available())
  {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) return;
    handleCommand(line);
    Serial.println("> ready");
  }
}

// ============================================================
// eSpeak setup
// ============================================================

// espeak_setup() - initialise the eSpeak-NG fallback synthesizer.
// Allocates the PSRAM synthesis scratch buffer, registers the chosen voice
// variant, starts the engine and selects the voice. On any failure it logs a
// message and leaves g_espeakReady false - the rest of the sketch then simply
// produces no audio for unknown words instead of crashing.
void espeak_setup()
{
  // Synthesis scratch buffer - prefer PSRAM, fall back to internal RAM.
  g_scratch = (uint8_t*)ps_malloc(SPEAK_SCRATCH_CAP);
  if (!g_scratch) g_scratch = (uint8_t*)malloc(SPEAK_SCRATCH_CAP);
  if (!g_scratch)
  {
    Serial.println("eSpeak: FAILED to allocate scratch buffer - fallback disabled");
    g_espeakReady = false;
    return;
  }

  // Register a voice variant BEFORE begin().
  //
  // "m3" is a neutral mid-range MALE variant, chosen to pair with a UK-male
  // Piper library (en_GB-alan-medium). To use a different voice: pick another
  // file from data/voices/!v/ (e.g. m1,m2,m4..m8, david, paul, robert,
  // edward) and update all three references (path, both symbols, setVoice).
  espeak.add("/mem/data/voices/!v/m3",
             espeak_ng_data_voices__v_m3,
             espeak_ng_data_voices__v_m3_len);

  if (!espeak.begin())
  {
    Serial.println("eSpeak: begin() failed - fallback disabled");
    g_espeakReady = false;
    return;
  }
  // British base voice "en" (already male) + the m3 variant.
  espeak.setVoice("en+m3");
  espeak.setRate(g_espeakWpm);

  // Cache eSpeak's native output format; 
  // Note: the whole project expects 22050 Hz.
  audio_info ai = espeak.audioInfo();
  g_espeakRate = ai.sample_rate;
  Serial.printf("eSpeak ready: %d Hz, %d-bit, %d ch\n",
                ai.sample_rate, ai.bits_per_sample, ai.channels);
  if (ai.sample_rate != 22050)
    Serial.println("WARNING: eSpeak rate != 22050; recorded WAVs must match it.");

  g_espeakReady = true;
}

// ============================================================
// COMMAND HANDLING
// ============================================================

// printHelp() - print the serial command reference to the console.
void printHelp()
{
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  SAY:<text>    Speak text. Phrase tier -> word tier ->");
  Serial.println("                eSpeak fallback.");
#ifdef ENABLE_ESPRESSIF_TTS
  Serial.println("  SAYCN:<text>  Force Espressif Chinese TTS (whole text).");
#endif
  Serial.println("  SPEAK:<text>  Force eSpeak synthesis of the whole text");
  Serial.println("                (bypasses the recorded library).");
  Serial.println("  PLAY:<path>   Play a WAV file from SD.");
  Serial.println("  VOL:<0-21>    Set playback volume.");
  Serial.println("  RATE:<80-450> Set eSpeak speech rate (words/min).");
  Serial.println("  LIST[:<dir>]  List SD root or a directory.");
  Serial.println("  RELOAD        Re-read /tts/manifest.json.");
  Serial.println("  HELP          Show this help.");
  Serial.println("  STATUS        Report ready + library counts.");
}

// resolveAudioPath() - turn a PLAY: argument into an absolute SD path.
// Rules:
//   "name"            -> "/tts/phrases/name.wav"  (bare name -> phrases dir)
//   "name.wav"        -> "/name.wav"              (has extension -> SD root)
//   "dir/name.wav"    -> "/dir/name.wav"          (contains '/' -> root-rel)
//   "/abs/path.wav"   -> unchanged                (already absolute)
// Returns an empty String if the token is blank.
String resolveAudioPath(const String& token)
{
  String t = token; t.trim();
  if (t.length() == 0) return String();
  if (t.startsWith("/")) return t;
  if (t.indexOf('/') >= 0) return String("/") + t;
  if (t.endsWith(".wav") || t.endsWith(".WAV")) return String("/") + t;
  return String("/tts/phrases/") + t + ".wav";
}

// handleCommand() - parse and dispatch one serial command line.
// The command keyword is matched case-insensitively; arguments keep their
// original case (SD paths and SAY/SPEAK text can be case-sensitive).
void handleCommand(const String& cmd)
{
  // Command keywords are matched case-insensitively: compare against an
  // uppercased copy, but take arguments from the original cmd so that file
  // paths and SAY/SPEAK text keep their original case.
  String u = cmd;
  u.toUpperCase();

  // Clear the error LED at the start of every command; it is re-lit below (or
  // inside say_segments) if this command hits an error.
  ledBlue(false);

  if (u.startsWith("SAY:"))
  {
    String text = cmd.substring(4); text.trim();
    if (text.length() == 0) { ledBlue(true); Serial.println("ERR: empty text"); return; }
    handleSay(text);
  }
#ifdef ENABLE_ESPRESSIF_TTS
  // ---- SAYCN: Espressif Chinese TTS (bypasses the hybrid tiers) ----
  // Synthesises the whole input text as Chinese with esp_tts_bridge and plays
  // it as a single SPEAK segment. Requires UTF-8 Chinese input - the check
  // stops ASCII text from being mispronounced by the Chinese voice - and, at
  // build time, the Espressif_TTS library plus its voice-data partition.
  // The entire block below is compiled out when ENABLE_ESPRESSIF_TTS is not
  // defined; SAYCN then falls through to the "unknown command" error.
  else if (u.startsWith("SAYCN:"))
  {
    String text = cmd.substring(6); text.trim();
    if (text.length() == 0) { ledBlue(true); Serial.println("ERR: empty text"); return; }
    if (!g_espTtsReady) { ledBlue(true); Serial.println("ERR: Espressif TTS not available"); return; }
    if (!espTtsContainsChinese(text))
    {
      ledBlue(true);
      Serial.println("ERR: SAYCN: requires UTF-8 Chinese text");
      return;
    }
    Serial.printf("SAYCN (Espressif): %s\n", text.c_str());
    std::vector<Segment> segs;
    Segment s; s.speak = true; s.espTts = true; s.data = text;
    s.pcm = nullptr; s.pcmLen = 0;
    segs.push_back(s);
    SD_CS_low(); delay(10);
    uint16_t bits = 16, ch = 1;
    uint32_t rate = planPlaybackRate(segs, &ch, &bits);
    SD_CS_high();
    if (!espTtsRenderChinese(segs[0].data, &segs[0].pcm, &segs[0].pcmLen, (int)rate))
    {
      ledBlue(true);
      Serial.println("ERR: Espressif TTS render failed");
      return;
    }
    say_segments(segs);
  }
#endif // ENABLE_ESPRESSIF_TTS
  else if (u.startsWith("SPEAK:"))
  {
    String text = cmd.substring(6); text.trim();
    if (text.length() == 0) { ledBlue(true); Serial.println("ERR: empty text"); return; }
    if (!g_espeakReady) { ledBlue(true); Serial.println("ERR: eSpeak not available"); return; }

    Serial.printf("SPEAK (forced): %s\n", text.c_str());
    std::vector<Segment> segs;
    Segment s; s.speak = true; s.espTts = false; s.data = text;
    s.pcm = nullptr; s.pcmLen = 0;
    segs.push_back(s);
    // SPEAK segments must be rendered to PCM before say_segments() can play
    // them - say_segments() does not synthesize, it only streams.
    renderSpeak(segs[0].data, &segs[0].pcm, &segs[0].pcmLen);
    say_segments(segs);
  }
  else if (u.startsWith("PLAY:"))
  {
    String path = resolveAudioPath(cmd.substring(5));
    if (path.length() == 0) { ledBlue(true); Serial.println("ERR: empty path"); return; }
    Serial.printf("Playing: %s\n", path.c_str());
    // PLAY: is just a one-FILE-segment utterance played through the shared
    // say_segments() path (handles I2S setup, amp mute/unmute and teardown).
    std::vector<Segment> segs;
    Segment s; s.speak = false; s.espTts = false; s.data = path;
    s.pcm = nullptr; s.pcmLen = 0;
    segs.push_back(s);
    say_segments(segs);
  }
  else if (u.startsWith("VOL:"))
  {
    int v = cmd.substring(4).toInt();
    if (v < 0)  v = 0;
    if (v > 21) v = 21;
    rak_audio.setVolume(v);
    Serial.printf("Volume set to %d\n", v);
  }
  else if (u.startsWith("RATE:"))
  {
    int wpm = cmd.substring(5).toInt();
    if (wpm < 80)  wpm = 80;
    if (wpm > 450) wpm = 450;
    g_espeakWpm = wpm;
    if (g_espeakReady && !espeak.setRate(g_espeakWpm))
    {
      ledBlue(true);
      Serial.println("ERR: failed to set eSpeak rate");
      return;
    }
    Serial.printf("eSpeak rate set to %d wpm\n", g_espeakWpm);
  }
  else if (u.startsWith("LIST"))
  {
    // Optional ":dir" argument; default to SD root.
    String dir = "/";
    int colon = cmd.indexOf(':');
    if (colon > 0) { dir = cmd.substring(colon + 1); dir.trim(); if (!dir.length()) dir = "/"; }
    SD_CS_low(); delay(10);
    listDir(dir.c_str(), 1);
    SD_CS_high();
  }
  else if (u == "RELOAD")  { loadLibrary(); }
  else if (u == "HELP")    { printHelp(); }
  else if (u == "STATUS")
  {
    Serial.printf("ready  (phrases=%u, words=%u, source=%s, eSpeak=%s, espTTS=%s, rate=%dwpm)\n",
                  (unsigned)g_phrases.size(), (unsigned)g_words.size(),
                  g_manifestLoaded ? "manifest" : "sd-scan",
                  g_espeakReady ? "on" : "off",
                  g_espTtsReady ? "on" : "off",
                  g_espeakWpm);
    Serial.printf("  amp: left=%s, right=%s%s\n",
                  g_ampLeftOk ? "ok" : "fail",
                  g_ampRightOk ? "ok" : "absent",
                  (g_ampLeftOk && !g_ampRightOk) ? " (left-only playback)" : "");
  }
  else
  {
    ledBlue(true);
    Serial.printf("ERR: unknown command '%s'  (type HELP)\n", cmd.c_str());
  }
}

// ============================================================
// SAY pipeline (phrase -> word -> eSpeak fallback)
// ============================================================

// handleSay() - the heart of the hybrid TTS. Turns free text into audio:
//   1. preprocess  - expand symbols (degC, %, ...) and lowercase
//   2. tokenise    - split into word tokens
//   3. number-expand - "23.5" -> "twenty three point five" (as digit tokens)
//   4. resolve     - walk the word stream, greedily matching the longest
//                    phrase, then a single word; consecutive unresolved
//                    words are coalesced into one eSpeak SPEAK segment
//   5. pre-render  - synthesize every SPEAK segment to a PSRAM PCM buffer
//   6. play        - say_segments() plays the whole list in one I2S session
// Progress is logged to serial at each step.
void handleSay(const String& text)
{
  Serial.printf("SAY input: %s\n", text.c_str());

  // 1. Preprocess (symbol expansion, lowercase).
  String prepped = preprocessText(text);
  Serial.printf("  preprocessed: %s\n", prepped.c_str());

  // 2. Tokenise.
  std::vector<String> tokens = tokenize(prepped);

  // 3. Expand numeric tokens into spoken-form word tokens.
  std::vector<String> words;
  for (const String& tok : tokens)
  {
    bool startsWithDigit = tok.length() > 0 && tok[0] >= '0' && tok[0] <= '9';
    bool startsWithNegativeDigit = tok.length() > 1 && tok[0] == '-'
                                && tok[1] >= '0' && tok[1] <= '9';
    if ((startsWithDigit || startsWithNegativeDigit) && isAsciiToken(tok) && looksLikeNumber(tok))
    {
      std::vector<String> num = expandNumber(tok);
      for (const String& w : num) words.push_back(w);
    }
    else words.push_back(tok);
  }
  Serial.print("  words: ");
  for (size_t i = 0; i < words.size(); i++) { Serial.print(words[i]); Serial.print(' '); }
  Serial.println();

  // 4. Resolve the word stream into an ordered list of segments.
  std::vector<Segment> segs;
  String pendingSpeak;          // accumulates consecutive unresolved words

  // Flush any accumulated unknown words as a single eSpeak SPEAK segment.
  // Coalescing keeps eSpeak's prosody natural ("the quux frobnicator" is
  // spoken as a phrase, not three choppy one-word utterances).
  auto flushSpeak = [&]()
  {
    if (pendingSpeak.length())
    {
      Segment s; s.speak = true; s.espTts = false; s.data = pendingSpeak;
      s.pcm = nullptr; s.pcmLen = 0;
      segs.push_back(s);
      pendingSpeak = "";
    }
  };

  size_t i = 0;
  while (i < words.size())
  {
    // ---- Tier 1: longest phrase match starting at words[i] ----
    size_t bestLen = 0;
    String bestPath;
    for (size_t p = 0; p < g_phrases.size(); p++)
    {
      const Phrase& ph = g_phrases[p];
      size_t plen = ph.words.size();
      if (plen <= bestLen) continue;            // can't beat current best
      if (i + plen > words.size()) continue;    // phrase longer than remaining
      bool match = true;
      for (size_t j = 0; j < plen; j++)
        if (words[i + j] != ph.words[j]) { match = false; break; }
      if (match) { bestLen = plen; bestPath = ph.path; }
    }

    if (bestLen > 0)
    {
      // Phrase hit - emit a FILE segment and skip past the matched words.
      flushSpeak();
      Segment s; s.speak = false; s.espTts = false; s.data = bestPath;
      s.pcm = nullptr; s.pcmLen = 0;
      segs.push_back(s);
      i += bestLen;
    }
    else
    {
      // ---- Tier 2: single recorded word ----
      String wordPath = lookupWord(words[i]);
      if (wordPath.length())
      {
        flushSpeak();
        Segment s; s.speak = false; s.espTts = false; s.data = wordPath;
        s.pcm = nullptr; s.pcmLen = 0;
        segs.push_back(s);
      }
      else
      {
        // ---- Tier 3: unknown - queue for eSpeak synthesis ----
        if (pendingSpeak.length()) pendingSpeak += " ";
        pendingSpeak += words[i];
      }
      i++;
    }
  }
  flushSpeak();   // emit any trailing run of unknown words

  if (segs.empty()) { Serial.println("  nothing to say"); return; }

  // ---- Report the speak vs file for each segment ----
  Serial.printf("  %u segment(s):\n", (unsigned)segs.size());
  for (size_t k = 0; k < segs.size(); k++)
  {
    if (segs[k].speak)
      Serial.printf("    [%s] \"%s\"\n",
                    segs[k].espTts ? "TTS  " : "SPEAK", segs[k].data.c_str());
    else
      Serial.printf("    [FILE ] %s\n", segs[k].data.c_str());
  }

  // 5. Pre-render every SPEAK segment to PCM. Done before playback so the
  //    later I2S session is gapless (no synthesis pause mid-utterance). The
  //    segs vector is not resized past this point, so the pcm pointers stay
  //    valid.
  SD_CS_low(); delay(10);
  uint16_t playBits = 16, playCh = 1;
  uint32_t playRate = planPlaybackRate(segs, &playCh, &playBits);
  SD_CS_high();
  Serial.printf("  playback plan: %u Hz, %u-bit, %u ch\n",
                (unsigned)playRate, (unsigned)playBits, (unsigned)playCh);

  for (size_t k = 0; k < segs.size(); k++)
  {
    if (!segs[k].speak) continue;
    // Espressif Chinese TTS render path. Only reached when a segment was
    // built by the SAYCN handler - i.e. only when ENABLE_ESPRESSIF_TTS is
    // defined. With the toggle disabled this branch is dead code (espTts is
    // never set true) and the espTtsRenderChinese call below resolves to a
    // no-op stub that is never invoked at runtime.
    if (segs[k].espTts)
    {
      if (!g_espTtsReady)
      {
        Serial.printf("  Espressif TTS off - cannot synthesize: %s\n",
                      segs[k].data.c_str());
        continue;
      }
      if (!espTtsRenderChinese(segs[k].data, &segs[k].pcm, &segs[k].pcmLen,
                               (int)playRate))
        Serial.printf("  Espressif TTS render failed: %s\n", segs[k].data.c_str());
    }
    else
    {
      if (!g_espeakReady)
      {
        Serial.printf("  eSpeak off - cannot synthesize: %s\n", segs[k].data.c_str());
        continue;
      }
      renderSpeak(segs[k].data, &segs[k].pcm, &segs[k].pcmLen);
    }
  }

  // 6. Play recorded + synthesized audio back-to-back in one I2S session.
  say_segments(segs);
}

// ============================================================
// eSpeak rendering + PCM playback
// ============================================================

// renderSpeak() - synthesize text with eSpeak into a freshly-allocated PCM
// buffer. eSpeak fills the shared g_scratch buffer via g_pcmSink during the
// blocking say() call; we then copy the result into its own buffer so it
// survives until playback.
//   text:   the words to synthesize
//   outPcm: receives a malloc/ps_malloc'd PCM buffer (caller must free it)
//   outLen: receives the buffer length in bytes
// Returns true on success, false if eSpeak is unavailable, produced nothing,
// or memory allocation failed.
bool renderSpeak(const String& text, uint8_t** outPcm, size_t* outLen)
{
  if (!g_espeakReady || !g_scratch) return false;

  g_scratchLen      = 0;
  g_scratchOverflow = false;
  espeak.say(text.c_str());          // blocking; fills g_scratch via g_pcmSink

  // The scratch buffer is sized for ~20 s of speech; overflow means eSpeak
  // produced an unreasonable amount of audio (e.g. a runaway synthesis bug).
  if (g_scratchOverflow)
    Serial.printf("WARNING: eSpeak output exceeded the %u-byte scratch buffer"
                  " - audio truncated\n", (unsigned)SPEAK_SCRATCH_CAP);

  if (g_scratchLen == 0) return false;

  // Copy out of the shared scratch buffer into a buffer owned by this segment.
  // Try PSRAM, fallback if needed.
  uint8_t* p = (uint8_t*)ps_malloc(g_scratchLen);
  if (!p) p = (uint8_t*)malloc(g_scratchLen);
  if (!p) return false;

  memcpy(p, g_scratch, g_scratchLen);
  *outPcm = p;
  *outLen = g_scratchLen;
  return true;
}

// i2sPackMonoSample() - map one mono sample to the L/R pair for I2S.
// Dual amps: duplicate to both channels. Left-only: audio on L, silence on R.
static void i2sPackMonoSample(int16_t s, int16_t pair[2])
{
  pair[0] = s;
  pair[1] = g_ampRightOk ? s : (int16_t)0;
}

// stream_pcm_to_i2s() - push 16-bit mono PCM to I2S via the RAK Gain() path.
void stream_pcm_to_i2s(const uint8_t* pcm, size_t len)
{
  const int16_t* samples = (const int16_t*)pcm;
  size_t count = len / sizeof(int16_t);
  int16_t pair[2];
  for (size_t i = 0; i < count; i++)
  {
    i2sPackMonoSample(samples[i], pair);
    uint32_t s32 = rak_audio.Gain(pair);
    I2S.write((const uint8_t*)&s32, sizeof(uint32_t));
  }
}

// planPlaybackRate() - keep I2S in stereo slot mode for the RAK Gain() path.
// We still output mono content, but every sample is packed as L/R int16 pair.
uint32_t planPlaybackRate(const std::vector<Segment>& segs, uint16_t* outCh,
                          uint16_t* outBits)
{
  *outCh = 2;
  *outBits = 16;

  for (const auto& s : segs)
  {
    if (!s.speak)
    {
      WavFormat h;
      if (read_format_only(s.data.c_str(), &h))
      {
        *outBits = h.bitsPerSample;
        *outCh = 2;
        return h.sampleRate;
      }
      break;
    }
  }
  return (uint32_t)g_espeakRate;
}

// say_segments() - play a resolved segment list as one continuous utterance.
// This is the single playback path for the whole sketch: SAY:, SPEAK: and
// PLAY: all build a Segment list and call it. It opens ONE I2S session (rate
// taken from the first recorded WAV, or the eSpeak rate if the utterance is
// all-synthesized), streams every segment back-to-back (optionally inserting
// the silence clip between them), then closes I2S and frees the SPEAK
// segments' PCM buffers.
// Takes the vector by reference because it frees each segment's pcm pointer.
void say_segments(std::vector<Segment>& segs)
{
  if (segs.empty()) return;

  SD_CS_low(); delay(10);
  uint16_t bits = 16, ch = 2;
  uint32_t rate = planPlaybackRate(segs, &ch, &bits);
  Serial.printf("  I2S: %u Hz, %u-bit, %u ch\n",
                (unsigned)rate, (unsigned)bits, (unsigned)ch);

  I2S.setPins(WB_IO6, WB_IO5, SDA1, SCL1);
  if (!I2S.begin(I2S_MODE_STD, rate,
                 (i2s_data_bit_width_t)bits, (i2s_slot_mode_t)ch))
  {
    ledBlue(true);
    Serial.println("Failed to initialize I2S!");
    SD_CS_high();
    // I2S failed, but still release any PCM buffers we already rendered.
    for (auto& s : segs) if (s.pcm) { free(s.pcm); s.pcm = nullptr; }
    return;
  }
  rak_audio.calculateIIR_Gain(rate);

  set_AMP_unmute();
  ledGreen(true);                  // GREEN: audio is playing
  for (size_t k = 0; k < segs.size(); k++)
  {
    // Optional short gap between segments for clarity.
    if (k > 0 && g_silenceAvailable) stream_wav_to_i2s(SILENCE_FILE, ch);

    if (segs[k].speak)
    {
      if (segs[k].pcm && segs[k].pcmLen)
        stream_pcm_to_i2s(segs[k].pcm, segs[k].pcmLen);
      else
      { 
        ledBlue(true); // error
        Serial.printf("  (no audio for SPEAK \"%s\")\n", segs[k].data.c_str()); 
      }
    }
    else // play wav file
    {
      if (!stream_wav_to_i2s(segs[k].data.c_str(), ch))
      { 
        ledBlue(true); //error 
        Serial.printf("  skip (open failed): %s\n", segs[k].data.c_str()); 
      }
    }
  }
  delay(100);              // let the I2S DMA drain before tearing down
  ledGreen(false);                 // playback finished
  set_AMP_mute();
  I2S.end();
  SD_CS_high();

  // Release the per-segment PCM buffers allocated by renderSpeak().
  for (auto& s : segs) if (s.pcm) { free(s.pcm); s.pcm = nullptr; }
}

// ============================================================
// Library loading (manifest preferred, SD scan fallback)
// ============================================================

// loadLibrary() - (re)build the phrase and word indexes. Prefers the fast
// path of parsing /tts/manifest.json; if that is missing or empty it falls
// back to scanning the /tts/phrases directory. Called at boot and by RELOAD.
void loadLibrary()
{
  g_phrases.clear();
  g_words.clear();
  g_manifestLoaded = false;

  SD_CS_low(); delay(10);
  bool haveManifest = SD.exists(MANIFEST_FILE);
  SD_CS_high();

  if (haveManifest) loadManifest();
  if (!g_manifestLoaded)
  {
    Serial.println("Manifest unavailable - falling back to SD directory scan.");
    loadPhrasesFromDir();
  }

  Serial.printf("Library ready: %u phrase(s), %u word key(s)  [source: %s]\n",
                (unsigned)g_phrases.size(), (unsigned)g_words.size(),
                g_manifestLoaded ? "manifest" : "sd-scan");
}

// loadManifest() - read /tts/manifest.json into RAM and parse it. The whole
// file is slurped into a PSRAM buffer (manifests can be tens of KB), parsed
// by parseManifest(), then the buffer is freed. Sets g_manifestLoaded if any
// phrase or word entries were produced.
void loadManifest()
{
  SD_CS_low(); delay(10);
  File f = SD.open(MANIFEST_FILE);
  if (!f) { Serial.printf("Cannot open %s\n", MANIFEST_FILE); SD_CS_high(); return; }
  size_t sz = f.size();
  if (sz == 0) { Serial.println("Manifest is empty"); f.close(); SD_CS_high(); return; }

  // Slurp the whole file (+1 for a NUL terminator). Prefer PSRAM.
  char* buf = (char*)ps_malloc(sz + 1);
  if (!buf) buf = (char*)malloc(sz + 1);
  if (!buf) { Serial.println("Manifest: out of memory"); f.close(); SD_CS_high(); return; }

  size_t got = 0;
  while (got < sz)
  {
    int n = f.read((uint8_t*)buf + got, sz - got);
    if (n <= 0) break;
    got += n;
  }
  buf[got] = 0;
  f.close();
  SD_CS_high();

  Serial.printf("Manifest read: %u bytes\n", (unsigned)got);
  parseManifest(buf, got);
  free(buf);
  g_manifestLoaded = (!g_phrases.empty() || !g_words.empty());
}

// extractIntAfter() - find "key" in buf and return the integer value that
// follows the next ':'. Returns -1 if the key or a number is not found.
// Used to read the manifest's "format" block (sample_rate / bits / channels).
static long extractIntAfter(const char* buf, const char* key)
{
  const char* p = strstr(buf, key);
  if (!p) return -1;
  p += strlen(key);
  while (*p && *p != ':') p++;
  if (!*p) return -1;
  p++;
  while (*p && isspace((unsigned char)*p)) p++;
  bool neg = false;
  if (*p == '-') { neg = true; p++; }
  if (!isdigit((unsigned char)*p)) return -1;
  long v = 0;
  while (isdigit((unsigned char)*p)) { v = v * 10 + (*p - '0'); p++; }
  return neg ? -v : v;
}

// quotedValueAfterColon() - starting at p, skip to ':', skip whitespace, then
// read the following "quoted string" into out (handles \-escapes minimally).
// Returns a pointer just past the closing quote, or NULL on malformed input.
// This is the primitive the lightweight manifest parser is built from.
static const char* quotedValueAfterColon(const char* p, String& out)
{
  while (*p && *p != ':') p++;
  if (!*p) return NULL;
  p++;
  while (*p && isspace((unsigned char)*p)) p++;
  if (*p != '"') return NULL;
  p++;
  out = "";
  while (*p && *p != '"')
  {
    if (*p == '\\' && *(p + 1)) p++;
    out += *p;
    p++;
  }
  if (*p != '"') return NULL;
  return p + 1;
}

// parseRegion() - scan the manifest text in [start,end) for "text"/"file"
// pairs and add each one to the phrase or word index. isPhrase selects which.
// This is a deliberately format-agnostic scan (no JSON library) - it works
// whether the manifest is pretty-printed or minified.
static void parseRegion(const char* start, const char* end, bool isPhrase)
{
  const char* cur = start;
  while (cur && cur < end)
  {
    const char* t = strstr(cur, "\"text\"");
    if (!t || t >= end) break;
    String textVal;
    const char* a1 = quotedValueAfterColon(t + 6, textVal);
    if (!a1) break;
    const char* fk = strstr(a1, "\"file\"");
    if (!fk || fk >= end) break;
    String fileVal;
    const char* a2 = quotedValueAfterColon(fk + 6, fileVal);
    if (!a2) break;
    if (isPhrase) addPhraseEntry(textVal, fileVal);
    else          addWordEntry(textVal, fileVal);
    cur = a2;
  }
}

// parseManifest() - parse the in-RAM manifest text. Logs the declared audio
// format (warning if it is not 22050 Hz), then parses the "phrases" array
// region and the "words" array region into g_phrases / g_words.
// buf must be NUL-terminated; len is its length.
void parseManifest(char* buf, size_t len)
{
  long sr = extractIntAfter(buf, "\"sample_rate\"");
  long bt = extractIntAfter(buf, "\"bits\"");
  long ch = extractIntAfter(buf, "\"channels\"");
  if (sr > 0)
  {
    Serial.printf("Manifest format: %ld Hz, %ld-bit, %ld ch\n", sr, bt, ch);
    if (sr != 22050)
      Serial.println("WARNING: manifest sample_rate != 22050 - regenerate WAVs.");
  }

  // The "phrases" array runs from its key up to the "words" key; "words"
  // runs from its key to end-of-buffer.
  char* phr = strstr(buf, "\"phrases\"");
  char* wrd = strstr(buf, "\"words\"");
  char* end = buf + len;
  if (phr) parseRegion(phr, wrd ? wrd : end, true);
  if (wrd) parseRegion(wrd, end, false);

  Serial.printf("Parsed manifest: %u phrase(s), %u word key(s)\n",
                (unsigned)g_phrases.size(), (unsigned)g_words.size());
}

// addPhraseEntry() - add one phrase to g_phrases. The text is lowercased and
// split into word tokens for matching. Duplicate paths are ignored, so the
// manifest may safely list the same phrase twice.
void addPhraseEntry(const String& text, const String& path)
{
  if (path.length() == 0) return;
  for (size_t i = 0; i < g_phrases.size(); i++)
    if (g_phrases[i].path == path) return;          // de-dupe by path
  String t = text; t.toLowerCase(); t.trim();
  Phrase p;
  p.words = splitOnChar(t, ' ');
  if (p.words.empty()) return;
  p.path = path;
  g_phrases.push_back(p);
}

// addWordEntry() - add one word to g_words under TWO lowercase keys:
//   - the spoken text          (e.g. "twenty")
//   - the file's stem          (e.g. "20" from ".../numbers/20.wav")
// The dual key lets number-expander output (digit tokens like "20") resolve
// directly. map::emplace keeps the first value for a key, so duplicate
// manifest entries are harmless.
void addWordEntry(const String& text, const String& path)
{
  if (path.length() == 0) return;

  // Key 1: the spoken text.
  String key = text; key.toLowerCase(); key.trim();
  if (key.length()) g_words.emplace(key, path);

  // Key 2: the filename stem (path basename minus ".wav").
  String base = path;
  int s = base.lastIndexOf('/');
  if (s >= 0) base = base.substring(s + 1);
  String low = base; low.toLowerCase();
  if (low.endsWith(".wav")) base = base.substring(0, base.length() - 4);
  base.toLowerCase();
  if (base.length()) g_words.emplace(base, path);
}

// splitOnChar() - split s on the separator char into a vector of non-empty
// substrings (empty fields are dropped). Used to tokenise phrase text.
std::vector<String> splitOnChar(const String& s, char sep)
{
  std::vector<String> out;
  int start = 0;
  while (start <= (int)s.length())
  {
    int idx = s.indexOf(sep, start);
    if (idx < 0)
    {
      if (start < (int)s.length()) out.push_back(s.substring(start));
      break;
    }
    if (idx > start) out.push_back(s.substring(start, idx));
    start = idx + 1;
  }
  return out;
}

// loadPhrasesFromDir() - fallback phrase loader used when no manifest exists.
// Scans /tts/phrases for *.wav files and derives each phrase's text from the
// filename, treating '_' as a word separator (battery_is_low.wav -> "battery
// is low"). Does NOT populate the word index - without a manifest, words are
// resolved live by findWordFile().
void loadPhrasesFromDir()
{
  SD_CS_low(); delay(10);
  File dir = SD.open(PHRASES_DIR);
  if (!dir || !dir.isDirectory())
  {
    Serial.printf("Phrases dir not found: %s\n", PHRASES_DIR);
    SD_CS_high();
    return;
  }
  File f = dir.openNextFile();
  while (f)
  {
    if (!f.isDirectory())
    {
      String name = f.name();
      int slash = name.lastIndexOf('/');
      String b = (slash >= 0) ? name.substring(slash + 1) : name;  // basename
      if (b.length() > 0 && b[0] != '.')                           // skip hidden
      {
        String lower = b; lower.toLowerCase();
        if (lower.endsWith(".wav"))
        {
          String stem = b.substring(0, b.length() - 4);
          stem.toLowerCase();
          stem.replace('_', ' ');
          addPhraseEntry(stem, String(PHRASES_DIR) + "/" + b);
        }
      }
    }
    f = dir.openNextFile();
  }
  SD_CS_high();
}

// findWordFile() - fallback word lookup (no manifest). Searches each folder
// in WORD_FOLDERS for "<word>.wav" and returns the first hit, or an empty
// String. Slower than the manifest map (one SD.exists() per folder).
String findWordFile(const String& word)
{
  for (size_t i = 0; i < WORD_FOLDERS_N; i++)
  {
    String path = String(WORD_FOLDERS[i]) + word + ".wav";
    if (SD.exists(path.c_str())) return path;
  }
  return String();
}

// lookupWord() - resolve one word token to a recorded WAV path.
// Uses the in-memory g_words map when a manifest is loaded (fast, authoritative);
// otherwise falls back to scanning WORD_FOLDERS on the SD card.
// Returns an empty String if the word has no recording (-> eSpeak fallback).
String lookupWord(const String& wordIn)
{
  String w = wordIn; w.toLowerCase();
  if (g_manifestLoaded)
  {
    auto it = g_words.find(w);
    if (it != g_words.end()) return it->second;
    return String();
  }
  return findWordFile(w);
}

// ============================================================
// Text preprocessing / numbers
// ============================================================

// replaceAll() - replace every occurrence of 'needle' in s with replacement
// (in place). Helper for preprocessText().
static void replaceAll(String& s, const char* needle, const char* replacement)
{
  int idx = 0;
  int nlen = strlen(needle);
  while ((idx = s.indexOf(needle, idx)) >= 0)
  {
    s = s.substring(0, idx) + replacement + s.substring(idx + nlen);
    idx += strlen(replacement);
  }
}

static void asciiToLowerInPlace(String& s)
{
  for (size_t i = 0; i < s.length(); i++)
  {
    char c = s[i];
    if (c >= 'A' && c <= 'Z') s.setCharAt(i, c + ('a' - 'A'));
  }
}

// preprocessText() - normalise input text before tokenising: spell out common
// symbols (degC -> "degrees celsius", % -> "percent", ...), turn punctuation
// into spaces, and lowercase everything. Returns the cleaned string.
String preprocessText(const String& in)
{
  String s = in;
  // "\xC2\xB0" is the UTF-8 encoding of the degree sign.
  replaceAll(s, "\xC2\xB0" "C", " degrees celsius ");
  replaceAll(s, "\xC2\xB0" "F", " degrees fahrenheit ");
  replaceAll(s, "\xC2\xB0",     " degrees ");
  replaceAll(s, "%",  " percent ");
  replaceAll(s, "+",  " plus ");
  replaceAll(s, "=",  " equals ");
  replaceAll(s, "&",  " and ");
  replaceAll(s, "/",  " slash ");
  replaceAll(s, ":",  " ");
  replaceAll(s, ";",  " ");
  replaceAll(s, ",",  " ");
  asciiToLowerInPlace(s);
  return s;
}

// tokenize() - split preprocessed text into word tokens. A token is a run of
// letters/digits plus '-' and decimal '.' (so "23.5" and "-12" stay intact).
// Sentence punctuation is a separator, even when the next sentence has no space.
std::vector<String> tokenize(const String& text)
{
  std::vector<String> out;
  String cur;
  for (size_t i = 0; i < text.length(); i++)
  {
    unsigned char c = (unsigned char)text[i];
    // Keep UTF-8 bytes (>=0x80) so Chinese text survives tokenization for SAY:.
    bool isAsciiAlphaNum = (c >= '0' && c <= '9')
                        || (c >= 'a' && c <= 'z')
                        || (c >= 'A' && c <= 'Z');
    bool prevIsDigit = i > 0 && text[i - 1] >= '0' && text[i - 1] <= '9';
    bool nextIsDigit = (i + 1) < text.length() && text[i + 1] >= '0' && text[i + 1] <= '9';
    bool isDecimalPoint = c == '.' && prevIsDigit && nextIsDigit;
    bool isTokenChar = isAsciiAlphaNum || isDecimalPoint || c == '-' || c >= 0x80;
    if (isTokenChar) cur += (char)c;
    else if (cur.length() > 0) { out.push_back(cur); cur = ""; }
  }
  if (cur.length() > 0) out.push_back(cur);

  // Trim punctuation that is not part of a number (e.g. a trailing '.').
  for (size_t i = 0; i < out.size(); i++)
  {
    String& t = out[i];
    while (t.length() > 0 && (t[t.length()-1] == '.' || t[t.length()-1] == '-'))
      t.remove(t.length() - 1);
    while (t.length() > 0 && t[0] == '.') t.remove(0, 1);
  }
  std::vector<String> cleaned;
  for (auto& t : out) if (t.length() > 0) cleaned.push_back(t);
  return cleaned;
}

bool isAsciiToken(const String& s)
{
  for (size_t i = 0; i < s.length(); i++)
    if ((unsigned char)s[i] >= 0x80) return false;
  return true;
}

// looksLikeNumber() - true if s is an integer or decimal, optionally signed
// (e.g. "12", "-3", "23.5"). Used to decide whether to run expandNumber().
bool looksLikeNumber(const String& s)
{
  if (s.length() == 0) return false;
  size_t i = 0;
  if (s[0] == '-') i = 1;
  if (i >= s.length()) return false;
  bool sawDigit = false, sawDot = false;
  for (; i < s.length(); i++)
  {
    unsigned char c = (unsigned char)s[i];
    // Strict ASCII numeric parser: reject any UTF-8 byte (>= 0x80).
    if (c >= (unsigned char)'0' && c <= (unsigned char)'9') sawDigit = true;
    else if (c == '.' && !sawDot)  sawDot = true;   // at most one '.'
    else return false;
  }
  return sawDigit;
}

// expandNumber() - expand a numeric string into spoken-form word tokens.
// The integer part goes through expandInteger(); a decimal part is spoken
// digit-by-digit after "point". Example: "-23.5" -> negative twenty three
// point five. Tokens are the same ones used as word-bank keys ("twenty",
// "three", "point", "five", or digit stems like "20").
std::vector<String> expandNumber(const String& num)
{
  std::vector<String> out;
  String s = num;
  if (s.startsWith("-")) { out.push_back("negative"); s = s.substring(1); }
  int dot = s.indexOf('.');
  String intPart  = (dot >= 0) ? s.substring(0, dot) : s;
  String fracPart = (dot >= 0) ? s.substring(dot + 1) : String();
  if (intPart.length() == 0) intPart = "0";
  long n = intPart.toInt();
  expandInteger(n, out);
  if (fracPart.length() > 0)
  {
    out.push_back("point");
    for (size_t i = 0; i < fracPart.length(); i++)
      out.push_back(String(fracPart[i]));           // each digit individually
  }
  return out;
}

// expandInteger() - append the spoken-form tokens for integer n to out.
// Handles 0 .. 999,999 by recursion (thousands), then hundreds, then the
// 0-20 / tens+ones range. Token values match the word-bank number files
// (e.g. "0".."20", "30","40",..., "hundred", "thousand").
void expandInteger(long n, std::vector<String>& out)
{
  if (n == 0) { out.push_back("0"); return; }
  if (n < 0)  { out.push_back("negative"); n = -n; }
  if (n >= 1000)
  {
    long t = n / 1000;
    expandInteger(t, out);            // recurse for the thousands group
    out.push_back("thousand");
    n %= 1000;
    if (n == 0) return;
  }
  if (n >= 100)
  {
    long h = n / 100;
    out.push_back(String(h));         // 1..9
    out.push_back("hundred");
    n %= 100;
    if (n == 0) return;
  }
  if (n <= 20) { out.push_back(String(n)); return; }   // 1..20 spoken directly
  long tens = (n / 10) * 10;          // 20,30,...,90
  long ones = n % 10;
  out.push_back(String(tens));
  if (ones > 0) out.push_back(String(ones));
}

// ============================================================
// WAV streaming / SD / amp init
//
// read_format_only() and stream_wav_to_i2s() are the WAV helpers behind
// say_segments(); everything below them is hardware bring-up adapted from the
// RAKwireless PlayBackFromSD example.
// ============================================================

// read_format_only() - open a WAV, read just its format fields (channels,
// sampleRate, bitsPerSample) into *out, and close it. Returns false if the
// file does not exist or cannot be opened.
bool read_format_only(const char* path, WavFormat* out)
{
  if (!SD.exists(path)) return false;
  File f = SD.open(path);
  if (!f) return false;
  uint8_t hdr[WAV_HEAD_LEN] = {0};
  f.read(hdr, sizeof(hdr));
  f.close();
  // Standard WAV header field offsets (little-endian).
  out->channels      = (hdr[23] << 8) | hdr[22];
  out->sampleRate    = ((uint32_t)hdr[27] << 24) | ((uint32_t)hdr[26] << 16)
                       | ((uint32_t)hdr[25] <<  8) |  (uint32_t)hdr[24];
  out->bitsPerSample = (hdr[35] << 8) | hdr[34];
  return true;
}

// stream_wav_to_i2s() - open a WAV, seek to its "data" chunk, and stream the
// PCM to the already-running I2S until end-of-file. When i2sSlots==2 and the
// WAV is mono, each sample is duplicated to L/R (RAK PlayBack22K convention).
bool stream_wav_to_i2s(const char* path, uint16_t i2sSlots)
{
  if (!SD.exists(path)) return false;
  File f = SD.open(path);
  if (!f) return false;

  uint8_t hdr[WAV_HEAD_LEN] = {0};
  f.read(hdr, sizeof(hdr));
  uint16_t wavCh = (hdr[23] << 8) | hdr[22];
  bool dupMono = (i2sSlots == 2 && wavCh == 1);

  uint32_t dataStart = 44;
  for (uint16_t i = 12; i < WAV_HEAD_LEN - 4; i++)
  {
    if (hdr[i]=='d' && hdr[i+1]=='a' && hdr[i+2]=='t' && hdr[i+3]=='a')
    { dataStart = i + 8; break; }
  }
  f.seek(dataStart);

  int16_t sample[2] = {0};
  uint32_t fileEnd = f.size();
  if (dupMono)
  {
    while (f.position() + 1 < fileEnd)
    {
      int n = f.read((uint8_t*)&sample[0], sizeof(int16_t));
      if (n < (int)sizeof(int16_t)) break;
      i2sPackMonoSample(sample[0], sample);
      uint32_t s32 = rak_audio.Gain(sample);
      I2S.write((const uint8_t*)&s32, sizeof(uint32_t));
    }
  }
  else
  {
    while (f.position() < fileEnd)
    {
      int n = f.read((uint8_t*)&sample, sizeof(sample));
      if (n <= 0) break;
      if (!g_ampRightOk) sample[1] = 0;
      uint32_t s32 = rak_audio.Gain(sample);
      I2S.write((const uint8_t*)&s32, sizeof(uint32_t));
    }
  }
  f.close();
  return true;
}

// AMP_init() - bring up RAK18060 TAS2560 amplifiers. Right chip is optional
// when only the left speaker is populated (0x4f will not respond).
void AMP_init(void)
{
  g_ampLeftOk  = AMP_Left.begin(AMP_LEFT_ADDRESS);
  g_ampRightOk = AMP_Right.begin(AMP_RIGTT_ADDRESS);

  if (g_ampLeftOk)
  {
    AMP_Left.set_pcm_channel(LeftMode);
    Serial.println("TAS2560 left init OK");
  }
  else
  {
    ledBlue(true);
    Serial.println("TAS2560 left init failed - no playback");
    delay(500);
  }

  if (g_ampRightOk)
  {
    AMP_Right.set_pcm_channel(RightMode);
    Serial.println("TAS2560 right init OK");
  }
  else
  {
    Serial.println("TAS2560 right init skipped (left-only speaker is OK)");
  }
}

// set_AMP_mute() / set_AMP_unmute() - mute or unmute present amplifiers only.
void set_AMP_mute()
{
  if (g_ampLeftOk)  AMP_Left.set_mute();
  if (g_ampRightOk) AMP_Right.set_mute();
}
void set_AMP_unmute()
{
  if (g_ampLeftOk)  AMP_Left.set_unmute();
  if (g_ampRightOk) AMP_Right.set_unmute();
}

// ledGreen() / ledBlue() - status LEDs (both active-HIGH on the RAK3112).
// Convention: GREEN is lit while audio is playing; BLUE is lit when the most
// recent command or operation hit an error (cleared at the start of the next
// command). setup() configures the pins before these are used.
void ledGreen(bool on) { digitalWrite(LED_GREEN, on ? HIGH : LOW); }
void ledBlue(bool on)  { digitalWrite(LED_BLUE,  on ? HIGH : LOW); }

// SD_init() - mount the microSD card (which lives on the RAK18003 interposer)
// and, on success, print a one-level listing of the card root.
void SD_init(void)
{
  SD_CS_low();
  if (!SD.begin(SS, SPI, 8000000, "/sd", 5)) { ledBlue(true); Serial.println("Card Mount Failed"); }
  else listDir("/", 0);
  SD_CS_high();
}

// RAK18003Init() - initialise the two I2C IO-expanders on the RAK18003 audio
// interposer and configure their pin directions. Blocks (flashing the LEDs)
// until both expanders respond, and also blocks until the RAK18060 amplifier
// board and an SD card are both detected - i.e. it will not return until the
// audio hardware is physically present and wired correctly.
void RAK18003Init(void)
{
  while (!Expander1.begin())
  {
    Serial.println("Did not find RAK18003 IO Expander Chip1,please check!");
    digitalWrite(LED_BLUE, HIGH); digitalWrite(LED_GREEN, HIGH); delay(200);
    digitalWrite(LED_BLUE, LOW);  digitalWrite(LED_GREEN, LOW);  delay(200);
  }
  while (!Expander2.begin())
  {
    Serial.println("Did not find RAK18003 IO Expander Chip2,please check!");
    digitalWrite(LED_BLUE, HIGH); digitalWrite(LED_GREEN, HIGH); delay(200);
    digitalWrite(LED_BLUE, LOW);  digitalWrite(LED_GREEN, LOW);  delay(200);
  }
  // Expander1: mixed in/out lines for the mic / amp / DSP module slots.
  Expander1.pinMode(0,  INPUT);  Expander1.pinMode(1,  INPUT);
  Expander1.pinMode(2,  OUTPUT); Expander1.pinMode(3,  OUTPUT);
  Expander1.pinMode(4,  INPUT);  Expander1.pinMode(5,  INPUT);
  Expander1.pinMode(6,  OUTPUT); Expander1.pinMode(7,  OUTPUT);
  Expander1.pinMode(8,  INPUT);  Expander1.pinMode(9,  INPUT);
  Expander1.pinMode(10, INPUT);  Expander1.pinMode(11, OUTPUT);
  Expander1.pinMode(12, OUTPUT); Expander1.pinMode(13, OUTPUT);
  Expander1.pinMode(14, OUTPUT); Expander1.pinMode(15, OUTPUT);

  // Expander2: all outputs; pin 2 is the SD-card chip-select (see SD_CS_*).
  for (int i = 0; i < 16; i++) Expander2.pinMode(i, OUTPUT);
  Expander2.digitalWrite(3, 0);

  while (Expander1.digitalRead(4) == 0) { Serial.println("There is no RAK18060 AMP board, please check !"); delay(200); }
  while (Expander1.digitalRead(0) == 1) { Serial.println("There is no SD card on the RAK18003 board, please check !"); delay(200); }
}

// SD_CS_high() / SD_CS_low() - drive the microSD chip-select. On this board
// the SD CS is not a CPU GPIO - it is pin 2 of Expander2 on the RAK18003.
// CS must be LOW (asserted) for any SD access and is raised between accesses.
void SD_CS_high() { Expander2.digitalWrite(2, 1); }
void SD_CS_low()  { Expander2.digitalWrite(2, 0); }

// listDir() - print a directory listing to serial. Recurses up to `levels`
// deep into sub-directories. Hidden entries (leading '.') and the Windows
// "System Volume Information" folder are skipped.
void listDir(const char* dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);
  File root = SD.open(dirname);
  if (!root) { Serial.println("Failed to open directory"); return; }
  if (!root.isDirectory()) { Serial.println("Not a directory"); return; }

  File file = root.openNextFile();
  while (file)
  {
    const char* name = file.name();
    if (name[0] == '.' || strcmp(name, "System Volume Information") == 0)
    {
      file = root.openNextFile();
      continue;
    }
    if (file.isDirectory())
    {
      Serial.print("  DIR : "); Serial.println(name);
      if (levels)
      {
        // file.name() may be a bare name or a full path depending on the SD
        // library version; build a usable child path either way.
        String childPath;
        if (name[0] == '/') childPath = name;
        else { childPath = String(dirname); if (!childPath.endsWith("/")) childPath += "/"; childPath += name; }
        listDir(childPath.c_str(), levels - 1);
      }
    }
    else
    {
      Serial.print("File: "); Serial.print(name);
      Serial.print("  Size: "); Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
