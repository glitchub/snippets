// Snarf raw bytes from stdin and write to stdout, until bytes stop coming,
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#define STDIN 0

#define die(...) cooked(), fprintf(stderr, __VA_ARGS__), exit(1)

int israw = -1;
struct termios tc;

void cooked(void)
{
    if (israw == 1)
    {
        tcsetattr(STDIN, TCSADRAIN, &tc);
        israw = 0;
    }
}

void raw(void)
{
    if (israw < 0)
    {
        // first time
        if (tcgetattr(STDIN, &tc) < 0) return;  // ignore non-tty
        atexit(cooked);                         // restore on exit
    }

    if (israw != 1)
    {
        struct termios t = tc;
        t.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
        t.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        tcsetattr(STDIN, TCSANOW, &t);
        israw = 1;
    }
}

// given timeout in seconds, read character from stdin, return -1 if error or timeout
int get(int timeout)
{
    struct timeval tv = {.tv_sec = timeout};
    fd_set fds = {0};
    FD_SET(0, &fds);

    while (select(1, &fds, NULL, NULL, &tv) > 0)
    {
        unsigned char c;
        int n = read(0, &c, 1);
        if (n == 1) return c;
        if (n == 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) break;
    }
    return -1;
}

// write character to stdout, return -1 if failed
int put(unsigned char c)
{
    while (1)
    {
        int n = write(1, &c, 1);
        if (n == 1) return 0;
        if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) return -1;
    }
}

int main(int argc, char *argv[])
{
    int c, snarfed = 0;

    raw(); // switch stdin to raw, if it's a tty

    fprintf(stderr, "Waiting 20 seconds for input on stdin...\n");

    if ((c = get(20)) < 0) die("Timeout!\n");

    do
    {
        put(c);
        snarfed++;
    } while((c = get(2)) >= 0);

    cooked();

    // try to let an output process finish up
    close(1);
    usleep(10000);

    fprintf(stderr, "Snarfed %d bytes\n", snarfed);
    return 0;
}
