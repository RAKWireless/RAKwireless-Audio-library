#include "audio.h"

const uint8_t volumetable[22]={   0,  1,  2,  3,  4 , 6 , 8, 10, 12, 14, 17,
								20, 23, 27, 30 ,34, 38, 43 ,48, 52, 58, 64}; //22 elements
// float           _filterBuff[3][2][2][2];       // IIR filters memory for Audio DSP
// filter_t        _filter[3];                    // digital filters

Audio::Audio() {
	_balance = 0;                  // -16 (mute left) ... +16 (mute right)
	_volume = 64;                       // volume
}
void Audio::begin(void) {
	init_flash();
}
void Audio::GetBoardId(char *array){

#ifdef _VARIANT_RAK4630_
    static char szUUID[17];
	int *p1 ,*p2;
	p1 = (int *)ID1;
	p2 = (int *)ID2;
	for(int i =0;i<17;i++)
		szUUID[i] = 0;
	sprintf(szUUID,"%4X%4X",*p1,*p2);
	memcpy(array,szUUID,16);
#elif defined (_VARIANT_RAK11300_)
    static char szUUID[64];
	int i;
	uint32_t ints = save_and_disable_interrupts();
	uint8_t n = getUniqueSerialNumber((uint8_t *)szUUID);

	for (i = 0; i < n / 2; i++)
		szUUID[i] = szUUID[2 * i];
	szUUID[i] = 0;
	restore_interrupts(ints);
    memcpy(array,szUUID,16);
#elif defined (_VARIANT_RAK11200_)
    static char szUUID[17];
	int i;
	uint8_t id[8];
	uint32_t nMacLow;
	uint32_t nMacHigh;
	
	for(int i=0;i<17;i++)
		szUUID[i] = 0;
	for(int i=0;i<8;i++)
		id[i] = 0;
	
	// Using ESP32 MAC (48 bytes only, so 2 bytes will be 0)
	nMacLow = (*(volatile uint32_t*)(BLK0_RDATA1_REG));
    nMacHigh = (*(volatile uint32_t*)(BLK0_RDATA2_REG));
	id[7] = 0;
	id[6] = 0;
	id[5] = (uint8_t)(nMacLow);
	id[4] = (uint8_t)(nMacLow >> 8);
	id[3] = (uint8_t)(nMacLow >> 16);
	id[2] = (uint8_t)(nMacLow >> 24);
	id[1] = (uint8_t)(nMacHigh);
	id[0] = (uint8_t)(nMacHigh >> 8);
	
	sprintf(szUUID,"%02X%02X%02X%02X%02X%02X%02X%02X",id[0],id[1],id[2],id[3],id[4],id[5],id[6],id[7]);
    memcpy(array,szUUID,16);  
#endif
    
}

void Audio::StoreLicense(uint32_t *lic){
	flash_write(CYBERON_FILEPATH, lic, LICEENSE_LENGTH*4);
}

uint32_t* Audio::GetLicenseAddr(void){
    uint32_t tmp[46];
	flash_read(CYBERON_FILEPATH, tmp, LICEENSE_LENGTH*4);
	memcpy(this->Cyberon_License,tmp,LICEENSE_LENGTH*4);
	return this->Cyberon_License;
}

void Audio::GetLicense(uint32_t *lic){
	flash_read(CYBERON_FILEPATH, lic, LICEENSE_LENGTH*4);
}


