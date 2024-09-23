#include "audio.h"

static uint8_t _duty_waveform[] = {
    0b00000001, 0b10000001, 0b10000111, 0b01111110
};

void
gbc_audio_init(gbc_audio_t *audio, uint32_t sample_rate)
{
    memset(audio, 0, sizeof(gbc_audio_t));
    audio->output_sample_rate = sample_rate;    
}

void 
gbc_audio_connect(gbc_audio_t *audio, gbc_memory_t *mem)
{
    audio->NR50 = connect_io_port(mem, IO_PORT_NR50);
    audio->NR51 = connect_io_port(mem, IO_PORT_NR51);
    audio->NR52 = connect_io_port(mem, IO_PORT_NR52);

    audio->c1.NRx0 = connect_io_port(mem, IO_PORT_NR10);
    audio->c1.NRx1 = connect_io_port(mem, IO_PORT_NR11);
    audio->c1.NRx2 = connect_io_port(mem, IO_PORT_NR12);
    audio->c1.NRx3 = connect_io_port(mem, IO_PORT_NR13);
    audio->c1.NRx4 = connect_io_port(mem, IO_PORT_NR14);

    audio->c2.NRx1 = connect_io_port(mem, IO_PORT_NR21);
    audio->c2.NRx2 = connect_io_port(mem, IO_PORT_NR22);
    audio->c2.NRx3 = connect_io_port(mem, IO_PORT_NR23);
    audio->c2.NRx4 = connect_io_port(mem, IO_PORT_NR24);

    audio->c3.NRx0 = connect_io_port(mem, IO_PORT_NR30);
    audio->c3.NRx1 = connect_io_port(mem, IO_PORT_NR31);
    audio->c3.NRx2 = connect_io_port(mem, IO_PORT_NR32);
    audio->c3.NRx3 = connect_io_port(mem, IO_PORT_NR33);
    audio->c3.NRx4 = connect_io_port(mem, IO_PORT_NR34);

    audio->c4.NRx1 = connect_io_port(mem, IO_PORT_NR41);
    audio->c4.NRx2 = connect_io_port(mem, IO_PORT_NR42);
    audio->c4.NRx3 = connect_io_port(mem, IO_PORT_NR43);
    audio->c4.NRx4 = connect_io_port(mem, IO_PORT_NR44);
}

static uint8_t 
ch1_audio(gbc_audio_t *audio)
{
    gbc_audio_channel_t *ch = &(audio->c1);

    /* https://gbdev.io/pandocs/Audio_details.html#dacs */
    if (!(*(ch->NRx2) & DAC_ON_MASK))
        return 0;

    uint8_t triggered =  0;
    if (*(ch->NRx2) & CHANNEL_TRIGGER_MASK) {
        /* since the game cannot read this field, i think we can change it */
        *(ch->NRx2) &= ~CHANNEL_TRIGGER_MASK;
        ch->on = 1;
        triggered = 1;
        /* TODO: seems that we have to somehow retrigger this channel which i dont know what it means exactly */
        ch->sample_cycles = 0;
        ch->sample_idx = 0;
        ch->sweep_pace = 0;
    }

    if (!ch->on) {
        return 0;
    }                

    if (triggered || (audio->frame_sequencer % FRAME_FREQ_SWEEP == 0)) {
        /* frequency sweep */
        if (ch->sweep_pace == 0) {
            ch->sweep_pace = CHANNEL_SWEEP_PACE(ch);
            uint8_t steps = CHANNEL_SWEEP_STEPS(ch);
            uint16_t period = CHANNEL_PERIOD(ch);
            if (CHANNEL_SWEEP_DIRECTION(ch)) {                                
                /* decrease */
                period -= period >> steps;
            } else {
                /* increase */
                period += period >> steps;
            }
            CHANNEL_PERIOD_UPDATE(ch, period);
            if (period >= 0x7ff) {
                /* overflowed */
                ch->on = 0;
            }                                                            
        } else {
            ch->sweep_pace--;
        }
    }
    
    if ((audio->frame_sequencer % FRAME_SOUND_LENGTH == 0)) {
        /* sound length */
        if (*(ch->NRx1) & CHANNEL_LENGTH_ENABLE_MASK) {
            uint8_t length = *(ch->NRx1) & 0x3f;
            if (length == 0) {
                ch->on = 0;
            } else {
                length++;                
                /* again, games cannot read this field, i think we can change it */
                *(ch->NRx1) &= ~0x3f;
                *(ch->NRx1) |= length;
            }
        }
    }

    if (triggered) {
        ch->volume = CHANNEL_EVENVLOPE_VOLUME(ch);
    }

    if (audio->frame_sequencer % FRAME_ENVELOPE_SWEEP == 0) {
        /* envelope sweep */
        uint8_t pace = CHANNEL_EVENVLOPE_PACE(ch);
        if (CHANNEL_EVENVLOPE_DIRECTION(ch)) {
            ch->volume += pace;
        } else {
            ch->volume -= pace;
        }                
    }
    
    uint16_t sample_rate = CHANNEL_SAMPLE_RATE(ch);
    if (ch->sample_cycles == 0) {                        
        if (ch->sample_idx == WAVEFORM_SAMPLES)
            ch->sample_idx = 1;
        else
            ch->sample_idx += 1;
        ch->sample_cycles = AUDIO_CLOCK_RATE / sample_rate;
    }

    uint8_t on = WAVEFORM_SAMPLE(_duty_waveform[CHANNEL_DUTY(ch)], ch->sample_idx-1);        
 
    return on * ch->volume;
}

