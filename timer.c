#include "timer.h"
#include "cpu.h"

static uint8_t _timer_mode_cycles[] = {
    TAC_TIMER_SPEED_MODE_0_CYCLES,
    TAC_TIMER_SPEED_MODE_1_CYCLES,
    TAC_TIMER_SPEED_MODE_2_CYCLES,
    TAC_TIMER_SPEED_MODE_3_CYCLES
};

void 
gbc_timer_init(gbc_timer_t *timer)
{
    memset(timer, 0, sizeof(gbc_timer_t));
}

void 
gbc_timer_connect(gbc_timer_t *timer, gbc_memory_t *mem)
{
    timer->mem = mem;
    timer->divp = connect_io_port(mem, IO_PORT_DIV);
    timer->timap = connect_io_port(mem, IO_PORT_TIMA);
    timer->tmap = connect_io_port(mem, IO_PORT_TMA);
    timer->tacp = connect_io_port(mem, IO_PORT_TAC);        
}

void gbc_timer_cycle(gbc_timer_t *timer)
{    
    if (++timer->div_cycles == TICK_DIVIDER) {
        timer->div_cycles = 0;
        (*timer->divp)++;    
    }

    if (!(*timer->tacp & TAC_TIMER_ENABLE)) {
        return;
    }
    
    uint8_t mode = *timer->tacp & TAC_TIMER_SPEED_MASK;
    uint8_t cycles = _timer_mode_cycles[mode];
    
    uint16_t tima = *timer->timap;    

    if (++timer->timer_cycles == cycles) {
        /* 
        https://gbdev.io/pandocs/Timer_and_Divider_Registers.html#ff06--tma-timer-modulo
        If a TMA write is executed on the same M-cycle as the content of TMA is transferred to TIMA due to a timer overflow, the old value is transferred to TIMA. 
        I didnt implement this. Should have implemented a flip-flop mechanism.
        */
        timer->timer_cycles = 0;
        tima += 1;        
        if (tima > 0xFF) {
            *timer->timap = *timer->tmap;
            REQUEST_INTERRUPT(timer->mem, INTERRUPT_TIMER);
        }
        IO_PORT_WRITE(timer->mem, IO_PORT_TIMA, tima);
    }

}