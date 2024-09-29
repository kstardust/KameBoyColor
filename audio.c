#include "audio.h"

/* there are many obscure details about the audio system in the gameboy
 * that I didnt implement, partially because the doc is kind of confusing
 * which I am not excatly sure what they mean, and another obvious reason:
 * I'm lazy.
 *
 * https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
 * https://gbdev.io/pandocs/Audio_Registers.html
 */

static uint8_t _duty_waveform[] = {
    0b00000001, 0b10000001, 0b10000111, 0b01111110
};

void
gbc_audio_init(gbc_audio_t *audio)
{
    memset(audio, 0, sizeof(gbc_audio_t));
    audio->NR52 = audio->audio_mem + IO_PORT_NR52 - AUDIO_PORT_BEGIN;
    audio->NR51 = audio->audio_mem + IO_PORT_NR51 - AUDIO_PORT_BEGIN;
    audio->NR50 = audio->audio_mem + IO_PORT_NR50 - AUDIO_PORT_BEGIN;

    audio->c1.NRx0 = audio->audio_mem + IO_PORT_NR10 - AUDIO_PORT_BEGIN;
    audio->c1.NRx1 = audio->audio_mem + IO_PORT_NR11 - AUDIO_PORT_BEGIN;
    audio->c1.NRx2 = audio->audio_mem + IO_PORT_NR12 - AUDIO_PORT_BEGIN;
    audio->c1.NRx3 = audio->audio_mem + IO_PORT_NR13 - AUDIO_PORT_BEGIN;
    audio->c1.NRx4 = audio->audio_mem + IO_PORT_NR14 - AUDIO_PORT_BEGIN;

    audio->c2.NRx1 = audio->audio_mem + IO_PORT_NR21 - AUDIO_PORT_BEGIN;
    audio->c2.NRx2 = audio->audio_mem + IO_PORT_NR22 - AUDIO_PORT_BEGIN;
    audio->c2.NRx3 = audio->audio_mem + IO_PORT_NR23 - AUDIO_PORT_BEGIN;
    audio->c2.NRx4 = audio->audio_mem + IO_PORT_NR24 - AUDIO_PORT_BEGIN;

    audio->c3.NRx0 = audio->audio_mem + IO_PORT_NR30 - AUDIO_PORT_BEGIN;
    audio->c3.NRx1 = audio->audio_mem + IO_PORT_NR31 - AUDIO_PORT_BEGIN;
    audio->c3.NRx2 = audio->audio_mem + IO_PORT_NR32 - AUDIO_PORT_BEGIN;
    audio->c3.NRx3 = audio->audio_mem + IO_PORT_NR33 - AUDIO_PORT_BEGIN;
    audio->c3.NRx4 = audio->audio_mem + IO_PORT_NR34 - AUDIO_PORT_BEGIN;

    audio->c4.NRx1 = audio->audio_mem + IO_PORT_NR41 - AUDIO_PORT_BEGIN;
    audio->c4.NRx2 = audio->audio_mem + IO_PORT_NR42 - AUDIO_PORT_BEGIN;
    audio->c4.NRx3 = audio->audio_mem + IO_PORT_NR43 - AUDIO_PORT_BEGIN;
    audio->c4.NRx4 = audio->audio_mem + IO_PORT_NR44 - AUDIO_PORT_BEGIN;

}

uint8_t
audio_read(void *udata, uint16_t addr)
{
    gbc_audio_t *audio = (gbc_audio_t*)udata;
    return audio->audio_mem[addr - AUDIO_BEGIN];
}

uint8_t
audio_write(void *udata, uint16_t addr, uint8_t data)
{
    gbc_audio_t *audio = (gbc_audio_t*)udata;
    audio->audio_mem[addr - AUDIO_BEGIN] = data;
    return data;
}

void
gbc_audio_connect(gbc_audio_t *audio, gbc_memory_t *mem)
{
    memory_map_entry_t entry;
    entry.id = AUDIO_ID;
    entry.addr_begin = AUDIO_BEGIN;
    entry.addr_end = AUDIO_END;
    entry.read = audio_read;
    entry.write = audio_write;
    entry.udata = audio;

    register_memory_map(mem, &entry);

    audio->mem = mem;
}

