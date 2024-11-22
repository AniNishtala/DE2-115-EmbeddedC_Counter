#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include <system.h>
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"

#define MAX_NUMBER 99
#define MIN_NUMBER 0
// Function prototypes
void update_display(int number);
//static void key0_isr();
static void key1_isr();
static void key2_isr();
static void key3_isr();
void delay(int delay);
void clear_hex_display();
static void handle_interrupts(void* context);
static void init_interrupt_pio();

volatile int edge_capture;
volatile int current_number = 0;
volatile int step_size = 1; // Default step size to 1
char LUT[10] = {0x40, 0x79, 0xA4, 0xB0, 0x99, 0x92,0x82, 0x78,0x80,0x90};
void delay(int delay)
{
    while (delay > 0)
    {
        delay = delay - 1;
    }
}

void update_display(int number)
{
	int tens = number / 10;
	int units = number % 10;
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, LUT[units]);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, LUT[tens]);
}

void clear_hex_display()
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
/*static void key0_isr()
{
    void* edge_capture_ptr = (void*)&edge_capture;
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_1_BASE, 0x0); // Enable all 3 interrupts
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_2_BASE, 0x0);
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_3_BASE, 0x0);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_1_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_2_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_3_BASE, 0);
    alt_ic_isr_register(KEY_1_IRQ_INTERRUPT_CONTROLLER_ID, KEY_1_IRQ, handle_interrupts, edge_capture_ptr, 0x0);
    alt_ic_isr_register(KEY_2_IRQ_INTERRUPT_CONTROLLER_ID, KEY_2_IRQ, handle_interrupts, edge_capture_ptr, 0x0);
    alt_ic_isr_register(KEY_3_IRQ_INTERRUPT_CONTROLLER_ID, KEY_3_IRQ, handle_interrupts, edge_capture_ptr, 0x0);
}*/
static void key1_isr()
{
    current_number = 0;
    update_display(current_number);
    delay(5000000);
    printf("Reset to 00\n");
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_1_BASE, 0x0);
	IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_1_BASE);
}

static void key2_isr()
{
	current_number = (current_number + step_size) % (MAX_NUMBER + 1);
	update_display(current_number);
	printf("Incremented to %02d\n", current_number);
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_2_BASE, 0x0);
	IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_2_BASE);
}

static void key3_isr()
{
    current_number = (current_number - step_size) < MIN_NUMBER ? MAX_NUMBER : (current_number - step_size);
    update_display(current_number);
    printf("Decremented to %02d\n", current_number);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_3_BASE, 0x0);
    IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_3_BASE);
}
static void handle_interrupts(void* context)
{
    volatile int* edge_capture_ptr = (volatile int*) context;
    *edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_1_BASE) + 2 * (IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_2_BASE)) + 4 * (IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_3_BASE));
    //printf("Edge capture register: %x\n", *edge_capture_ptr); // Print the edge capture register value
    if(*edge_capture_ptr == 0x0)
    	return;
    if (*edge_capture_ptr == 0x1) {
        key1_isr();
    }
    if (*edge_capture_ptr == 0x2) {
    	key2_isr();
    }
    if (*edge_capture_ptr == 0x4) {
    	key3_isr();
    }

    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_1_BASE, 0); // Clear the edge capture register
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_2_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_3_BASE, 0);
}

static void init_interrupt_pio()
{
    void* edge_capture_ptr = (void*)&edge_capture;
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_1_BASE, 0x1); // Enable all 3 interrupts
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_2_BASE, 0x2);
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_3_BASE, 0x4);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_1_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_2_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_3_BASE, 0);
    alt_ic_isr_register(KEY_1_IRQ_INTERRUPT_CONTROLLER_ID, KEY_1_IRQ, handle_interrupts, edge_capture_ptr, 0x0);
    alt_ic_isr_register(KEY_2_IRQ_INTERRUPT_CONTROLLER_ID, KEY_2_IRQ, handle_interrupts, edge_capture_ptr, 0x0);
    alt_ic_isr_register(KEY_3_IRQ_INTERRUPT_CONTROLLER_ID, KEY_3_IRQ, handle_interrupts, edge_capture_ptr, 0x0);
}

int main()
{
	clear_hex_display();
	update_display(0);
	printf("Hello, World!\n");
    IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, 0xD);
    while (1)
    {
    	int sw = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
    	step_size = (sw & 0x01) ? 2 : 1;
    	printf("Step size set to %s\n", (step_size == 2) ? "even" : "odd");
    	IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, step_size);
    	if(sw == 1)
    	{
    		init_interrupt_pio();
    		usleep(30000);
    	}
    	else
    	{
    		//key0_isr();
    		usleep(30000);
    	}
    }
    return 0;
}
