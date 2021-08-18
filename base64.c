// Base64 encode/decode

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1)

int main(int argc, char *argv[])
{
    int decode = 0;

    if (argc > 1)
    {
        if (strcmp(argv[1], "-d")) die("Usage:\n"
                                       "    base64 < binary > base64\n"
                                       "    base64 -d < base64 > binary\n");
        decode = 1;
    }

    // The base64 character set
    const char base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    if (decode)
    {
        // base64 to binary: aaaaaa bbbbbb cccccc dddddd -> aaaaaabb bbbbcccc ccdddddd
        int state = 0, next, c;
        while ((c = getchar()) >= 0)
        {
            char *p = strchr(base64, c);
            if (!c || !p) continue; // ignore invalid
            c = p - base64;         // convert to 0-63
            switch (state)
            {
                case 0: next = c << 2; state = 1; break;
                case 1: putchar(next | c >> 4); next = c << 4; state = 2; break;
                case 2: putchar(next | c >> 2); next = c << 6; state = 3; break;
                case 3: putchar(next | c); state = 0; break;
            }
        }
    }
    else
    {
        // binary to base64: aaaaaaaa bbbbbbbb cccccccc -> aaaaaa aabbbb bbbbcc cccccc
        int state = 0, next, c;
        while ((c = getchar()) >= 0)
        {
            switch(state)
            {
                case 0: putchar(base64[c >> 2]); next = (c & 3) << 4; state = 1; break;
                case 1: putchar(base64[next | c >> 4]); next = (c & 15) << 2; state = 2; break;
                case 2: putchar(base64[next | c >> 6]); putchar(base64[c & 63]); state = 0; break;
            }
        }

        if (state)
        {
            // final and pad to multiple of 4
            putchar(base64[next]);
            putchar('=');
            if (state == 1) putchar('=');
        }
    }
}
