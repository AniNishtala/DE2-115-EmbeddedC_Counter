#include <stdio.h>
#include <system.h>
#include <altera_avalon_pio_regs.h>
#include <alt_types.h>

int main()
{
    int in;
    int option;
    int hundreds = 0;
    int tens = 0;
    int ones = 0;
    char LUT[10] = {0x40, 0x79, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0x78, 0x80, 0x90};
    // Turning off all the Hex Displays
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_4_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_5_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_6_BASE, 0xFF);
    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_7_BASE, 0xFF);
    printf("Use the Switches (SW)(SW 7 - SW 0) to display 8-bit number on Seven-Segment Display. \n");
    printf("Unsigned Magnitude or Signed Magnitude (1 for unsigned, 2 for signed, 3 for both) \n");
    printf("Enter an option to have signed or unsigned number: ");
    scanf("%d", &option);

    if(option == 1)
    {
        printf("Press the reset button (Key_0) to reset the program.\n");
        while(1)
        {
            in = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
            in = in & 0xFF;
            hundreds = 0;
            tens = 0;
            ones = 0;

            IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, in);

            while((in > 100) || (in == 100))
            {
                in = in - 100;
                hundreds++;
            }

            while((in > 10) || (in == 10))
            {
                in = in - 10;
                tens++;
            }

            while(in > 0)
            {
                in = in - 1;
                ones++;
            }

            IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, LUT[ones]);
            IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, LUT[tens]);
            IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, LUT[hundreds]);
        }
    }
    else if(option == 2)
    {
        printf("Press the reset button (Key_0) to reset the program.\n");
        while(1)
        {
            in = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);

            if((in & 0x80) == 0x80)
            {
                IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_7_BASE, 0xBF);
                if(in != 0x80)
                {
                    in = (~in) + 1;
                    in = in & 0x7F;
                }
                else
                {
                    in = in & 0xFF;
                }
            }
            else
            {
                IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, LUT[0]);
                in = in & 0x7F;
            }

            hundreds = 0;
            tens = 0;
            ones = 0;
            IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, in);

            while((in > 100) || (in == 100))
            {
                in = in - 100;
                hundreds++;
            }

            while((in > 10) || (in == 10))
            {
                in = in - 10;
                tens++;
            }

            while(in > 0)
            {
                in = in - 1;
                ones++;
            }

            IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_4_BASE, LUT[ones]);
            IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_5_BASE, LUT[tens]);
            IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_6_BASE, LUT[hundreds]);
        }
    }
    else if(option == 3)
    {
    	printf("Press the reset button (Key_0) to reset the program.\n");
        while(1)
        {
            in = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
            in = in & 0xFF;
            hundreds = 0;
            tens = 0;
            ones = 0;

            IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, in);

            while((in > 100) || (in == 100))
            {
                in = in - 100;
                hundreds++;
            }

            while((in > 10) || (in == 10))
            {
                in = in - 10;
                tens++;
            }

            while(in > 0)
            {
                in = in - 1;
                ones++;
            }

            IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_0_BASE, LUT[ones]);
            IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_1_BASE, LUT[tens]);
            IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_2_BASE, LUT[hundreds]);
            //------------------------
                in = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);

                if((in & 0x80) == 0x80)
                {
                    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_7_BASE, 0xBF);
                    if(in != 0x80)
                    {
                        in = (~in) + 1;
                        in = in & 0x7F;
                    }
                    else
                    {
                        in = in & 0xFF;
                    }
                }
                else
                {
                    IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_3_BASE, LUT[0]);
                    in = in & 0x7F;
                }

                hundreds = 0;
                tens = 0;
                ones = 0;
                IOWR_ALTERA_AVALON_PIO_DATA(RED_LEDS_BASE, in);

                while((in > 100) || (in == 100))
                {
                    in = in - 100;
                    hundreds++;
                }

                while((in > 10) || (in == 10))
                {
                    in = in - 10;
                    tens++;
                }

                while(in > 0)
                {
                    in = in - 1;
                    ones++;
                }

                IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_4_BASE, LUT[ones]);
                IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_5_BASE, LUT[tens]);
                IOWR_ALTERA_AVALON_PIO_DATA(SEVEN_SEG_6_BASE, LUT[hundreds]);
            }
        }
    else
    {
        printf("Invalid option entered.\n\n");
        printf("Press the reset button (Key_0) to reset the program.\n");
    }

    return 0;
}
