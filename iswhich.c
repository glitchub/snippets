#include <ctype.h>
#include <stdio.h>

// show which ASCII characters are affected by which ctype function

#define yn(x) (x)?"yes":"no"

char *ctrls[32] = { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS",
                    "HT", "LF", "VT", "FF", "CR", "SO", "SI", "DLE", "DC1", "DC2",
                    "DC3", "DC4", "NAK", "SYN", "ETB", "CAN", "EM", "SUB", "ESC",
                    "FS", "GS", "RS", "US"};

int main(void)
{

    for (int c=0; c<128; c++)
    {
        if (!(c & 31))
            printf("\tisalnum\tisalpha\tisblank\tiscntrl\tisdigit\tisgraph\tislower\tisprint\tispunct\tisspace\tisupper\tisxdigit\n");

        if (c <= 31)
            printf("%02X %s", c, ctrls[c]);
        else if (c == 127)
            printf("%02X DEL", c);
        else
            printf("%02X '%c'", c, c);

        printf("\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
            yn(isalnum(c)), yn(isalpha(c)), yn(isblank(c)), yn(iscntrl(c)), yn(isdigit(c)), yn(isgraph(c)),
            yn(islower(c)), yn(isprint(c)), yn(ispunct(c)), yn(isspace(c)), yn(isupper(c)), yn(isxdigit(c)));
    }

}
