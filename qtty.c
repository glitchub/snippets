// Query the tty on stdin and report current termios flags with descriptions from
// termios man page. Flags not supported by linux are not shown.

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string.h>

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1)

struct flags { char *name; int flag; int type; char *description; };

// possible types
#define BIT 0 // flag is tcflag_t single bit mask
#define VAL 1 // flag is tcflag_t multi-bit mask
#define CHR 2 // flag is cc_t index to a character
#define NUM 3 // flag is cc_t index to a byte

struct flags iflags[] =
{
    { "IGNBRK",  IGNBRK,    BIT, "Ignore BREAK condition on input." },
    { "BRKINT",  BRKINT,    BIT, "If IGNBRK is set, a BREAK is ignored.  If it is not set but BRKINT is set, then a BREAK causes the "
                                 "input and output queues to be flushed, and if the terminal is the controlling terminal of a "
                                 "foreground process group, it will cause a SIGINT to be sent to this foreground process group.  When "
                                 "neither IGNBRK nor BRKINT are set, a BREAK reads as a null byte ('\\0'), except when PARMRK is set, "
                                 "in which case it reads as the sequence \\377 \\0 \\0." },
    { "IGNPAR",  IGNPAR,    BIT, "Ignore framing and parity errors." },
    { "PARMRK",  PARMRK,    BIT, "If IGNPAR is not set, prefix a character with a parity error or framing error with \\377 \\0.  If "
                                 "neither IGNPAR nor PARMRK is set, read a character with a parity error or framing error as \\0." },
    { "INPCK",   INPCK,     BIT, "Enable input parity checking." },
    { "ISTRIP",  ISTRIP,    BIT, "Strip off eighth bit." },
    { "INLCR",   INLCR,     BIT, "Translate NL to CR on input." },
    { "IGNCR",   IGNCR,     BIT, "Ignore carriage return on input." },
    { "ICRNL",   ICRNL,     BIT, "Translate carriage return to newline on input (unless IGNCR is set)." },
    { "IUCLC",   IUCLC,     BIT, "Map uppercase characters to lowercase on input." },
    { "IXON",    IXON,      BIT, "Enable XON/XOFF flow control on output." },
    { "IXANY",   IXANY,     BIT, "(XSI) Typing any character will restart stopped output.  (The default is to allow just the START "
                                 "character to restart output.)" },
    { "IXOFF",   IXOFF,     BIT, "Enable XON/XOFF flow control on input." },
    { "IUTF8",   IUTF8,     BIT, "Input is UTF8; this allows character-erase to be correctly performed in cooked mode." },
    { NULL }
};

struct flags oflags[] =
{
    { "OPOST",   OPOST,     BIT, "Enable implementation-defined output processing." },
    { "OLCUC",   OLCUC,     BIT, "Map lowercase characters to uppercase on output." },
    { "ONLCR",   ONLCR,     BIT, "(XSI) Map NL to CR-NL on output." },
    { "OCRNL",   OCRNL,     BIT, "Map CR to NL on output." },
    { "ONOCR",   ONOCR,     BIT, "Don't output CR at column 0." },
    { "ONLRET",  ONLRET,    BIT, "Don't output CR." },
    { "OFILL",   OFILL,     BIT, "Send fill characters for delay, rather than using a timed delay." },
    { "NLDLY",   NLDLY,     VAL, "Newline delay mask.  Values are NL0 and NL1." },
    { "CRDLY",   CRDLY,     VAL, "Carriage return delay mask.  Values are CR0, CR1, CR2, or CR3." },
    { "TABDLY",  TABDLY,    VAL, "Horizontal tab delay mask.  Values are TAB0, TAB1, TAB2, TAB3 (or XTABS).  A value of TAB3, that is, "
                                 "XTABS, expands tabs to spaces (with tab stops every eight columns)." },
    { "VTDLY",   VTDLY,     VAL, "Vertical tab delay mask.  Values are VT0 or VT1." },
    { "FFDLY",   FFDLY,     VAL, "Form feed delay mask.  Values are FF0 or FF1." },
    { NULL }
};

