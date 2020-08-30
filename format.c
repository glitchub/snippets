// Format strings in the style of printf, passing them character-by-character
// to the designated output function. This code is intended for use in embedded
// systems compiled with GCC.
//
// The following format sequences are supported:
//      %[flags][width]c                                -- argument is char
//      %[flags][width][.precision]s                    -- argument is pointer to char (NULL will be treated as "(null)")
//      %[flags][width][.precision][size]{i|d}          -- argument is signed int, printed as decimal with a sign
//      %[flags][width][.precision][size]{u|b|o|x|X}    -- argument is unsigned int, printed as decimal, binary, octal or hex
//
// Flags are zero or more of:
//      "-" : output is left-justified within specified width instead of right-justified
//      "0" : right-justified numbers have with leading "0"s instead of spaces
//      "+" : positive signed integers have a leading "+"
//      " " : positive signed integers have a leading space
//
// Width is minimum total number of characters to be output, padded with space
// or "0" as required. Specify "0" to "255", or "*" to use the low byte of the
// next int argument.
//
// Precision is maximum number of string characters to output (including 0), or
// mininum number of numeric digits to output (with leading "0"s as needed).
// Specify ".0" to ".255", or ".*" to use the low byte of the next int
// argument.
//
// Size is one of:
//      none : argument is an int
//      "l"  : argument is a long
//      "ll" : argument is a long long
// All other values must be cast to one of these.

// #include <stdarg.h>
#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

#ifdef PASS_OUTCHAR
// Outchar function is passed as a pointer, this increases stack but allows format()
// to be used for printf and sprintf-style functions.
void format(void (*outchar) (unsigned int), char *fmt, ...)
#else
// Outchar function is statically defined.
#ifdef POC
    // For demo purposes, we'll use putchar.
    extern int putchar(int);
    #define outchar(c) putchar(c)
#else
    // IRL you would define it to be a call to something like uart_tx().
    #error You must define outchar()!
