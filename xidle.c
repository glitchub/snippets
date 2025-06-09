// Report X display idle time. Build with:
//    LDLIBS="-lX11 -lXss" make xidle
// If scrnsaver.h not found you need libXss headers. On debian, try:
//    sudo apt install libxss-dev

#include <X11/extensions/scrnsaver.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1);

#define msday  (86400*1000) // milliseconds per day
#define mshour (3600*1000)  // milliseconds per hour
#define msmin  (60*1000)    // milliseconds per minute

int main(int argc, char **argv)
{
    // verbose time if -v
    int verbose = 0;
    if (*++argv && !strcmp(*argv, "-v")) { verbose=1; argv++; }

    // Use the display name specified on command line, or $DISPLAY, or default to ":0"
    char *display_name = *argv ?: getenv("DISPLAY") ?: ":0";
    Display *display = XOpenDisplay(display_name);
    if (!display) die("Can't open display \"%s\"\n", display_name);

    XScreenSaverInfo *info = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
    if (!info) die("Can't get screensaver info for display \"%s\"\n", display_name);

    unsigned long idle = info->idle; // 32-bit milliseconds wraps every 49 days
    if (verbose)
        printf("Display \"%s\" idle %ld days, %ld hours, %ld minutes, %.3f seconds\n", display_name, idle / msday, (idle % msday) / mshour, (idle % mshour) / msmin, (idle % msmin) / 1000.0);
    else
        printf("%ld\n", idle);
    return 0;
}
