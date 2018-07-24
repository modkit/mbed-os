#include "mbed.h"

DigitalOut led1(P0_13);
DigitalOut col(P0_4, 0);

// main() runs in its own thread in the OS
int main() {
    while (true) {
        led1 = !led1;
        wait(0.5);
    }
}