#endif
void format(char *fmt, ...)
#endif
{
    va_list ap;
    va_start(ap, fmt);

    for (; *fmt; fmt++)
    {
        if (*fmt != '%')
        {
            outchar(*fmt);
            continue;
        }
        fmt++;

        // wedge format options into 32 bits
        struct
        {
            unsigned char left:1;       // true if flag '-'
            unsigned char zero:1;       // true if flag '0'
            unsigned char precise:1;    // true if precision was set
            unsigned char binary:1;     // true if decoding binary
            unsigned char octal:1;      // true if decoding octal
            unsigned char hex:1;        // true if decoding hex (if neither, then decimal)
            unsigned char lower:1;      // true if lowercase hex
            unsigned char space:1;      // true if sign is ' '
            unsigned char plus:1;       // true if sign is '+' (overrides space)
            unsigned char minus:1;      // true if sign is '-' (overrides plus and space)
            unsigned char size:2;       // 0 = int, 1 = long, 2 = long long
            unsigned char unused:4;
            unsigned char width;        // width 0-255
            unsigned char precision;    // precision 0-255
        } opts = {0};

        // flags
        while (1)
        {
            switch (*fmt)
            {
                case '-':               // align left
                    opts.left = 1;
                    break;
                case '0':               // pad with '0' instead of space
                    opts.zero = 1;
                    break;
                case '+':               // prefix positive signed with '+'
                    opts.plus = 1;
                    break;
                case ' ':               // prefix positive signed with ' '
                    opts.space = 1;
                    opts.plus = 0;
                    break;
                default:
                    goto endflags;
            }
            fmt++;
        }
        endflags:;

        // width 0 to 255
        if (*fmt == '*')
        {
            // use arg
            opts.width = va_arg(ap, unsigned int);
            fmt++;
        } else
            // inline
            while (*fmt >= '0' && *fmt <= '9') opts.width = (opts.width * 10) + (*fmt++ - '0');

        // precision 0 to 255
        if (*fmt == '.')
        {
            fmt++;
            if (*fmt == '*')
            {
                // use arg
                opts.precision = va_arg(ap, unsigned int);
                fmt++;
            } else
                // inline
                while (*fmt >= '0' && *fmt <= '9') opts.precision = (opts.precision * 10) + (*fmt++ - '0');
            opts.precise = 1; // remember precision is set, for %s"
        }

        // length, 'l' and 'll' only
        while (*fmt == 'l' && opts.size < 2)
        {
            opts.size++;
            fmt++;
        }

        if (*fmt == 'c')
        {
            // char
            if (!opts.left)
                while (opts.width-- > 1) outchar(' ');       // pad right before char
            outchar(va_arg(ap, int) & 0xff);                 // note char arg promotes to int
            if (opts.left)
                while (opts.width-- > 1) outchar(' ');       // pad left after char
        } else
        if (*fmt == 's')
        {
            // string
            unsigned char chars = 0;
            char *s = va_arg(ap, char *);
            if (!s)
                s = "(null)";
            while (s[chars] && chars < 255) chars++;        // 255 chars max
            if (opts.precise && opts.precision < chars)
                chars = opts.precision;                     // truncate to precision, possibly 0
            if (opts.width > chars)
                opts.width -= chars;
            else
                opts.width = 0;
            if (!opts.left)
                while (opts.width--) outchar(' ');           // pad right before string
            while (chars--) outchar(*s++);
            if (opts.left)
                while (opts.width--) outchar(' ');           // pad left after string
        } else
        {
            // must be numeric
            unsigned long long number;

            // prepare for decimal       18446744073709551615
            unsigned long long divisor = 10000000000000000000ULL;
            unsigned char digits = 20;

            switch (*fmt)
            {
                case 'i':
                case 'd':
                {
                    // get signed number
                    switch (opts.size)
                    {
                        case 0:  number = (long long)va_arg(ap, int); break;
                        case 1:  number = (long long)va_arg(ap, long int); break;
                        default: number = (long long)va_arg(ap, long long int); break;
                    }

                    // if negative, set sign char and make it unsigned
                    if ((long long)number < 0)
                    {
                        opts.minus = 1;
                        number = -(long long)number;
                    }
                    break;
                }

                case 'b':
                    opts.binary = 1;
                    divisor = 0x8000000000000000ULL;
                    digits = 64;
                    goto getnum;

                case 'o':
                    opts.octal = 1;
                    //         1777777777777777777777
                    divisor = 01000000000000000000000ULL;
                    digits = 22;
                    goto getnum;

                case 'x':
                    opts.lower = 1;
                    // fall thru

                case 'X':
                    opts.hex = 1;
                    //          ffffffffffffffff
                    divisor = 0x1000000000000000ULL;
                    digits = 16;
                    goto getnum;

                case 'u':
                    // fall thru

                getnum:
                    // get unsigned number
                    opts.plus = 0; // ignore flags
                    opts.space = 0;
                    switch (opts.size)
                    {
                        case 0:  number = va_arg(ap, unsigned int); break;
                        case 1:  number = va_arg(ap, unsigned long int); break;
                        default: number = va_arg(ap, unsigned long long int); break;
                    }
                    break;

                default:
                    // ignore others
                    continue;
            }

            // shift divisor to first non zero
            #define RADIX (opts.binary ? 2 : opts.octal ? 8 : opts.hex ? 16 : 10)
            #define SIGN (opts.minus ? '-' : opts.plus ? '+' : opts.space ? ' ' : 0)
            while (digits > 1 && divisor > number)
            {
                divisor /= RADIX;
                digits--;
            }

            if (opts.precision < digits)
                opts.precision = digits;                    // minimum precision is number of digits

            if (opts.width && SIGN) opts.width--;           // adjust width for sign

            if (opts.width > opts.precision)                // adjust width for precision
                opts.width -= opts.precision;
            else
                opts.width = 0;

            if (opts.zero && SIGN) outchar(SIGN);            // put sign before zeros

            if (!opts.left)
                while (opts.width--)
                    outchar(opts.zero ? '0' : ' ');          // pad to width

            if (!opts.zero && SIGN) outchar(SIGN);           // put sign after spaces

            while (opts.precision-- > digits) outchar('0');  // pad to precision

            while (digits--)                                 // output digits in left-to-right order
            {
                unsigned char n = (number / divisor) % RADIX;
                outchar((n < 10) ? n+'0' : opts.lower ? n-10+'a' : n-10+'A');
                divisor /= RADIX;
            }

            if (opts.left)
                while (opts.width--) outchar(' ');           // right pad always uses space
        }
    }
    va_end(ap);
}

#ifdef POC
// If compiled with 'CFLAGS=-DPOC make format', create an executable that
// exercises the various format options and word sizes.
#include <limits.h>

#ifdef PASS_OUTCHAR
// The outchar function is passed as a parameter, for this demo we'll use
// putchar (note we need to cast it).
#include <stdio.h>
#define dprintf(...) format((void(*)(unsigned int))putchar,__VA_ARGS__)
#else
// The outchar function is statically defined, we don't do anything special
// here.
#define dprintf(...) format(__VA_ARGS__)
#endif