void Audio::setBalance(int8_t bal){ // bal -16...16
    if(bal < -16) bal = -16;
    if(bal >  16) bal =  16;
    _balance = bal;
}
void Audio::setVolume(uint8_t vol) { // vol 22 steps, 0...21
    if(vol > 21) vol = 21;
    _volume = volumetable[vol];
}
uint8_t Audio::getVolume() {
    for(uint8_t i = 0; i < 22; i++) {
        if(volumetable[i] == _volume) return i;
    }
    _volume = 12; // if _volume not found in table
    return _volume;
}
int32_t Audio::Gain(int16_t s[2]) {
    int32_t v[2];
    float step = (float)_volume /64;
    uint8_t l = 0, r = 0;

    if(_balance < 0){
        step = step * (float)(abs(_balance) * 4);
        l = (uint8_t)(step);
    }
    if(_balance > 0){
        step = step * _balance * 4;
        r = (uint8_t)(step);
    }

    v[LEFTCHANNEL] = (s[LEFTCHANNEL]  * (_volume - l)) >> 6;
    v[RIGHTCHANNEL]= (s[RIGHTCHANNEL] * (_volume - r)) >> 6;

    return (v[LEFTCHANNEL] << 16) | (v[RIGHTCHANNEL] & 0xffff);
}
int32_t Audio::Gain(uint32_t data) {

	int16_t s[2];	
    int32_t v[2];
    float step = (float)_volume /64;
    uint8_t l = 0, r = 0;

	s[0] = data&0xffff;
	s[1] = (data>>16)&0xffff;

    if(_balance < 0){
        step = step * (float)(abs(_balance) * 4);
        l = (uint8_t)(step);
    }
    if(_balance > 0){
        step = step * _balance * 4;
        r = (uint8_t)(step);
    }

    v[LEFTCHANNEL] = (s[LEFTCHANNEL]  * (_volume - l)) >> 6;
    v[RIGHTCHANNEL]= (s[RIGHTCHANNEL] * (_volume - r)) >> 6;

    return (v[LEFTCHANNEL] << 16) | (v[RIGHTCHANNEL] & 0xffff);
}

int Audio::calculateIIR_Gain(uint32_t sampleRate)
{
	IIR_calculateCoefficients(sampleRate,_gain0, _gain1, _gain2); // must be recalculated after each samplerate change

	return 0;
}

// bool Audio::playSample(int16_t sample[2]) {

//     if (getBitsPerSample() == 8) { // Upsample from unsigned 8 bits to signed 16 bits
//         sample[LEFTCHANNEL]  = ((sample[LEFTCHANNEL]  & 0xff) -128) << 8;
//         sample[RIGHTCHANNEL] = ((sample[RIGHTCHANNEL] & 0xff) -128) << 8;
//     }

//     sample[LEFTCHANNEL]  = sample[LEFTCHANNEL]  >> 1; // half Vin so we can boost up to 6dB in filters
//     sample[RIGHTCHANNEL] = sample[RIGHTCHANNEL] >> 1;

//     // Filterchain, can commented out if not used
//     sample = IIR_filterChain0(sample);
//     sample = IIR_filterChain1(sample);
//     sample = IIR_filterChain2(sample);
//     //-------------------------------------------

//     uint32_t s32 = Gain(sample); // vosample2lume;
 
// IIR_calculateCoefficients(m_gain0, m_gain1, m_gain2); // must be recalculated after each samplerate change

//     return true;
// }

// void Audio::setTone(int8_t gainLowPass, int8_t gainBandPass, int8_t gainHighPass){
//     // see https://www.earlevel.com/main/2013/10/13/biquad-calculator-v2/
//     // values can be between -40 ... +6 (dB)

//     m_gain0 = gainLowPass;
//     m_gain1 = gainBandPass;
//     m_gain2 = gainHighPass;

//     IIR_calculateCoefficients(m_gain0, m_gain1, m_gain2);

//     /*
//         This will cause a clicking sound when adjusting the EQ.
//         Because when the EQ is adjusted, the IIR filter will be cleared and played,
//         mixed in the audio data frame, and a click-like sound will be produced.
//     */
//     /*
//     int16_t tmp[2]; tmp[0] = 0; tmp[1]= 0;

