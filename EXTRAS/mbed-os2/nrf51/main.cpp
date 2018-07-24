#include "mbed.h"

DigitalOut myled(P0_13);
DigitalOut col(P0_4);

int main() {
    while(1) {
        myled = 1;
        wait(0.2);
        myled = 0;
        wait(0.2);
    }
}
