// Query the tty on stdin and report current termios flags with descriptions from
// termios man page. Flags not supported by linux are not shown.

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <termios.h>

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1)

struct flags { char *name; int flag; int type; char *description; };

struct flags iflags[] =
{
    { "IGNBRK",  IGNBRK,    0, "Ignore BREAK condition on input." },
    { "BRKINT",  BRKINT,    0, "If IGNBRK is set, a BREAK is ignored.  If it is not set but BRKINT is set, then a BREAK causes the "
                               "input and output queues to be flushed, and if the terminal is the controlling terminal of a "
                               "foreground process group, it will cause a SIGINT to be sent to this foreground process group.  When "
                               "neither IGNBRK nor BRKINT are set, a BREAK reads as a null byte ('\\0'), except when PARMRK is set, "
                               "in which case it reads as the sequence \\377 \\0 \\0." },
    { "IGNPAR",  IGNPAR,    0, "Ignore framing and parity errors" },
    { "PARMRK",  PARMRK,    0, "If IGNPAR is not set, prefix a character with a parity error or framing error with \\377 \\0.  If "
                               "neither IGNPAR nor PARMRK is set, read a character with a parity error or framing error as \\0." },
    { "INPCK",   INPCK,     0, "Enable input parity checking." },
    { "ISTRIP",  ISTRIP,    0, "Strip off eighth bit." },
    { "INLCR",   INLCR,     0, "Translate NL to CR on input." },
    { "IGNCR",   IGNCR,     0, "Ignore carriage return on input." },
    { "ICRNL",   ICRNL,     0, "Translate carriage return to newline on input (unless IGNCR is set)." },
    { "IUCLC",   IUCLC,     0, "Map uppercase characters to lowercase on input." },
    { "IXON",    IXON,      0, "Enable XON/XOFF flow control on output." },
    { "IXANY",   IXANY,     0, "(XSI) Typing any character will restart stopped output.  (The default is to allow just the START "
                               "character to restart output.)" },
    { "IXOFF",   IXOFF,     0, "Enable XON/XOFF flow control on input" },
    { "IUTF8",   IUTF8,     0, "Input is UTF8; this allows character-erase to be correctly performed in cooked mode." },
    { NULL }
};

struct flags oflags[] =
{
    { "OPOST",   OPOST,     0, "Enable implementation-defined output processing." },
    { "OLCUC",   OLCUC,     0, "Map lowercase characters to uppercase on output." },
    { "ONLCR",   ONLCR,     0, "(XSI) Map NL to CR-NL on output." },
    { "OCRNL",   OCRNL,     0, "Map CR to NL on output." },
    { "ONOCR",   ONOCR,     0, "Don't output CR at column 0." },
    { "ONLRET",  ONLRET,    0, "Don't output CR." },
    { "OFILL",   OFILL,     0, "Send fill characters for delay, rather than using a timed delay." },
    { "NLDLY",   NLDLY,     1, "Newline delay mask.  Values are NL0 and NL1." },
    { "CRDLY",   CRDLY,     1, "Carriage return delay mask.  Values are CR0, CR1, CR2, or CR3." },
    { "TABDLY",  TABDLY,    1, "Horizontal tab delay mask.  Values are TAB0, TAB1, TAB2, TAB3 (or XTABS).  A value of TAB3, that is, "
                               "XTABS, expands tabs to spaces "
                               "(with tab stops every eight columns)." },
    { "VTDLY",   VTDLY,     1, "Vertical tab delay mask.  Values are VT0 or VT1." },
    { "FFDLY",   FFDLY,     1, "Form feed delay mask.  Values are FF0 or FF1." },
    { NULL }
};