// test values
#define CHR     'x'
#define STR     "abcdefghijklmnop"
#define POS     INT_MAX
#define LLPOS   LLONG_MAX
#define NEG     INT_MIN
#define LLNEG   LLONG_MIN
#define UNS     UINT_MAX
#define LLUNS   ULLONG_MAX

int main(void)
{
    dprintf("This is a test!\n");
    dprintf("Char       : '%c'\n", CHR);
    dprintf("  Right    : '%25c'\n", CHR);
    dprintf("  Left     : '%-*c'\n", 25, CHR);                        // variable width

    dprintf("String     : '%s'\n", STR);
    dprintf("  Right    : '%25s'\n", STR);
    dprintf("    Prec 8 : '%25.*s'\n", 8, STR);                       // variable precision
    dprintf("    Prec 0 : '%25.0s'\n", STR);                          // should output 25 spaces only
    dprintf("  Left     : '%-25s'\n", STR);
    dprintf("    Prec 8 : '%-*.*s'\n", 25, 8, STR);                   // variable width and precision

    dprintf("Ints       : '%d' '+%d' '% d' '%d'\n", POS, POS, POS, NEG);
    dprintf("  Right    : '%25d' '%+25d' '% 25d' '%25d'\n", POS, POS, POS, NEG);
    dprintf("    Prec 23: '%25.23d' '%+25.23d' '% 25.23d' '%25.23d' \n", POS, POS, POS, NEG);
    dprintf("  Left     : '%-25d' '%+-25d' '% -25d' '%-25d'\n", POS, POS, POS, NEG);
    dprintf("    Prec 23: '%-25.23d' '%+-25.23d' '% -25.23d' '%-25.23d'\n", POS, POS, POS, NEG);

    dprintf("LL ints    : '%lld' '+%lld' '% lld' '%lld'\n", LLPOS, LLPOS, LLPOS, LLNEG);
    dprintf("  Right    : '%25lld' '%+25lld' '% 25lld' '%25lld'\n", LLPOS, LLPOS, LLPOS, LLNEG);
    dprintf("    Prec 23: '%25.23lld' '%+25.23lld' '% 25.23lld' '%25.23lld' \n", LLPOS, LLPOS, LLPOS, LLNEG);
    dprintf("  Left     : '%-25lld' '%+-25lld' '% -25lld' '%-25lld'\n", LLPOS, LLPOS, LLPOS, LLNEG);
    dprintf("    Prec 23: '%-25.23lld' '%+-25.23lld' '% -25.23lld' '%-25.23lld' \n", LLPOS, LLPOS, LLPOS, LLNEG);

    dprintf("Unsigned   : '%u' '%o' '%x' '%X'\n", UNS, UNS, UNS, UNS);
    dprintf("  Right    : '%25u' '%25o' '%25x' '%25X'\n", UNS, UNS, UNS, UNS);
    dprintf("    Prec 23: '%25.23u' '%+25.23o' '% 25.23x' '%25.23X' \n", UNS, UNS, UNS, UNS);
    dprintf("  Left     : '%-25u' '%+-25o' '% -25x' '%-25X'\n", UNS, UNS, UNS, UNS);
    dprintf("    Prec 23: '%-25.23u' '%+-25.23o' '% -25.23x' '%-25.23X' \n", UNS, UNS, UNS, UNS);

    dprintf("LL unsigned: '%llu' '%llo' '%llx' '%llX'\n", LLUNS, LLUNS, LLUNS, LLUNS);
    dprintf("  Right    : '%25llu' '%25llo' '%25llx' '%25llX'\n", LLUNS, LLUNS, LLUNS, LLUNS);
    dprintf("    Prec 23: '%25.23llu' '%+25.23llo' '% 25.23llx' '%25.23llX' \n", LLUNS, LLUNS, LLUNS, LLUNS);
    dprintf("  Left     : '%-25llu' '%+-25llo' '% -25llx' '%-25llX'\n", LLUNS, LLUNS, LLUNS, LLUNS);
    dprintf("    Prec 23: '%-25.23llu' '%+-25.23llo' '% -25.23llx' '%-25.23llX' \n", LLUNS, LLUNS, LLUNS, LLUNS);

    dprintf("Binary     : '%-32b' '%64llb'\n", 0x012345678, 0xDEADFACECAFEBABEULL);
}
#endif