struct flags cflags[] =
{
    { "CBAUD",   CBAUD,     VAL, "Baud speed mask (4+1 bits).  Some common values are 13 = 9600, 14 = 19200, 15 = 38400, 4097 = "
                                 "19200, 4098 = 115200, 4099 = 230400." },
    { "CSIZE",   CSIZE,     VAL, "Character size mask.  Values are CS5, CS6, CS7, or CS8." },
    { "CSTOPB",  CSTOPB,    BIT, "Set two stop bits, rather than one." },
    { "CREAD",   CREAD,     BIT, "Enable receiver." },
    { "PARENB",  PARENB,    BIT, "Enable parity generation on output and parity checking for input." },
    { "PARODD",  PARODD,    BIT, "If set, then parity for input and output is odd; otherwise even parity is used." },
    { "HUPCL",   HUPCL,     BIT, "Lower modem control lines after last process closes the device (hang up)." },
    { "CLOCAL",  CLOCAL,    BIT, "Ignore modem control lines." },
    { "CMSPAR",  CMSPAR,    BIT, "Use \"stick\" (mark/space) parity (supported on certain serial devices): if PARODD is set, the parity "
                                 "bit is always 1; if PARODD is not set, then the parity bit is always 0)." },
    { "CRTSCTS", CRTSCTS,   BIT, "Enable RTS/CTS (hardware) flow control." },
    { NULL }
};

struct flags lflags[] =
{
    { "ISIG",    ISIG,      BIT, "When any of the characters INTR, QUIT, SUSP, or DSUSP are received, generate the corresponding "
                                 "signal." },
    { "ICANON",  ICANON,    BIT, "Enable canonical mode." },
    { "ECHO",    ECHO,      BIT, "Echo input characters." },
    { "ECHOE",   ECHOE,     BIT, "If ICANON is also set, the ERASE character erases the preceding input character, and WERASE erases "
                                 "the preceding word." },
    { "ECHOK",   ECHOK,     BIT, "If ICANON is also set, the KILL character erases the current line." },
    { "ECHONL",  ECHONL,    BIT, "If ICANON is also set, echo the NL character even if ECHO is not set." },
    { "ECHOCTL", ECHOCTL,   BIT, "If ECHO is also set, terminal special characters other than TAB, NL, START, and STOP are "
                                 "echoed as ^X, where X is the character with ASCII code 0x40 greater than the special character.  For "
                                 "example, character 0x08 (BS) is echoed as ^H." },
    { "ECHOPRT", ECHOPRT,   BIT, "If ICANON and ECHO are also set, characters are printed as they are being erased." },
    { "ECHOKE",  ECHOKE,    BIT, "If ICANON is also set, KILL is echoed by erasing each character on the line, as specified by ECHOE "
                                 "and ECHOPRT." },
    { "NOFLSH",  NOFLSH,    BIT, "Disable flushing the input and output queues when generating signals for the INT, QUIT, and SUSP "
                                 "characters." },
    { "TOSTOP",  TOSTOP,    BIT, "Send the SIGTTOU signal to the process group of a background process which tries to write to its "
                                 "controlling terminal." },
    { "IEXTEN",  IEXTEN,    BIT, "Enable implementation-defined input processing.  This flag, as well as ICANON must be enabled for "
                                 "the special characters EOL2, LNEXT, REPRINT, WERASE to be interpreted, and for the IUCLC flag to be "
                                 "effective." },
    { NULL },
};