struct flags cflags[] =
{
    { "CBAUD",   CBAUD,     1, "Baud speed mask (4+1 bits).  Some common values are 13 = 9600, 14 = 19200, 15 = 38400, 4097 = "
                               "19200, 4098 = 115200, 4099 = 230400." },
    { "CSIZE",   CSIZE,     1, "Character size mask.  Values are CS5, CS6, CS7, or CS8." },
    { "CSTOPB",  CSTOPB,    0, "Set two stop bits, rather than one." },
    { "CREAD",   CREAD,     0, "Enable receiver." },
    { "PARENB",  PARENB,    0, "Enable parity generation on output and parity checking for input." },
    { "PARODD",  PARODD,    0, "If set, then parity for input and output is odd; otherwise even parity is used." },
    { "HUPCL",   HUPCL,     0, "Lower modem control lines after last process closes the device (hang up)." },
    { "CLOCAL",  CLOCAL,    0, "Ignore modem control lines." },
    { "CMSPAR",  CMSPAR,    0, "Use \"stick\" (mark/space) parity (supported on certain serial devices): if PARODD is set, the parity "
                               "bit is always 1; if PARODD is not set, then the parity bit is always 0)." },
    { "CRTSCTS", CRTSCTS,   0, "Enable RTS/CTS (hardware) flow control." },
    { NULL }
};

struct flags lflags[] =
{
    { "ISIG",    ISIG,      0, "When any of the characters INTR, QUIT, SUSP, or DSUSP are received, generate the corresponding "
                               "signal." },
    { "ICANON",  ICANON,    0, "Enable canonical mode" },
    { "ECHO",    ECHO,      0, "Echo input characters." },
    { "ECHOE",   ECHOE,     0, "If ICANON is also set, the ERASE character erases the preceding input character, and WERASE erases "
                               "the preceding word." },
    { "ECHOK",   ECHOK,     0, "If ICANON is also set, the KILL character erases the current line." },
    { "ECHONL",  ECHONL,    0, "If ICANON is also set, echo the NL character even if ECHO is not set" },
    { "ECHOCTL", ECHOCTL,   0, "If ECHO is also set, terminal special characters other than TAB, NL, START, and STOP are "
                               "echoed as ^X, where X is the character with ASCII code 0x40 greater than the special character.  For "
                               "example, character 0x08 (BS) is echoed as ^H." },
    { "ECHOPRT", ECHOPRT,   0, "If ICANON and ECHO are also set, characters are printed as they are being erased." },
    { "ECHOKE",  ECHOKE,    0, "If ICANON is also set, KILL is echoed by erasing each character on the line, as specified by ECHOE "
                               "and ECHOPRT." },
    { "NOFLSH",  NOFLSH,    0, "Disable flushing the input and output queues when generating signals for the INT, QUIT, and SUSP "
                               "characters." },
    { "TOSTOP",  TOSTOP,    0, "Send the SIGTTOU signal to the process group of a background process which tries to write to its "
                               "controlling terminal." },
    { "IEXTEN",  IEXTEN,    0, "Enable implementation-defined input processing.  This flag, as well as ICANON must be enabled for "
                               "the special characters EOL2, LNEXT, REPRINT, WERASE to be interpreted, and for the IUCLC flag to be "
                               "effective." },
    { NULL },
};

