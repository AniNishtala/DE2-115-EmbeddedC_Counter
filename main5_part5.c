#include <stdio.h>
#include <unistd.h>
#include <altera_avalon_pio_regs.h>
#include <sys/alt_irq.h>
#include <system.h>
#include <alt_types.h>
#include <altera_avalon_timer_regs.h>

static void timer_init();
static void timer_isr();
void clear_hex();
int toggle = 0;

void clear_hex()
{
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_4_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_5_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_6_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_7_BASE, 0xFF);
}

static void timer_init()
{
    // Setting the stop bit high
    IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 8);

    // 1 second clock
    int period = SYSTEM_TIMER_LOAD_VALUE * 1000;
    int period_h = period >> 16;
    int period_l = period & 0xFFFF;

    IOWR_ALTERA_AVALON_TIMER_PERIODL(SYSTEM_TIMER_BASE, period_l);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(SYSTEM_TIMER_BASE, period_h);
    IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 7);

    alt_ic_isr_register(SYSTEM_TIMER_IRQ_INTERRUPT_CONTROLLER_ID, SYSTEM_TIMER_IRQ, timer_isr, NULL, NULL);
}

static void timer_isr()
{
    IOWR_ALTERA_AVALON_TIMER_STATUS(SYSTEM_TIMER_BASE, 0);

    if (toggle)
    {
        IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, 0xFFFFF);
        toggle = 0;
    }
    else
    {
        IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, 0);
        toggle = 1;
    }
}

int main()
{
    clear_hex();
    timer_init();
    while (1)
    {
        // Main loop
    }
    return 0;
}