struct flags cc[] =
{
    { "VEOF",     VEOF,     CHR, "End-of-file character.  More precisely: this character causes the pending tty buffer to be sent "
                                 "to the waiting user program without waiting for end-of-line.  If it is the first character of the "
                                 "line, the read(2) in the user program returns 0, which signifies end-of-file.  Recognized when "
                                 "ICANON is set, and then not passed as input." },
    { "VEOL",     VEOL,     CHR, "Additional end-of-line character.  Recognized when ICANON is set." },
    { "VEOL2",    VEOL2,    CHR, "Yet another end-of-line character.  Recognized when ICANON is set." },
    { "VERASE",   VERASE,   CHR, "Erase character.  This erases the previous not-yet-erased character, but does not erase past EOF "
                                 "or beginning-of-line.  Recognized when ICANON is set, and then not passed as input." },
    { "VINTR",    VINTR,    CHR, "Interrupt character.  Send a SIGINT signal.  Recognized when ISIG is set, and then not passed as "
                                 "input." },
    { "VKILL",    VKILL,    CHR, "Kill character.  This erases the input since the last EOF or beginning-of-line.  Recognized when "
                                 "ICANON is set, and then not passed as input." },
    { "VLNEXT",   VLNEXT,   CHR, "Literal next.  Quotes the next input character, depriving it of a possible special meaning.  "
                                 "Recognized when IEXTEN is set, and then not passed as input." },
    { "VMIN",     VMIN,     NUM, "Minimum number of characters for noncanonical read." },
    { "VQUIT",    VQUIT,    CHR, "Quit character.  Send SIGQUIT signal.  Recognized when ISIG is set, and then not passed as input." },
    { "VREPRINT", VREPRINT, CHR, "Reprint unread characters.  Recognized when ICANON and IEXTEN are set, and then not passed as "
                                 "input." },
    { "VSTART",   VSTART,   CHR, "Start character.  Restarts output stopped by the Stop character.  Recognized when IXON is set, "
                                 "and then not passed as input." },
    { "VSTOP",    VSTOP,    CHR, "Stop character.  Stop output until Start character typed.  Recognized when IXON is set, and then "
                                 "not passed as input." },
    { "VSUSP",    VSUSP,    CHR, "Suspend character.  Send SIGTSTP signal.  Recognized when ISIG is set, and then not passed as "
                                 "input." },
    { "VTIME",    VTIME,    NUM, "Timeout in deciseconds for noncanonical read." },
    { "VWERASE",  VWERASE,  CHR, "Word erase.  Recognized when ICANON and IEXTEN are set, and then not passed as input." },
    { 0 },
};

// Given a header, and a string of arbitrary length, print the header and the string wrapped to specified width.
// Subsequent lines are indented to the length of the header with whitespace and the last character of the header.
void putwrap(const char *head, const char *text, int width)
{
    int hwidth = strlen(head);
    int twidth = width - hwidth - 1;
    if (twidth <= 0) return;
    int lines = 0;

    while(*text)
    {
        int idx = 0, len = 0;                                       // index, printable length
        while(text[idx] && idx < twidth)
        {
            if (isspace(text[idx])) len = idx;                      // print to last whitespace
            idx++;
        }
        if (!len || !text[idx] || isspace(text[idx])) len = idx;    // maybe print entire

        printf("%*s %.*s\n", hwidth, lines++ ? (head + hwidth - 1) : head, len, text);

        text += len;
        while (isspace(*text)) text++;
    }
}

void showflags(char *name, struct flags *flags, void *value, int width)
{
    int v;
    char head[32];

    printf("%s:\n", name);
    for (; flags->name; flags++)
    {
        switch(flags->type)
        {
            case BIT:
                // flag is tcflag_t single bit mask, display as "yes" or "no"
                sprintf(head, "  %-8s = %4s :", flags->name, *(tcflag_t *)value & flags->flag ? "yes" : "no");
                break;
            case VAL:
                // flag is tcflag_t multiple-bit mask, display as numeric
                sprintf(head, "  %-8s = %4d :", flags->name, *(tcflag_t *)value & flags->flag >> __builtin_ctz(flags->flag));
                break;
            case CHR:
                // flag is cc_t index to a character, display as ^X or ASCII if possible, else numeric
                v = *((cc_t *)value + flags->flag);
                if (v > 0 && v < 32)
                    sprintf(head, "  %-8s =   ^%c :", flags->name, v+64);
                else if (v >= 32 && v <= 126)
                    sprintf(head, "  %-8s =  '%c' :", flags->name, v);
                else
                    sprintf(head, "  %-8s = %4d :", flags->name, v);
                break;
            case NUM:
                // flag is cc_t index to a byte, display as numeric
                sprintf(head, "  %-8s = %4d :", flags->name, *((cc_t *)value + flags->flag));
                break;
        }
        putwrap(head, flags->description, width);
    }
}

int main(void)
{
    int width = 80;
    struct winsize ws;
    if (!ioctl(1, TIOCGWINSZ, &ws)) width = ws.ws_col-2; // use width of console on stdout, or 80 if redirected.

    struct termios t;
    if (tcgetattr(0, &t)) die("Failed to get termios attributes for stdin.\n");

    showflags("c_iflag", iflags, &t.c_iflag, width);
    showflags("\nc_oflag", oflags, &t.c_oflag, width);
    showflags("\nc_cflag", cflags, &t.c_cflag, width);
    showflags("\nc_lflag", lflags, &t.c_lflag, width);
    showflags("\nc_cc", cc, &t.c_cc, width);
}