static int8_t
ch1_audio(gbc_audio_t *audio)
{
    gbc_audio_channel_t *ch = &(audio->c1);

    /* https://gbdev.io/pandocs/Audio_details.html#dacs */
    if (!(*(ch->NRx2) & DAC_ON_MASK))
        return 0;

    uint8_t triggered =  0;
    if (*(ch->NRx4) & CHANNEL_TRIGGER_MASK) {
        /* since the game cannot read this field, i think we can change it */
        *(ch->NRx4) &= ~CHANNEL_TRIGGER_MASK;
        ch->on = 1;
        triggered = 1;

        /* TODO: we need to somehow retrigger this channel which I'm not exactly sure what it means */
        ch->sample_cycles = 0;
        ch->waveform_idx = 0;
        ch->sweep_pace = 0;
        ch->volume = CHANNEL_EVENVLOPE_VOLUME(ch);
        ch->volume_pace = CHANNEL_EVENVLOPE_PACE(ch);
        ch->volume_pace_counter = ch->volume_pace;
        ch->volume_dir = CHANNEL_EVENVLOPE_DIRECTION(ch);
    }

    if (!ch->on) {
        return 0;
    }

    if (triggered || (audio->frame_sequencer % FRAME_FREQ_SWEEP == 0)) {
        /* frequency sweep */

        if (CHANNEL_SWEEP_PACE(ch) == 0) {
            /* write 0 to sweep pace immediately disable the sweep */
            ch->sweep_pace = 0;
        } else {
            if (ch->sweep_pace == 0) {
                ch->sweep_pace = CHANNEL_SWEEP_PACE(ch);
                uint8_t steps = CHANNEL_SWEEP_STEPS(ch);
                uint16_t period = CHANNEL_PERIOD(ch);
                /* step shift needs be non-zero
                    which isn't mentioned in the PanDoc but in gg8 wiki
                    https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frequency_Sweep
                */
                if (steps != 0) {
                    if (CHANNEL_SWEEP_DIRECTION(ch)) {
                        /* decrease */
                        period -= period >> steps;
                    } else {
                        /* increase */
                        period += period >> steps;
                    }
                    period &= 0x7ff;
                    CHANNEL_PERIOD_UPDATE(ch, period);
                    if (period >= 0x7ff) {
                        /* overflowed */
                        ch->on = 0;
                    }
                }
            } else {
                ch->sweep_pace--;
            }
        }
    }

    if ((audio->frame_sequencer % FRAME_SOUND_LENGTH == 0)) {
        /* sound length */
        if (*(ch->NRx4) & CHANNEL_LENGTH_ENABLE_MASK) {
            uint8_t length = *(ch->NRx1) & 0x3f;
            if (length == 0) {
                ch->on = 0;
            } else {
                length++;
                /* again, games cannot read this field, i think we can change it */
                length &= 0x3f;
                *(ch->NRx1) &= ~0x3f;
                *(ch->NRx1) |= length;
            }
        }
    }

    if (ch->volume_pace != 0 &&
        (audio->frame_sequencer % FRAME_ENVELOPE_SWEEP == 0)) {
        /* envelope sweep */
        ch->volume_pace_counter--;
        if (ch->volume_pace_counter == 0) {
            ch->volume_pace_counter = ch->volume_pace;
            if (ch->volume_dir) {
                ch->volume++;
            } else {
                if (ch->volume != 0)
                    ch->volume--;
            }
            ch->volume &= 0xf;
        }
    }

    uint16_t sample_period = CHANNEL_PERIOD(ch);
    ch->sample_cycles++;
    if (ch->sample_cycles == 0x7ff) {
        if (ch->waveform_idx == WAVEFORM_SAMPLES)
            ch->waveform_idx = 1;
        else
            ch->waveform_idx += 1;
        ch->sample_cycles = sample_period;
    }

    uint8_t on = WAVEFORM_SAMPLE(_duty_waveform[CHANNEL_DUTY(ch)], ch->waveform_idx-1);

    return on * ch->volume;
}

