#include <stdio.h>
#include <stdlib.h>

// "Mutlitasking" demo for C. This is 100% legal C, using magic macros and
// leveraging the little-understood fact that switch() is not actually block
// oriented, it's much more like a computed goto. The C preprocessor must
// support the __LINE__ macro.

// task(NAME) creates a void function with specified name, and inserts preamble
// code to initialize the function state and 'switch' to it. The 'static'
// keyword can be used before task() if desired.
#define task(name) void name(void){static int __taskstate = 0;switch(__taskstate){default:{

// yield() saves the current source line number to the function's __state
// variable and returns. Then compiles a case statement for the same line
// number, so the switch(__state) in the preamble will jump there upon
// re-entry.
#define yield() do{__taskstate=__LINE__;return;case __LINE__:;}while(0)

// endtask cleans up the function semantics created by task() and also resets
// the task state to 0.
#define endtask }}__taskstate = 0;}

// Notes:

// Normally a task loops forever, yield()ing periodically. If the task
// explicitly returns, it will restart from the last yield().  If the task
// simply exits (without return statement), it will restart from the beginning.

// Stack variables can't be maintained across yield(), use statics or globals
// instead. The compiler should complain that "variable is used uninitialized"
// if you try it, but this is unreliable as of gcc 8.3.0.

// There can't be more than one yield() per source line. The compiler should
// complain about "duplicate case value" if you try it.

// Very Bad Bugs will result if you attempt to yield() from inside a switch
// statement. As far as I know there's no way to make the compiler detect it.
// So don't do that.

// A simple loop
task(one)
{
    while (1)
    {
        printf("one 1\n");
        yield();

        printf("one 2\n");
        yield();
    }
}
endtask

// Another simple loop with some setup code
task(two)
{
    printf("two setup\n");
    yield();
    while (1)
    {
        printf("two again\n");
        yield();
    }
}
endtask

// Use of a transient variable not referenced across yield().
task(three)
{
    int x;
    printf("three setup\n");
    yield();
    while(1)
    {
        printf("three");
        for (x = 0; x < 3; x++) printf(" %d", x);
        printf("\n");

        yield();
    }
}
endtask

// Use of a static variable referenced across yield(). In this case removing
// the 'static' will cause invalid behavior does not reliably trigger a compile
// error.
task(four)
{
    static int x;
    for (x = 0; x < 5; x++)
    {
        printf("four %d\n", x);
        yield();
    }
}
endtask

// Demonstrate the effect of implicit return.  This is functionally identical
// to task one.
task(five)
{
    printf("five 1\n");
    yield();

    printf("five 2\n");

    // implicit return restarts at the top
}
endtask

// Demonstrate the effect of an explicit return.  This is functionally
// identical to task two.
task(six)
{
    printf("six setup\n");
    yield();

    printf("six again\n");
    return; // explcit return restarts at the last yield()
}
endtask

void background(void)
{
    // NULL-terminated list of tasks
    const void(*tasks[])(void) = {one, two, three, four, five, six, NULL};

    // Run each
    for (int t = 0; tasks[t]; t++) tasks[t]();
}

int main(void)
{
    printf("main setup\n");      // some foreground stuff
    background();
    for (int i = 1; i < 7; i++)
    {
       printf("main %d\n", i);   // more foreground stuff
       background();
    }
    printf("Done\n");
}
