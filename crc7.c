// Return CRC7 of start CRC and given data, polynomial == 0x09 (MMC)
// See https://reveng.sourceforge.io/crc-catalogue/1-15.htm#crc.cat.crc-7-mmc
unsigned char crc7(unsigned char crc, unsigned char *data, int len)
{
    static const unsigned char table[256] =
    {
        0x00, 0x12, 0x24, 0x36, 0x48, 0x5A, 0x6C, 0x7E, 0x90, 0x82, 0xB4, 0xA6, 0xD8, 0xCA, 0xFC, 0xEE,
        0x32, 0x20, 0x16, 0x04, 0x7A, 0x68, 0x5E, 0x4C, 0xA2, 0xB0, 0x86, 0x94, 0xEA, 0xF8, 0xCE, 0xDC,
        0x64, 0x76, 0x40, 0x52, 0x2C, 0x3E, 0x08, 0x1A, 0xF4, 0xE6, 0xD0, 0xC2, 0xBC, 0xAE, 0x98, 0x8A,
        0x56, 0x44, 0x72, 0x60, 0x1E, 0x0C, 0x3A, 0x28, 0xC6, 0xD4, 0xE2, 0xF0, 0x8E, 0x9C, 0xAA, 0xB8,
        0xC8, 0xDA, 0xEC, 0xFE, 0x80, 0x92, 0xA4, 0xB6, 0x58, 0x4A, 0x7C, 0x6E, 0x10, 0x02, 0x34, 0x26,
        0xFA, 0xE8, 0xDE, 0xCC, 0xB2, 0xA0, 0x96, 0x84, 0x6A, 0x78, 0x4E, 0x5C, 0x22, 0x30, 0x06, 0x14,
        0xAC, 0xBE, 0x88, 0x9A, 0xE4, 0xF6, 0xC0, 0xD2, 0x3C, 0x2E, 0x18, 0x0A, 0x74, 0x66, 0x50, 0x42,
        0x9E, 0x8C, 0xBA, 0xA8, 0xD6, 0xC4, 0xF2, 0xE0, 0x0E, 0x1C, 0x2A, 0x38, 0x46, 0x54, 0x62, 0x70,
        0x82, 0x90, 0xA6, 0xB4, 0xCA, 0xD8, 0xEE, 0xFC, 0x12, 0x00, 0x36, 0x24, 0x5A, 0x48, 0x7E, 0x6C,
        0xB0, 0xA2, 0x94, 0x86, 0xF8, 0xEA, 0xDC, 0xCE, 0x20, 0x32, 0x04, 0x16, 0x68, 0x7A, 0x4C, 0x5E,
        0xE6, 0xF4, 0xC2, 0xD0, 0xAE, 0xBC, 0x8A, 0x98, 0x76, 0x64, 0x52, 0x40, 0x3E, 0x2C, 0x1A, 0x08,
        0xD4, 0xC6, 0xF0, 0xE2, 0x9C, 0x8E, 0xB8, 0xAA, 0x44, 0x56, 0x60, 0x72, 0x0C, 0x1E, 0x28, 0x3A,
        0x4A, 0x58, 0x6E, 0x7C, 0x02, 0x10, 0x26, 0x34, 0xDA, 0xC8, 0xFE, 0xEC, 0x92, 0x80, 0xB6, 0xA4,
        0x78, 0x6A, 0x5C, 0x4E, 0x30, 0x22, 0x14, 0x06, 0xE8, 0xFA, 0xCC, 0xDE, 0xA0, 0xB2, 0x84, 0x96,
        0x2E, 0x3C, 0x0A, 0x18, 0x66, 0x74, 0x42, 0x50, 0xBE, 0xAC, 0x9A, 0x88, 0xF6, 0xE4, 0xD2, 0xC0,
        0x1C, 0x0E, 0x38, 0x2A, 0x54, 0x46, 0x70, 0x62, 0x8C, 0x9E, 0xA8, 0xBA, 0xC4, 0xD6, 0xE0, 0xF2
    };

    while (len--) crc = table[*data++ ^ crc];
    return crc;
}

// Print CRC7 of data on stdin. Example:
//  $ echo -n 123456789 | ./crc7
//  CRC8 is 0x75
#include <stdio.h>
#include <unistd.h>
int main(void)
{
    unsigned char data[64];     // size is arbitrary
    int len;

    unsigned char crc = 0x00;

    while ((len = read(0, data, sizeof(data))) > 0) crc = crc7(crc, data, len);

    printf("CRC7 is 0x%02X\n", crc >> 1); // shift right to normalize

    return 0;
}
