#include "addr.h"
void addr_init(void)
{
    //иою╜
    //P02 P03 P12 P13 P14 P15
    P0PU |= 0x01 << 2;
    P0PU |= 0x01 << 3;
    P1PU |= 0x01 << 2;
    P1PU |= 0x01 << 3;
    P1PU |= 0x01 << 4;
    P1PU |= 0x01 << 5;
}


unsigned char addr_read(void)
{
    unsigned char addr = 0;
    addr |= ((unsigned char)(!P15)) << 5;
    addr |= ((unsigned char)(!P14)) << 4;
    addr |= ((unsigned char)(!P13)) << 3;
    addr |= ((unsigned char)(!P12)) << 2;
    addr |= ((unsigned char)(!P03)) << 1;
    addr |= ((unsigned char)(!P02)) << 0;
    return addr;
}


