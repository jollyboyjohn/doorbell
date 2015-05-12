/* 
    Simple doorbell emulator
    Sends the appropriate signal to a wireless doorbell to make it ring
    Note: Don't run on an external shell - interrupts will cause havoc.
*/

#include "doorbell.h"
#include "hw.h" // My basic Raspberry Pi hardware library

static const int bcm_gpio = 18;

// volatile tells GCC that memory location data will change outside this scope
volatile unsigned *gpio;
volatile unsigned *intr;
volatile unsigned *time;

// Doorbell specification #1: rings
enum ringtones {
    DINGDONG = 3, // 1100
    BANJO,    // 0010
    PHONE,    // 1010
    CIRCUS,   // 0110
    BIGBEN,   // 1110
    TWINKLE,  // 0001
    WORLD,    // 1001
    DOGBARK = 11  // 1101
};
   
// Doorbell specification #2: data transmission 
static int tx_one [2] = { SHORT, LONG };
static int tx_zero [2] = { LONG, SHORT };

// My old doorbells code
static int uid = 108; // 001101100

// What I want to ring out
static int tone = DINGDONG; 

// Convert bits to appropriate radio pulses
void txPulses(int pattern[], int size, unsigned *start) {
    int i;
    char signal = 1;
    for (i=0; i<size; i++) {
        (*start) = time_wait(&time, pattern[i], start);

	if (signal) 
	    gpio_pull_high(&gpio, bcm_gpio);          
	else
	    gpio_pull_low(&gpio, bcm_gpio);          

        signal = !signal; // flip the bit
    }
}

// Convert hex data to bits
void txBits (int bits, int size, unsigned *start) {
    int b;
    for (b=0; b<size; b++) {
        if ( (bits) & (1<<(b)) )
            txPulses(tx_one, sizeof(tx_one)/sizeof(int), start);
        else
            txPulses(tx_zero, sizeof(tx_zero)/sizeof(int), start);
    }
}

// Function to send data
void sendCmd (void) {
    int i;
    // Variable to enforce strict timing
    unsigned start = 0; 

    // Send the same code <REPEAT> times
    for (i=0; i<REPEAT; i++) {
        txBits(uid, 9, &start);
        txBits(tone, 4, &start);
        start = time_wait(&time, SYNC, &start);
    }
}

int main(int argc, char **argv) {
    // wait for interrupts to finish
    sleep(1);

    // Prep the Pi for transmission: setup, prepare output, pause system
    hw_init(&gpio, &intr, &time);
    gpio_set_inputs(&gpio, bcm_gpio);
    gpio_set_outputs(&gpio, bcm_gpio);
    block_intr(&intr, 1);

    // Send the data
    sendCmd();

    // Unblock the Raspberry Pi
    block_intr(&intr, 0);

    return 0;
}
