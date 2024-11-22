#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include <system.h>
#include <math.h>
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "altera_avalon_timer_regs.h"

int red_led = 0;
int incrementing_decrementing_flag = 1;
int x;
int period_multipler = 1000;
volatile int edge_capture;

void clear_hex();
static void timer_init();
static void timer_isr();
void pio_init();
void handle_key_interrupts(void* context);
void toggle_timer();
void turn_on_timer();
void turn_off_timer();

int main()
{
    clear_hex();
    timer_init(period_multipler);
    pio_init();

    while(1)
    {
        // Uncommented section removed for brevity

        IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, x);
    }

    return 0;
}

static void timer_init(int period_multipler)
{
    // setting the stop bit high
    IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 8);

    // period_multipler is initially set to 1000
    int period = SYSTEM_TIMER_LOAD_VALUE * period_multipler;
    int period_h = period >> 16;
    int period_l = period & 0xFFFF;

    IOWR_ALTERA_AVALON_TIMER_PERIODL(SYSTEM_TIMER_BASE, period_l);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(SYSTEM_TIMER_BASE, period_h);

    IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 7);

    alt_ic_isr_register(SYSTEM_TIMER_IRQ_INTERRUPT_CONTROLLER_ID,
                        SYSTEM_TIMER_IRQ, timer_isr, NULL, NULL);
}

static void timer_isr()
{
    // reset status
    IOWR_ALTERA_AVALON_TIMER_STATUS(SYSTEM_TIMER_BASE, 0);

    if(incrementing_decrementing_flag)
    {
        x = pow(2, red_led);
        red_led++;

        if(red_led == 18)
            incrementing_decrementing_flag = 0;
    }
    else
    {
        x = x / 2;
        red_led = red_led - 1;

        if(red_led == 0)
            incrementing_decrementing_flag = 1;
    }
}

void pio_init()
{
    void* edge_capture_ptr = (void*)&edge_capture;

    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEYS_BASE, 7);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0x0);

    alt_ic_isr_register(KEYS_IRQ_INTERRUPT_CONTROLLER_ID, KEYS_IRQ,
                        handle_key_interrupts, edge_capture_ptr, 0x00);
}

void handle_key_interrupts(void* context)
{
    volatile int* edge_capture_ptr = (volatile int*) context;
    *edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE);

    if(*edge_capture_ptr & 4)
    {
        // sets the delay for 1s
        period_multipler = 1000;
        timer_init(period_multipler);
    }
    else if(*edge_capture_ptr & 2)
    {
        // sets the delay for 250 ms
        period_multipler = 250;
        timer_init(period_multipler);
    }
    else if((*edge_capture_ptr & 1) & (IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE) & 1))
    {
        toggle_timer();
    }
    else if(*edge_capture_ptr & 1)
    {
        // sets the delay for 125 ms
        period_multipler = 125;
        timer_init(period_multipler);
    }

    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0x0);
    IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE);
}

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

void toggle_timer()
{
    if(IORD_ALTERA_AVALON_TIMER_STATUS(SYSTEM_TIMER_BASE) & 2)
        IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 8);
    else
        IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 7);
}
