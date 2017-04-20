#define DUE

#define BAUD 9600

#define CS_EN LOW
#define H_EN LOW
#define DIR_ON LOW

#define GLOBAL_ADDR 0

/* number of channels being used */
#define NCHANNELS 6 


/* number of bytes in a channel command, preamble, post-amble, packet checksum */
#define CMD_LEN 3  
#define PRE_LEN 1
#define POST_LEN 1
#define PCK_CHK_LEN 1


#define PCK_OK 1

/* error codes for arduino to send back to PC */
#define PRE_ERR 1
#define POST_ERR 2
#define PCK_CHK_ERR 4

#define POL_B 3
#define ENA_B 2
#define UPD_B 1
#define DIR_B 0


#define START_DELAY 500     /* delay before first write to DAC */
#define DAC_ADC_DELAY 500   /* delay between write to DAC and read from ADC */
#define ADC_DAC_DELAY 500   /* delay between read from ADC and write to DAC */

#define DAC_SELECT  0b00000000    /* write to DAC A;          bit 15 */
#define INPUT_BUF   0b00000000    /* Vref input not buffered; bit 14 */
#define GAIN_SELECT 0b00100000    /* output gain = 1;         bit 13 */
#define PWR_DOWN    0b00010000    /* output power-down ctrl;  bit 12 */
