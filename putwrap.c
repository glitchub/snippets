#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

// ./putwrap "This is a header :" "This is some arbitrary text used to illustrate how putwrap works! 123456789012"
// This is a header : This is
//                  : some
//                  : arbitrary
//                  : text used
//                  : to
//                  : illustrate
//                  : how putwrap
//                  : works!
//                  : 12345678901
//                  : 2
int main(int argc, char *argv[])
{
    putwrap(argv[1], argv[2], 30);
    return 0;
}
