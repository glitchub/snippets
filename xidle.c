// Report X display idle seconds, build with:
//
//   CFLAGS="-lX11 -lXss" make xidle

#include <X11/extensions/scrnsaver.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1);

int main(int argc, char *argv[])
{
    // Use the display name specified on command line, or $DISPLAY, or default to ":0"
    char *display_name = argv[1] ?: getenv("DISPLAY") ?: ":0";
    Display *display = XOpenDisplay(display_name);
    if (!display) die("Can't open display \"%s\"\n", display_name);

    XScreenSaverInfo *info = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
    if (!info) die("Can't get screensaver info for display \"%s\"\n", display_name);

    printf("Display \"%s\" idle %.3f seconds\n", display_name, info->idle/1000.0);
    return 0;
}