struct flags cc[] =
{
    { "VEOF",     VEOF,     2, "End-of-file character.  More precisely: this character causes the pending tty buffer to be sent "
                               "to the waiting user program without waiting for end-of-line.  If it is the first character of the "
                               "line, the read(2) in the user program returns 0, which signifies end-of-file.  Recognized when "
                               "ICANON is set, and then not passed as input." },
    { "VEOL",     VEOL,     2, "Additional end-of-line character.  Recognized when ICANON is set." },
    { "VEOL2",    VEOL2,    2, "Yet another end-of-line character.  Recognized when ICANON is set." },
    { "VERASE",   VERASE,   2, "Erase character.  This erases the previous not-yet-erased character, but does not erase past EOF "
                               "or beginning-of-line.  Recognized when ICANON is set, and then not passed as input." },
    { "VINTR",    VINTR,    2, "Interrupt character.  Send a SIGINT signal.  Recognized when ISIG is set, and then not passed as "
                               "input." },
    { "VKILL",    VKILL,    2, "Kill character.  This erases the input since the last EOF or beginning-of-line.  Recognized when "
                               "ICANON is set, and then not passed as input." },
    { "VLNEXT",   VLNEXT,   2, "Literal next.  Quotes the next input character, depriving it of a possible special meaning.  "
                               "Recognized when IEXTEN is set, and then not passed as input." },
    { "VMIN",     VMIN,     3, "Minimum number of characters for noncanonical read" },
    { "VQUIT",    VQUIT,    2, "Quit character.  Send SIGQUIT signal.  Recognized when ISIG is set, and then not passed as input." },
    { "VREPRINT", VREPRINT, 2, "Reprint unread characters.  Recognized when ICANON and IEXTEN are set, and then not passed as "
                               "input." },
    { "VSTART",   VSTART,   2, "Start character.  Restarts output stopped by the Stop character.  Recognized when IXON is set, "
                               "and then not passed as input." },
    { "VSTOP",    VSTOP,    2, "Stop character.  Stop output until Start character typed.  Recognized when IXON is set, and then "
                               "not passed as input." },
    { "VSUSP",    VSUSP,    2, "Suspend character.  Send SIGTSTP signal.  Recognized when ISIG is set, and then not passed as "
                               "input." },
    { "VTIME",    VTIME,    3, "Timeout in deciseconds for noncanonical read" },
    { "VWERASE",  VWERASE,  2, "Word erase.  Recognized when ICANON and IEXTEN are set, and then not passed as input." },
    { 0 },
};

void putwrap(char *s, char *prefix, int width)
{
    char *pf = "";
    while(*s)
    {
        char *p;
        for (p = s; p - s < width - 1; p++)
            if (!*p) { printf("%s%s\n", pf, s); return; }
        for (; !isspace(*p); p--)
            if (p == s) { p = s + width - 1; break; }
        printf("%s%.*s\n", pf, (p - s) + 1, s);
        s = p+1;
        while (isspace(*s)) s++;
        pf = prefix;
    }
}

void showflags(char *name, struct flags *flags, void *value, int width)
{
    int v;

    printf("%s:\n", name);
    for (; flags->name; flags++)
    {
        switch(flags->type)
        {
            case 0:
                // masked boolean
                printf("  %-8s = %4s : ", flags->name, *(tcflag_t *)value & flags->flag ? "yes" : "no");
                break;
            case 1:
                // masked numeric
                printf("  %-8s = %4d : ", flags->name, *(tcflag_t *)value & flags->flag >> __builtin_ctz(flags->flag));
                break;
            case 2:
                // indexed unsigned char as ^X or numeric
                v = *((cc_t *)value+flags->flag);
                if (v > 0 && v < 32)
                    printf("  %-8s =   ^%c : ", flags->name, v+64);
                else
                    printf("  %-8s = %4d : ", flags->name, v);
                break;
            case 3:
                // indexed unsigned char as numeric
                printf("  %-8s = %4d : ", flags->name, *((cc_t *)value+flags->flag));
                break;
        }
        putwrap(flags->description, "                  : ", width-20);
    }
}

int main(void)
{
    int width = 80;
    struct winsize ws;
    if (!ioctl(0, TIOCGWINSZ, &ws)) width = ws.ws_col;

    struct termios t;
    if (tcgetattr(0, &t)) die("Failed to get termios attributes for stdin.\n");

    showflags("c_iflag", iflags, &t.c_iflag, width);
    showflags("\nc_oflag", oflags, &t.c_oflag, width);
    showflags("\nc_cflag", cflags, &t.c_cflag, width);
    showflags("\nc_lflag", lflags, &t.c_lflag, width);
    showflags("\nc_cc", cc, &t.c_cc, width);
}