static int8_t
ch2_audio(gbc_audio_t *audio)
{
    gbc_audio_channel_t *ch = &(audio->c2);

    /* https://gbdev.io/pandocs/Audio_details.html#dacs */
    if (!(*(ch->NRx2) & DAC_ON_MASK))
        return 0;

    uint8_t triggered =  0;
    if (*(ch->NRx4) & CHANNEL_TRIGGER_MASK) {
        /* since the game cannot read this field, i think we can change it */
        *(ch->NRx4) &= ~CHANNEL_TRIGGER_MASK;
        ch->on = 1;
        triggered = 1;
        ch->sample_cycles = 0;
        ch->waveform_idx = 0;
        ch->sweep_pace = 0;
        ch->volume = CHANNEL_EVENVLOPE_VOLUME(ch);
        ch->volume_pace = CHANNEL_EVENVLOPE_PACE(ch);
        ch->volume_pace_counter = ch->volume_pace;
        ch->volume_dir = CHANNEL_EVENVLOPE_DIRECTION(ch);
    }

    if (!ch->on) {
        return 0;
    }

    if ((audio->frame_sequencer % FRAME_SOUND_LENGTH == 0)) {
        /* sound length */
        if (*(ch->NRx4) & CHANNEL_LENGTH_ENABLE_MASK) {
            uint8_t length = *(ch->NRx1) & 0x3f;
            if (length == 0) {
                ch->on = 0;
            } else {
                length++;
                /* again, games cannot read this field, i think we can change it */
                length &= 0x3f;
                *(ch->NRx1) &= ~0x3f;
                *(ch->NRx1) |= length;
            }
        }
    }

    if (ch->volume_pace != 0 &&
        (audio->frame_sequencer % FRAME_ENVELOPE_SWEEP == 0)) {
        /* envelope sweep */
        ch->volume_pace_counter--;
        if (ch->volume_pace_counter == 0) {
            ch->volume_pace_counter = ch->volume_pace;
            if (ch->volume_dir) {
                ch->volume++;
            } else {
                if (ch->volume != 0)
                    ch->volume--;
            }
            ch->volume &= 0xf;
        }
    }

    uint16_t sample_period = CHANNEL_PERIOD(ch);
    ch->sample_cycles++;
    if (ch->sample_cycles == 0x7ff) {
        if (ch->waveform_idx == WAVEFORM_SAMPLES)
            ch->waveform_idx = 1;
        else
            ch->waveform_idx += 1;
        ch->sample_cycles = sample_period;
    }

    uint8_t on = WAVEFORM_SAMPLE(_duty_waveform[CHANNEL_DUTY(ch)], ch->waveform_idx-1);

    return on * ch->volume;
}

static int8_t
ch3_audio(gbc_audio_t *audio)
{
    gbc_audio_channel_t *ch = &(audio->c3);

    if (!(*(ch->NRx0) & CH3_DAC_ON_MASK))
        return 0;

    uint8_t triggered =  0;
    if (*(ch->NRx4) & CHANNEL_TRIGGER_MASK) {
        /* since the game cannot read this field, i think we can change it */
        *(ch->NRx4) &= ~CHANNEL_TRIGGER_MASK;
        ch->on = 1;
        triggered = 1;
        ch->sample_cycles = 0;
        ch->waveform_idx = 0;
        ch->sweep_pace = 0;
    }

    if (!ch->on) {
        return 0;
    }

    if ((audio->frame_sequencer % FRAME_SOUND_LENGTH == 0)) {
        /* sound length */
        if (*(ch->NRx4) & CHANNEL_LENGTH_ENABLE_MASK) {
            uint8_t length = *(ch->NRx1);
            if (length == 0) {
                ch->on = 0;
            } else {
                length++;
                *(ch->NRx1) = length;
            }
        }
    }

    uint16_t sample_period = CHANNEL_PERIOD(ch);
    ch->sample_cycles += 1;
    if (ch->sample_cycles == 0x7ff) {
        if (ch->waveform_idx == CH3_WAVEFORM_SAMPLES)
            ch->waveform_idx = 1;
        else
            ch->waveform_idx += 1;
        ch->sample_cycles = sample_period;
    }

    uint8_t volume = CHANNEL3_OUTPUT_LEVEL(ch);
    if (volume == 0)
        return 0;

    uint8_t sample_offset = (ch->waveform_idx-1) / 2;
    /* According to Pandoc, when CH3 is started, the first sample read is the one at index 1, I didn't implement this */
    uint8_t wave_form = IO_PORT_READ(audio->mem, IO_PORT_WAVE_RAM + sample_offset);
    if (ch->waveform_idx % 2 == 0)
        wave_form &= 0xf;
    else
        wave_form >>= 4;

    return wave_form >> (volume-1);
}

