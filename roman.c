// Decimal-to-Roman, requires UTF-8
#include <stdio.h>
#include <stdlib.h>

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1)

#define M "\xe1\xb4\x8d" // U+1D0D, small cap M
#define D "\xe1\xb4\x85" // U+1D05, small cap D
#define C "\xe1\xb4\x84" // U+1D04, small cap C
#define L "\xca\x9f"     // U+029F, small cap L
#define X "x"            // no UTF, use lowercase ASCII
#define V "\xe1\xb4\xa0" // U+1D20, small cap V
#define I "\xc9\xaa"     // U+026A, small cap I
#define O "\xcc\x85"     // U+0305, combining overline (after the character that gets it)

// Interesting values and their associated UTF-8 strings
struct
{
    int value;
    char *utf8;
} roman[] =
{
    { 1000000, M O     },
    { 900000,  C O M O },
    { 500000,  D O     },
    { 400000,  C O D O },
    { 100000,  C O     },
    { 90000,   X O C O },
    { 50000,   L O     },
    { 40000,   X O L O },
    { 10000,   X O     },
    { 9000,    M X O   },
    { 5000,    V O     },
    { 4000,    M V O   },
    { 1000,    M       },
    { 900,     C M     },
    { 500,     D       },
    { 400,     C D     },
    { 100,     C       },
    { 90,      X C     },
    { 50,      L       },
    { 40,      X L     },
    { 10,      X       },
    { 9,       I X     },
    { 5,       V       },
    { 4,       I V     },
    { 1,       I       },
};

int main(int argc, char *argv[])
{
    int n;

    if (argc <= 1 || (n = atoi(argv[1])) <= 0)
        die("Please supply a positive non-zero argument\n");

    for (int i = 0; n > 0; i++)
    {
        while (n >= roman[i].value)
        {
            fputs(roman[i].utf8, stdout);
            n -= roman[i].value;
        }
    }
    fputc('\n', stdout);
}