static uint8_t 
ch2_audio(gbc_audio_t *audio)
{
    gbc_audio_channel_t *ch = &(audio->c2);

    /* https://gbdev.io/pandocs/Audio_details.html#dacs */
    if (!(*(ch->NRx2) & DAC_ON_MASK))
        return 0;

    uint8_t triggered =  0;
    if (*(ch->NRx2) & CHANNEL_TRIGGER_MASK) {
        /* since the game cannot read this field, i think we can change it */
        *(ch->NRx2) &= ~CHANNEL_TRIGGER_MASK;
        ch->on = 1;
        triggered = 1;
        /* TODO: seems that we have to somehow retrigger this channel which i dont know what it means exactly */
        ch->sample_cycles = 0;
        ch->sample_idx = 0;
        ch->sweep_pace = 0;
    }

    if (!ch->on) {
        return 0;
    }                
    
    if ((audio->frame_sequencer % FRAME_SOUND_LENGTH == 0)) {
        /* sound length */
        if (*(ch->NRx1) & CHANNEL_LENGTH_ENABLE_MASK) {
            uint8_t length = *(ch->NRx1) & 0x3f;
            if (length == 0) {
                ch->on = 0;
            } else {
                length++;                
                /* again, games cannot read this field, i think we can change it */
                *(ch->NRx1) &= ~0x3f;
                *(ch->NRx1) |= length;
            }
        }
    }

    if (triggered) {
        ch->volume = CHANNEL_EVENVLOPE_VOLUME(ch);
    }

    if (audio->frame_sequencer % FRAME_ENVELOPE_SWEEP == 0) {
        /* envelope sweep */
        uint8_t pace = CHANNEL_EVENVLOPE_PACE(ch);
        if (CHANNEL_EVENVLOPE_DIRECTION(ch)) {
            ch->volume += pace;
        } else {
            ch->volume -= pace;
        }                
    }
    
    uint16_t sample_rate = CHANNEL_SAMPLE_RATE(ch);
    if (ch->sample_cycles == 0) {                        
        if (ch->sample_idx == WAVEFORM_SAMPLES)
            ch->sample_idx = 1;
        else
            ch->sample_idx += 1;
        ch->sample_cycles = AUDIO_CLOCK_RATE / sample_rate;
    }

    uint8_t on = WAVEFORM_SAMPLE(_duty_waveform[CHANNEL_DUTY(ch)], ch->sample_idx-1);        

    return on * ch->volume;
}

static uint8_t
ch3_audio(gbc_audio_t *audio)
{}

static uint8_t
ch4_audio(gbc_audio_t *audio)
{}

void
gbc_audio_cycle(gbc_audio_t *audio)
{
    if (!(*audio->NR52 & NR52_AUDIO_ON))
        return;

    audio->m_cycles++;
    if (audio->m_cycles != AUDIO_CLOCK_CYCLES) {        
        return;
    }

    audio->m_cycles = 0;
    audio->cycles++;

    /* 
    Actually frame sequencer is not a seperate timer in real GameBoy, it is tied to DIV register.
    I simplied it.
    https://gbdev.io/pandocs/Audio_details.html#div-apu */
    audio->frame_sequencer_cycles++;        
    if (audio->frame_sequencer_cycles == FRAME_SEQUENCER_CYCLES) {
        audio->frame_sequencer_cycles = 0;
        audio->frame_sequencer++;
    }

    uint8_t c1 = ch1_audio(audio);
    uint8_t c2 = 0;//ch2_audio(audio);
    uint8_t c3 = ch3_audio(audio);
    uint8_t c4 = ch4_audio(audio);

    uint8_t sample = c1 + c2 + c3 + c4;
    /* rewind */
    if (audio->frame_sequencer == FRAME_ENVELOPE_SWEEP)
        audio->frame_sequencer = 1;

    if (audio->output_sample_cycles == 0) {
        audio->audio_write(sample);
        audio->output_sample_cycles = AUDIO_CLOCK_RATE / audio->output_sample_rate;                                            
    }
    audio->output_sample_cycles--;
    /* TODO: volume panning */
}
