#include <stdio.h>
#include <unistd.h>
#include <altera_avalon_pio_regs.h>
#include <sys/alt_irq.h>
#include <system.h>
#include <alt_types.h>
#include <altera_avalon_timer_regs.h>
#include <altera_avalon_jtag_uart_regs.h>

static void timer_init();
static void timer_isr(void *context);
static void switch_isr(void *context);
void clear_hex();
void update_display(int time);
void send_message(const char *message);

volatile int elapsed_time = 0;
volatile int running = 0;
static const char LUT[36] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, // 0-9
    0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E, 0xC2, 0x89, 0xCF, 0xE1, // A-I
    0x8A, 0xC7, 0xAA, 0xC8, 0xA3, 0x8C, 0x98, 0xAF, 0x92, 0x87, // J-R
    0xC1, 0xE3, 0xE2, 0x9B, 0x91, 0xA4                          // S-Z
};
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

void update_display(int time)
{
    int seconds = time % 60;
    int tens = seconds / 10;
    int ones = seconds % 10;

    char ones_display = LUT[ones];
    char tens_display = LUT[tens];
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, ones_display);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, tens_display);
}

void send_message(const char *message)
{
    while (*message)
    {
        IOWR_ALTERA_AVALON_JTAG_UART_DATA(JTAGUART_0_BASE, *message++);
    }
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

static void timer_isr(void *context)
{
    IOWR_ALTERA_AVALON_TIMER_STATUS(SYSTEM_TIMER_BASE, 0);
    if (running)
    {
        elapsed_time++;
        update_display(elapsed_time);
    }
}

static void switch_isr(void *context)
{
    int switches = IORD_ALTERA_AVALON_PIO_EDGE_CAP(SWITCHES_BASE);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SWITCHES_BASE, 0); // Clear the edge capture register

    if (switches & 0x1) // Start/Stop switch
    {
        running = !running;
        if (running)
        {
            IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, 0x1); // Turn on the running state LED
            send_message("Stopwatch started\n");
            elapsed_time++;
            update_display(elapsed_time);
        }
        else
        {
            IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, 0x0); // Turn off the running state LED
            send_message("Stopwatch stopped\n");
        }
    }

    if ((switches & 0x2) || (switches & 0x3)) // Reset switch
    {
        elapsed_time = 0;
        update_display(elapsed_time);
        send_message("Stopwatch reset\n");
    }
}

int main()
{
    clear_hex();
    timer_init();

    // Initialize switches for interrupts
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(SWITCHES_BASE, 0x3); // Enable interrupts for the first two switches
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SWITCHES_BASE, 0x0); // Clear the edge capture register
    alt_ic_isr_register(SWITCHES_IRQ_INTERRUPT_CONTROLLER_ID, SWITCHES_IRQ, switch_isr, NULL, NULL);

    while (1)
    {
        // Main loop
    }
    return 0;
}
