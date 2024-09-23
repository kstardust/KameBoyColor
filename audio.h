#ifndef _GBC_AUDIO
#define _GBC_AUDIO

#include "common.h"
#include "memory.h"
#include "cpu.h"

#define NR52_AUDIO_ON   0x80
#define NR52_CH1ON      0x8
#define NR52_CH2ON      0x4
#define NR52_CH3ON      0x2
#define NR52_CH4ON      0x1

#define NR10_PACE       0x70
#define NR10_DIRECTION  0x8
#define NR10_IND_STEPS  0x7

#define GBC_OUTPUT_SAMPLE_RATE 44100

/* https://gbdev.io/pandocs/Audio_Registers.html#ff13--nr13-channel-1-period-low-write-only */
#define AUDIO_CLOCK_RATE        1048576
#define AUDIO_CLOCK_CYCLES      (CLOCK_RATE / AUDIO_CLOCK_RATE)
#define WAVEFORM_SAMPLES        8

#define FRAME_SEQUENCER_CYCLES (AUDIO_CLOCK_RATE / 512) /* runs at 512Hz */
#define FRAME_ENVELOPE_SWEEP   8
#define FRAME_SOUND_LENGTH     2
#define FRAME_FREQ_SWEEP       4


#define CHANNEL_PERIOD_UPDATE(c, p) {    \
    *((c)->NRx3) = (p & 0xff);            \
    *((c)->NRx4) &= 0xf8;                 \
    *((c)->NRx4) |= (p >> 8) & 0x7;       \
}

#define CHANNEL_PERIOD(c) (uint16_t) ((( (*((c)->NRx4)) & 0x7) << 8) | (*((c)->NRx3)) )
#define CHANNEL_SAMPLE_RATE(c)  (AUDIO_CLOCK_RATE / (2048 - CHANNEL_PERIOD(c)))
#define CHANNEL_HZ(c)  (CHANNEL_SAMPLE_RATE((c)) / WAVEFORM_SAMPLES)
#define CHANNEL_DUTY(c) (( (*((c)->NRx1)) >> 6) & 0x3)
#define CHANNEL_SWEEP_PACE(c) (( (*((c)->NRx0)) >> 4) & 0x7)
#define CHANNEL_SWEEP_DIRECTION(c) ( (*((c)->NRx0)) & 0x8)
#define CHANNEL_SWEEP_STEPS(c) ( (*((c)->NRx0)) & 0x7)

#define CHANNEL_EVENVLOPE_PACE(c) ( (*((c)->NRx2)) | 0x7)
#define CHANNEL_EVENVLOPE_VOLUME(c) ( (*((c)->NRx2)) >> 4)
#define CHANNEL_EVENVLOPE_DIRECTION(c) ( (*((c)->NRx2)) & 0x8)

#define WAVEFORM_SAMPLE(wav, idx) ((wav) & (1 << (idx)))

#define DAC_ON_MASK 0xf8
#define CHANNEL_TRIGGER_MASK        0x80
#define CHANNEL_LENGTH_ENABLE_MASK  0x40

typedef struct gbc_audio gbc_audio_t;
typedef struct gbc_audio_channel gbc_audio_channel_t;

struct gbc_audio_channel {
    uint8_t *NRx0;
    uint8_t *NRx1;
    uint8_t *NRx2;
    uint8_t *NRx3;
    uint8_t *NRx4;

    uint16_t sample_cycles;
    uint8_t sample_idx;
    uint8_t sweep_pace;
    uint8_t timer;
    uint8_t volume;            

    uint8_t on;
};

struct gbc_audio {
    uint64_t cycles;                /* audio cycles, ticks at AUDIO_CLOCK_RATE */
    gbc_audio_channel_t c1;
    gbc_audio_channel_t c2;
    gbc_audio_channel_t c3;
    gbc_audio_channel_t c4;    

    /* https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Registers */
    
    /* Control/Status */
    uint8_t *NR52;
    uint8_t *NR51;
    uint8_t *NR50;
        
    void (*audio_write)(uint8_t);
    void (*audio_update)(void *udata);

    uint32_t output_sample_rate;
    uint32_t output_sample_cycles;
    /* https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frame_Sequencer */    
    uint16_t frame_sequencer_cycles;    
    uint8_t m_cycles;

    uint8_t frame_sequencer;    
};

void gbc_audio_connect(gbc_audio_t *audio, gbc_memory_t *mem);
void gbc_audio_init(gbc_audio_t *audio, uint32_t sample_rate);
void gbc_audio_cycle(gbc_audio_t *audio);

#endif