//     IIR_filterChain0(tmp, true ); // flush the filter
//     IIR_filterChain1(tmp, true ); // flush the filter
//     IIR_filterChain2(tmp, true ); // flush the filter
//     */
// }
//---------------------------------------------------------------------------------------------------------------------
//            ***     D i g i t a l   b i q u a d r a t i c     f i l t e r     ***
//---------------------------------------------------------------------------------------------------------------------
void Audio::IIR_calculateCoefficients(uint32_t sampleRate,int8_t G0, int8_t G1, int8_t G2){  // Infinite Impulse Response (IIR) filters

    // G1 - gain low shelf   set between -40 ... +6 dB
    // G2 - gain peakEQ      set between -40 ... +6 dB
    // G3 - gain high shelf  set between -40 ... +6 dB
    // https://www.earlevel.com/main/2012/11/26/biquad-c-source-code/

    if(sampleRate < 1000) return;  // fuse

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if(G0 < -40) G0 = -40;      // -40dB -> Vin*0.01
    if(G0 > 6) G0 = 6;          // +6dB -> Vin*2
    if(G1 < -40) G1 = -40;
    if(G1 > 6) G1 = 6;
    if(G2 < -40) G2 = -40;
    if(G2 > 6) G2 = 6;

    const float FcLS   =  500;  // Frequency LowShelf[Hz]
    const float FcPKEQ = 3000;  // Frequency PeakEQ[Hz]
    const float FcHS   = 6000;  // Frequency HighShelf[Hz]

    float K, norm, Q, Fc, V ;

    // LOWSHELF
    Fc = (float)FcLS / (float)sampleRate; // Cutoff frequency
    K = tanf((float)PI * Fc);
    V = powf(10, fabs(G0) / 20.0);

    if (G0 >= 0) {  // boost
        norm = 1 / (1 + sqrtf(2) * K + K * K);
        _filter[LOWSHELF].a0 = (1 + sqrtf(2*V) * K + V * K * K) * norm;
        _filter[LOWSHELF].a1 = 2 * (V * K * K - 1) * norm;
        _filter[LOWSHELF].a2 = (1 - sqrtf(2*V) * K + V * K * K) * norm;
        _filter[LOWSHELF].b1 = 2 * (K * K - 1) * norm;
        _filter[LOWSHELF].b2 = (1 - sqrtf(2) * K + K * K) * norm;
    }
    else {          // cut
        norm = 1 / (1 + sqrtf(2*V) * K + V * K * K);
        _filter[LOWSHELF].a0 = (1 + sqrtf(2) * K + K * K) * norm;
        _filter[LOWSHELF].a1 = 2 * (K * K - 1) * norm;
        _filter[LOWSHELF].a2 = (1 - sqrtf(2) * K + K * K) * norm;
        _filter[LOWSHELF].b1 = 2 * (V * K * K - 1) * norm;
        _filter[LOWSHELF].b2 = (1 - sqrtf(2*V) * K + V * K * K) * norm;
    }

    // PEAK EQ
    Fc = (float)FcPKEQ / (float)sampleRate; // Cutoff frequency
    K = tanf((float)PI * Fc);
    V = powf(10, fabs(G1) / 20.0);
    Q = 2.5; // Quality factor
    if (G1 >= 0) { // boost
        norm = 1 / (1 + 1/Q * K + K * K);
        _filter[PEAKEQ].a0 = (1 + V/Q * K + K * K) * norm;
        _filter[PEAKEQ].a1 = 2 * (K * K - 1) * norm;
        _filter[PEAKEQ].a2 = (1 - V/Q * K + K * K) * norm;
        _filter[PEAKEQ].b1 = _filter[PEAKEQ].a1;
        _filter[PEAKEQ].b2 = (1 - 1/Q * K + K * K) * norm;
    }
    else {    // cut
        norm = 1 / (1 + V/Q * K + K * K);
        _filter[PEAKEQ].a0 = (1 + 1/Q * K + K * K) * norm;
        _filter[PEAKEQ].a1 = 2 * (K * K - 1) * norm;
        _filter[PEAKEQ].a2 = (1 - 1/Q * K + K * K) * norm;
        _filter[PEAKEQ].b1 = _filter[PEAKEQ].a1;
        _filter[PEAKEQ].b2 = (1 - V/Q * K + K * K) * norm;
    }

    // HIGHSHELF
    Fc = (float)FcHS / (float)sampleRate; // Cutoff frequency
    K = tanf((float)PI * Fc);
    V = powf(10, fabs(G2) / 20.0);
    if (G2 >= 0) {  // boost
        norm = 1 / (1 + sqrtf(2) * K + K * K);
        _filter[HIFGSHELF].a0 = (V + sqrtf(2*V) * K + K * K) * norm;
        _filter[HIFGSHELF].a1 = 2 * (K * K - V) * norm;
        _filter[HIFGSHELF].a2 = (V - sqrtf(2*V) * K + K * K) * norm;
        _filter[HIFGSHELF].b1 = 2 * (K * K - 1) * norm;
        _filter[HIFGSHELF].b2 = (1 - sqrtf(2) * K + K * K) * norm;
    }
    else {
        norm = 1 / (V + sqrtf(2*V) * K + K * K);
        _filter[HIFGSHELF].a0 = (1 + sqrtf(2) * K + K * K) * norm;
        _filter[HIFGSHELF].a1 = 2 * (K * K - 1) * norm;
        _filter[HIFGSHELF].a2 = (1 - sqrtf(2) * K + K * K) * norm;
        _filter[HIFGSHELF].b1 = 2 * (K * K - V) * norm;
        _filter[HIFGSHELF].b2 = (V - sqrtf(2*V) * K + K * K) * norm;
    }

//    log_i("LS a0=%f, a1=%f, a2=%f, b1=%f, b2=%f", _filter[0].a0, _filter[0].a1, _filter[0].a2,
//                                                  _filter[0].b1, _filter[0].b2);
//    log_i("EQ a0=%f, a1=%f, a2=%f, b1=%f, b2=%f", _filter[1].a0, _filter[1].a1, _filter[1].a2,
//                                                  _filter[1].b1, _filter[1].b2);
//    log_i("HS a0=%f, a1=%f, a2=%f, b1=%f, b2=%f", _filter[2].a0, _filter[2].a1, _filter[2].a2,
//                                                  _filter[2].b1, _filter[2].b2);
}
//---------------------------------------------------------------------------------------------------------------------
int16_t* Audio::IIR_filterChain0(int16_t iir_in[2], bool clear){  // Infinite Impulse Response (IIR) filters
    uint8_t z1 = 0, z2 = 1;
    enum: uint8_t {in = 0, out = 1};
    float inSample[2];
    float outSample[2];
    static int16_t iir_out[2];

    if(clear){
        memset(_filterBuff, 0, sizeof(_filterBuff));            // zero IIR filterbuffer
        iir_out[0] = 0;
        iir_out[1] = 0;
        iir_in[0]  = 0;
        iir_in[1]  = 0;
    }

    inSample[LEFTCHANNEL]  = (float)(iir_in[LEFTCHANNEL]);
    inSample[RIGHTCHANNEL] = (float)(iir_in[RIGHTCHANNEL]);

    outSample[LEFTCHANNEL] =   _filter[0].a0  * inSample[LEFTCHANNEL]
                             + _filter[0].a1  * _filterBuff[0][z1][in] [LEFTCHANNEL]
                             + _filter[0].a2  * _filterBuff[0][z2][in] [LEFTCHANNEL]
                             - _filter[0].b1  * _filterBuff[0][z1][out][LEFTCHANNEL]
                             - _filter[0].b2  * _filterBuff[0][z2][out][LEFTCHANNEL];

    _filterBuff[0][z2][in] [LEFTCHANNEL]  = _filterBuff[0][z1][in][LEFTCHANNEL];
    _filterBuff[0][z1][in] [LEFTCHANNEL]  = inSample[LEFTCHANNEL];
    _filterBuff[0][z2][out][LEFTCHANNEL]  = _filterBuff[0][z1][out][LEFTCHANNEL];
    _filterBuff[0][z1][out][LEFTCHANNEL]  = outSample[LEFTCHANNEL];
    iir_out[LEFTCHANNEL] = (int16_t)outSample[LEFTCHANNEL];


    outSample[RIGHTCHANNEL] =  _filter[0].a0 * inSample[RIGHTCHANNEL]
                             + _filter[0].a1 * _filterBuff[0][z1][in] [RIGHTCHANNEL]
                             + _filter[0].a2 * _filterBuff[0][z2][in] [RIGHTCHANNEL]
                             - _filter[0].b1 * _filterBuff[0][z1][out][RIGHTCHANNEL]
                             - _filter[0].b2 * _filterBuff[0][z2][out][RIGHTCHANNEL];

    _filterBuff[0][z2][in] [RIGHTCHANNEL] = _filterBuff[0][z1][in][RIGHTCHANNEL];
    _filterBuff[0][z1][in] [RIGHTCHANNEL] = inSample[RIGHTCHANNEL];
    _filterBuff[0][z2][out][RIGHTCHANNEL] = _filterBuff[0][z1][out][RIGHTCHANNEL];
    _filterBuff[0][z1][out][RIGHTCHANNEL] = outSample[RIGHTCHANNEL];
    iir_out[RIGHTCHANNEL] = (int16_t) outSample[RIGHTCHANNEL];

    return iir_out;
}
//---------------------------------------------------------------------------------------------------------------------
int16_t* Audio::IIR_filterChain1(int16_t iir_in[2], bool clear){  // Infinite Impulse Response (IIR) filters

    uint8_t z1 = 0, z2 = 1;
    enum: uint8_t {in = 0, out = 1};
    float inSample[2];
    float outSample[2];
    static int16_t iir_out[2];

    if(clear){
        memset(_filterBuff, 0, sizeof(_filterBuff));            // zero IIR filterbuffer
        iir_out[0] = 0;
        iir_out[1] = 0;
        iir_in[0]  = 0;
        iir_in[1]  = 0;
    }

    inSample[LEFTCHANNEL]  = (float)(iir_in[LEFTCHANNEL]);
    inSample[RIGHTCHANNEL] = (float)(iir_in[RIGHTCHANNEL]);

    outSample[LEFTCHANNEL] =   _filter[1].a0  * inSample[LEFTCHANNEL]
                             + _filter[1].a1  * _filterBuff[1][z1][in] [LEFTCHANNEL]
                             + _filter[1].a2  * _filterBuff[1][z2][in] [LEFTCHANNEL]
                             - _filter[1].b1  * _filterBuff[1][z1][out][LEFTCHANNEL]
                             - _filter[1].b2  * _filterBuff[1][z2][out][LEFTCHANNEL];

    _filterBuff[1][z2][in] [LEFTCHANNEL]  = _filterBuff[1][z1][in][LEFTCHANNEL];
    _filterBuff[1][z1][in] [LEFTCHANNEL]  = inSample[LEFTCHANNEL];
    _filterBuff[1][z2][out][LEFTCHANNEL]  = _filterBuff[1][z1][out][LEFTCHANNEL];
    _filterBuff[1][z1][out][LEFTCHANNEL]  = outSample[LEFTCHANNEL];
    iir_out[LEFTCHANNEL] = (int16_t)outSample[LEFTCHANNEL];


    outSample[RIGHTCHANNEL] =  _filter[1].a0 * inSample[RIGHTCHANNEL]
                             + _filter[1].a1 * _filterBuff[1][z1][in] [RIGHTCHANNEL]
                             + _filter[1].a2 * _filterBuff[1][z2][in] [RIGHTCHANNEL]
                             - _filter[1].b1 * _filterBuff[1][z1][out][RIGHTCHANNEL]
                             - _filter[1].b2 * _filterBuff[1][z2][out][RIGHTCHANNEL];

    _filterBuff[1][z2][in] [RIGHTCHANNEL] = _filterBuff[1][z1][in][RIGHTCHANNEL];
    _filterBuff[1][z1][in] [RIGHTCHANNEL] = inSample[RIGHTCHANNEL];
    _filterBuff[1][z2][out][RIGHTCHANNEL] = _filterBuff[1][z1][out][RIGHTCHANNEL];
    _filterBuff[1][z1][out][RIGHTCHANNEL] = outSample[RIGHTCHANNEL];
    iir_out[RIGHTCHANNEL] = (int16_t) outSample[RIGHTCHANNEL];

    return iir_out;
}
//---------------------------------------------------------------------------------------------------------------------
int16_t* Audio::IIR_filterChain2(int16_t iir_in[2], bool clear){  // Infinite Impulse Response (IIR) filters

    uint8_t z1 = 0, z2 = 1;
    enum: uint8_t {in = 0, out = 1};
    float inSample[2];
    float outSample[2];
    static int16_t iir_out[2];

    if(clear){
        memset(_filterBuff, 0, sizeof(_filterBuff));            // zero IIR filterbuffer
        iir_out[0] = 0;
        iir_out[1] = 0;
        iir_in[0]  = 0;
        iir_in[1]  = 0;
    }

    inSample[LEFTCHANNEL]  = (float)(iir_in[LEFTCHANNEL]);
    inSample[RIGHTCHANNEL] = (float)(iir_in[RIGHTCHANNEL]);

    outSample[LEFTCHANNEL] =   _filter[2].a0  * inSample[LEFTCHANNEL]
                             + _filter[2].a1  * _filterBuff[2][z1][in] [LEFTCHANNEL]
                             + _filter[2].a2  * _filterBuff[2][z2][in] [LEFTCHANNEL]
                             - _filter[2].b1  * _filterBuff[2][z1][out][LEFTCHANNEL]
                             - _filter[2].b2  * _filterBuff[2][z2][out][LEFTCHANNEL];

    _filterBuff[2][z2][in] [LEFTCHANNEL]  = _filterBuff[2][z1][in][LEFTCHANNEL];
    _filterBuff[2][z1][in] [LEFTCHANNEL]  = inSample[LEFTCHANNEL];
    _filterBuff[2][z2][out][LEFTCHANNEL]  = _filterBuff[2][z1][out][LEFTCHANNEL];
    _filterBuff[2][z1][out][LEFTCHANNEL]  = outSample[LEFTCHANNEL];
    iir_out[LEFTCHANNEL] = (int16_t)outSample[LEFTCHANNEL];


    outSample[RIGHTCHANNEL] =  _filter[2].a0 * inSample[RIGHTCHANNEL]
                             + _filter[2].a1 * _filterBuff[2][z1][in] [RIGHTCHANNEL]
                             + _filter[2].a2 * _filterBuff[2][z2][in] [RIGHTCHANNEL]
                             - _filter[2].b1 * _filterBuff[2][z1][out][RIGHTCHANNEL]
                             - _filter[2].b2 * _filterBuff[2][z2][out][RIGHTCHANNEL];

    _filterBuff[2][z2][in] [RIGHTCHANNEL] = _filterBuff[2][z1][in][RIGHTCHANNEL];
    _filterBuff[2][z1][in] [RIGHTCHANNEL] = inSample[RIGHTCHANNEL];
    _filterBuff[2][z2][out][RIGHTCHANNEL] = _filterBuff[2][z1][out][RIGHTCHANNEL];
    _filterBuff[2][z1][out][RIGHTCHANNEL] = outSample[RIGHTCHANNEL];
    iir_out[RIGHTCHANNEL] = (int16_t) outSample[RIGHTCHANNEL];

    return iir_out;
}


Audio Audio_Handle;