static int8_t
ch4_audio(gbc_audio_t *audio)
{
    gbc_audio_channel_t *ch = &(audio->c4);

    if (!(*(ch->NRx2) & DAC_ON_MASK))
        return 0;

    uint8_t triggered =  0;
    if (*(ch->NRx4) & CHANNEL_TRIGGER_MASK) {
        /* since the game cannot read this field, i think we can change it */
        *(ch->NRx4) &= ~CHANNEL_TRIGGER_MASK;
        ch->on = 1;
        triggered = 1;

        ch->volume = CHANNEL_EVENVLOPE_VOLUME(ch);
        ch->volume_pace = CHANNEL_EVENVLOPE_PACE(ch);
        ch->volume_pace_counter = ch->volume_pace;
        ch->volume_dir = CHANNEL_EVENVLOPE_DIRECTION(ch);
        ch->lfsr = 0x7fff;

        ch->sample_cycles = 0;

    }

    if (!ch->on) {
        return 0;
    }

    if ((audio->frame_sequencer % FRAME_SOUND_LENGTH == 0)) {
        /* sound length */
        if (*(ch->NRx4) & CHANNEL_LENGTH_ENABLE_MASK) {

            uint8_t length = *(ch->NRx1) & 0x3f;

            if (length == 0) {
                ch->on = 0;
            } else {
                length++;
                length &= 0x3f;
                *(ch->NRx1) &= ~0x3f;
                *(ch->NRx1) |= length;
            }
        }
    }

    if (ch->volume_pace != 0 &&
        (audio->frame_sequencer % FRAME_ENVELOPE_SWEEP == 0)) {
        /* envelope sweep */
        ch->volume_pace_counter--;
        if (ch->volume_pace_counter == 0) {
            ch->volume_pace_counter = ch->volume_pace;
            if (ch->volume_dir) {
                ch->volume++;
            } else {
                if (ch->volume != 0)
                    ch->volume--;
            }
            ch->volume &= 0xf;
        }
    }

    if (audio->cycles % FRAME_NOISE_TICK == 0) {

        if (ch->sample_cycles == 0) {
            uint8_t shift = CHANNEL4_CLOCK_SHIFT(ch);
            uint8_t divider = CHANNEL4_CLOCK_DIVIDER(ch);
            uint16_t lfsr = ch->lfsr;

            uint8_t b = (lfsr & 1) ^ ((lfsr >> 1) & 1);
            lfsr >>= 1;
            lfsr &= ~(1 << 14);
            lfsr |= b << 14;
            if (CHANNEL4_LFSR_SHORT_MODE(ch)) {
                lfsr &= ~(1 << 6);
                lfsr |= b << 6;
            }

            ch->lfsr = lfsr;

            uint32_t period = 1 << (shift + 1);
            if (divider == 0) {
                period >>= 1;
            } else {
                period *= divider;
            }

            ch->sample_cycles = period * 2;
        } else {
            ch->sample_cycles--;
        }
    }

    return (ch->lfsr & 1) * ch->volume;
}

void
gbc_audio_cycle(gbc_audio_t *audio)
{
    if (!(*(audio->NR52) & NR52_AUDIO_ON))
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

    int8_t c1 = ch1_audio(audio);
    int8_t c2 = ch2_audio(audio);
    /* ch3 runs at 2097152 Hz, but our audio module updates at 1048576 Hz,
        I'm doing linear interpolation here
    */
    int8_t c3 = (ch3_audio(audio) + ch3_audio(audio)) / 2;
    int8_t c4 = ch4_audio(audio) * 4;

    *(audio->NR52) &= ~0xf;
    *(audio->NR52) |= (audio->c1.on)
                    | ((audio->c2.on) << 1)
                    | ((audio->c3.on) << 2)
                    | ((audio->c4.on) << 3);

    /* todo read NR50 */
    uint8_t volume = 0x7;

    audio->sample += (c1 + c2 + c3 + c4) * volume / 4;

    /* rewind */
    if (audio->frame_sequencer == FRAME_ENVELOPE_SWEEP)
        audio->frame_sequencer = 1;

    if (audio->output_sample_cycles == 0) {
        /* linear interpolation */
        int8_t sample = audio->sample / audio->sample_divider;

        audio->audio_write(sample, sample);
        audio->output_sample_cycles = SAMPLE_TO_AUDIO_CYCLES;
        audio->output_sample_cycles_remainder += SAMPLE_TO_AUDIO_CYCLES_REMAINDER;

        if (audio->output_sample_cycles_remainder >= REMAINDER_SCALING_FACTOR) {
            audio->output_sample_cycles++;
            audio->output_sample_cycles_remainder -= REMAINDER_SCALING_FACTOR;
        }
        audio->sample = 0;
        audio->sample_divider = audio->output_sample_cycles;
    }
    audio->output_sample_cycles--;
    /* TODO: volume panning */
}
