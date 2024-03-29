// Test program for progressbar.h
// Build with:
//     CXXFLAGS="-Wall -Werror" make progressbar
// Run with and without "| cat" to observe the fancy vs plain bar style.

#include "progressbar.h"

int main(int argc, char *argv[0])
{
    int expected = (argc > 1) ? atoi(argv[1]) : 10; // anything greater than 1

    ProgressBar bar(expected, "Test: ");

    for (int x = 0; x < expected; x++)
    {
        if (x) usleep(5000000/expected); // total time about 5 seconds
        bar.Update();
    }

    return 0;
}
