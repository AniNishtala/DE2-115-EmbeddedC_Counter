#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include <system.h>
#include <math.h>
#include <altera_avalon_pio_regs.h>
#include <sys/alt_irq.h>
#include <altera_avalon_timer_regs.h>
#include <stdlib.h>

void clear_hex();
void start_on_hex();
static void timer_init(int ms);
void timer_isr();
void pio_init();
void handle_key_interrupts(void* context);
void stop_timer();
void binary_to_decimal_seven_seg(int num);
void up_timer_seven_segment();
void toggle_red_leds();
void toggle_green_leds();
int random_num_generator();
void clear_all_leds();
void fail_on_hex();
void opps_on_hex();
void checking_timer_after_game();

int counter = 0;
int tens_hex;
int ones_hex;
int hundreds_hex;
int thousands_hex;
int timer_isr_choice;
int red_leds_toggle = 0;
int green_leds_toggle = 0;
int random_time_before_leds_flash;
int ms_period = 1000;
int red_led_counter = 0; // will be used to flash LEDS when timer is 1ms
char hex_table[16] = {0x40, 0x79, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0x78, 0x80, 0x90, 0x88, 0x83, 0xA7, 0xA1, 0x86, 0x8E};
volatile int edge_capture;

int main() {
    clear_hex();
    clear_all_leds();
    start_on_hex();
    srand(time(NULL));
    printf("Press KEY 3 to start the game\n");
    printf("Press KEY 2 once the Red LEDs turn on\n");
    printf("Press KEY 1 to reset the game\n");
    pio_init();
    while (1) {}
    return 0;
}

void binary_to_decimal_seven_seg(int num) {
    thousands_hex = 0;
    hundreds_hex = 0;
    tens_hex = 0;
    ones_hex = 0;
    int count = num;

    while (count >= 1000) {
        count -= 1000;
        thousands_hex++;
    }
    while (count >= 100) {
        count -= 100;
        hundreds_hex++;
    }
    while (count >= 10) {
        count -= 10;
        tens_hex++;
    }
    while (count > 0) {
        count -= 1;
        ones_hex++;
    }

    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, hex_table[ones_hex]);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, hex_table[tens_hex]);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, hex_table[hundreds_hex]);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, hex_table[thousands_hex]);
}

void pio_init() {
    void* edge_capture_ptr = (void*)&edge_capture;
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEYS_BASE, 7);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0x0);
    alt_ic_isr_register(KEYS_IRQ_INTERRUPT_CONTROLLER_ID, KEYS_IRQ, handle_key_interrupts, edge_capture_ptr, 0x00);
}

static void timer_init(int ms) {
    // setting the stop bit high
    IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 8);
    // period_multiplier is initially set to 1000
    int period = SYSTEM_TIMER_LOAD_VALUE * ms;
    int period_h = period >> 16;
    int period_l = period & 0xFFFF;
    IOWR_ALTERA_AVALON_TIMER_PERIODL(SYSTEM_TIMER_BASE, period_l);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(SYSTEM_TIMER_BASE, period_h);
    IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 7);
    alt_ic_isr_register(SYSTEM_TIMER_IRQ_INTERRUPT_CONTROLLER_ID, SYSTEM_TIMER_IRQ, timer_isr, NULL, NULL);
}

void timer_isr() {
    IOWR_ALTERA_AVALON_TIMER_STATUS(SYSTEM_TIMER_BASE, 0);

    switch (timer_isr_choice) {
        case 1:
            toggle_red_leds();
            ms_period = 1;
            timer_isr_choice = 2;
            timer_init(ms_period);
            break;
        case 2:
            if (red_led_counter == 150) { // this portion of code is used to flash the red leds
                toggle_red_leds();
                red_led_counter = 0;
            } else {
                red_led_counter++;
            }
            up_timer_seven_segment();
            break;
        case 3:
            clear_all_leds();
            toggle_green_leds();
            break;
        case 4:
            clear_all_leds();
            toggle_red_leds();
            break;
    }
}

void up_timer_seven_segment() {
    // reset status
    IOWR_ALTERA_AVALON_TIMER_STATUS(SYSTEM_TIMER_BASE, 0);
    if (counter > 5000) {
        counter = 0;
        stop_timer();
    } else {
        binary_to_decimal_seven_seg(counter); // printing ms time on the Hex displays
        counter++;
    }
}

void handle_key_interrupts(void* context) {
    volatile int* edge_capture_ptr = (volatile int*)context;
    *edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE);

    if (*edge_capture_ptr & 4) { // start button is key3
        timer_isr_choice = 1;
        clear_hex();
        ms_period = ((rand() % 13) + 2) * 1000;
        timer_init(ms_period);
    } else if (*edge_capture_ptr & 2) {
        stop_timer();
        checking_timer_after_game();
    } else if (*edge_capture_ptr & 1) {
        stop_timer();
        start_on_hex();
        clear_all_leds();
        counter = 0;
    }

    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE, 0x0);
    IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEYS_BASE);
}

void checking_timer_after_game() {
    if (counter == 0) {
        opps_on_hex();
        timer_isr_choice = 4;
        timer_init(150);
    } else if (counter <= 1000) {
        timer_isr_choice = 3; // timer isr 3 is green leds flash and success
        timer_init(150);
    } else {
        fail_on_hex();
        timer_isr_choice = 4;
        timer_init(150);
    }
}

void stop_timer() {
    IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 8);
}

void start_timer() {
    IOWR_ALTERA_AVALON_TIMER_CONTROL(SYSTEM_TIMER_BASE, 7);
}

void toggle_red_leds() {
    if (red_leds_toggle == 1) {
        red_leds_toggle = 0;
        IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, 0);
    } else {
        red_leds_toggle = 1;
        IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, 0xFFFFF);
    }
}

void toggle_green_leds() {
    if (green_leds_toggle == 1) {
        green_leds_toggle = 0;
        IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, 0);
    } else {
        green_leds_toggle = 1;
        IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, 0xFFFFF);
    }
}

int random_num_generator() {
    int num = 0;
    srand(17);
    while (num <= 1) {
        num = rand() % 16;
    }
    return num;
}

void start_on_hex() {
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, 0x07);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, 0x2f);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, 0x08);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, 0x07);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_4_BASE, 0x12);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_5_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_6_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_7_BASE, 0xFF);
}

void fail_on_hex() {
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, 0x47);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, 0x79);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, 0x08);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, 0x0e);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_4_BASE, 0xff);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_5_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_6_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_7_BASE, 0xFF);
}

void opps_on_hex() {
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, 0x12);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, 0x0c);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, 0x0c);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, 0x40);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_4_BASE, 0xff);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_5_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_6_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_7_BASE, 0xFF);
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

void clear_all_leds() {
    IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(GREEN_LEDS_BASE, 0);
}
