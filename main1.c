#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include <system.h>
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"

#define MAX_ODD_NUMBER 9
#define MIN_ODD_NUMBER 1
// Function prototypes
void update_display(int number);
static void key0_isr();
static void key1_isr();
static void key2_isr();
static void key3_isr();
void delay(int delay);
void clear_hex_display();
static void handle_interrupts(void* context);
static void init_interrupt_pio();

volatile int edge_capture;
volatile  int interrupts_enabled = 1;
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
	IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, number);
}

/*void clear_hex_display()
{
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, 0xFF);
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, 0xFF);
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, 0xFF);
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, 0xFF);
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_4_BASE, 0xFF);
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_5_BASE, 0xFF);
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_6_BASE, 0xFF);
	IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_7_BASE, 0xFF);
}*/
static void key0_isr()
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
}
static void key1_isr()
{
	IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, 0x1);
	delay(5000000);
	printf("Interrupt occuring with key 1 \n");
	IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, 0x0);
	printf("Reset \n");
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_1_BASE, 0x0);
	IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_1_BASE);
}

static void key2_isr()
{
	IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, 0x2);
	delay(5000000);
	printf("Interrupt occuring with key 2 \n");
	IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, 0x1);
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_2_BASE, 0x0);
	IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_2_BASE);
}

static void key3_isr()
{
	printf("Interrupt detected with key 3 \n");
	printf("\n Enter a number for your current number (1 or 9) \n");
	IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, 0x4);
	int current_number;
	scanf("%d", &current_number);
	if(current_number != 1 && current_number != 9)
	{
		printf("Error! Your input is invalid. \n");
		key3_isr();
	}
	while (1)
	{
		int SW_in = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
		if(SW_in & 0x01)
		{
			update_display(current_number);
			current_number += 2;
			if(current_number > MAX_ODD_NUMBER)
			{
				current_number = MIN_ODD_NUMBER;
				update_display(current_number);
				break;
			}
			update_display(current_number);
			usleep(200000);
		}
		else
		{
			update_display(current_number);
			current_number -= 2;
			if(current_number < MIN_ODD_NUMBER)
			{
				current_number = MAX_ODD_NUMBER;
				update_display(current_number);
				break;
			}
			update_display(current_number);
			usleep(200000);
		}
	}
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
	//clear_hex_display();
	printf("Hello, World!\n");
    IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, 0xD);
    while (1)
    {
    	int in = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
    	if(in == 1)
    	{
    		init_interrupt_pio();
    		usleep(30000);
    	}
    	else
    	{
    		key0_isr();
    		usleep(30000);
    	}
    }
    return 0;
}
