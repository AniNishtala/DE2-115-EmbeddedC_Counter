#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include <system.h>
#include <altera_avalon_pio_regs.h>
#include <sys/alt_irq.h>
#include <altera_avalon_timer_regs.h>

// Global variables
volatile int edge_capture;
char LUT[16] = {0x40, 0x79, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0x78, 0x80, 0x90, 0x88, 0x83, 0xA7, 0xA1, 0x86, 0x8E};
int tens_hex = 0;
int ones_hex = 0;
int incrementflag = 1;
int counter = 0;

// Function declarations
static void timer_init();
static void timer_isr();
void clear_hex();
void pio_init();
void toggle_timer_isr();
void handle_key_interrupts(void* context);
void write_to_seven(int count);

int main()
{
    clear_hex();
    timer_init();
    pio_init();
    while (1) {
        write_to_seven(counter);
    }
    return 0;
}

void write_to_seven(int count) {
    tens_hex = 0;
    ones_hex = 0;

    while ((count > 10) | (count == 10)) {
        count = count - 10;
        tens_hex++;
    }

    while (count > 0) {
        count = count - 1;
        ones_hex++;
    }

    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, LUT[ones_hex]);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, LUT[tens_hex]);
}

static void timer_init() {
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

static void timer_isr() {
    // Reset status
    IOWR_ALTERA_AVALON_TIMER_STATUS(SYSTEM_TIMER_BASE, 0);

    if (incrementflag) {
        if (counter % 2 == 0)
            counter++;
        else
            counter = counter + 2;
    } else {
        if (counter % 2 == 0)
            counter = counter - 1;
        else
            counter = counter - 2;
    }

    if (counter >= 15)
        incrementflag = 0; // Start decrementing
    if (counter <= 0)
        incrementflag = 1; // Start incrementing
}

void pio_init() {
    void* edge_capture_ptr = (void*)&edge_capture;

    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEYS_BASE, 7);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0x0);

    alt_ic_isr_register(KEYS_IRQ_INTERRUPT_CONTROLLER_ID, KEYS_IRQ, handle_key_interrupts, edge_capture_ptr, 0x00);
}

void handle_key_interrupts(void* context) {
    volatile int* edge_capture_ptr = (volatile int*) context;
    *edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE);

    if (*edge_capture_ptr & 1 || *edge_capture_ptr & 2 || *edge_capture_ptr & 4 || *edge_capture_ptr & 7) {
        toggle_timer_isr();
    }

    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0x0);
    IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE);
}

void toggle_timer_isr() {
    if (IORD_ALTERA_AVALON_TIMER_STATUS(SYSTEM_TIMER_BASE) & 2)
        IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 8); // Stop timer
    else
        IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 7); // Start timer
}

void clear_hex() {
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_4_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_5_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_6_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_7_BASE, 0xFF);
}
