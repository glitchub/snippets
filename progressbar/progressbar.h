// Create a text progress bar on stdout at the current cursor position. Example:
//
//   #include <unistd.h>
//   #include "progressbar.h"
//   int main()
//   {
//       int expected = 25; // anything greater than 1
//       ProgressBar bar(expected);
//       for (int x = 0; x < expected; x++)
//       {
//           <do something>;
//           bar.Update();
//       }
//       return 0;
//   }
//
// The constructor is given the expected number of events, Update() is then called for each event in
// order to progressively update the bar.
//
// The caller should not write to stdout until all updates have been performed. If for some reason
// the cursor must moved between Update()'s then Draw() can called to recreate the bar in its
// current state at the new cursor position. Draw() can also be called before the first Update() to
// show a preliminary "empty" bar. Draw() is always called by the first Update() if not already called.
//
// If stdout is a tty then the "fancy" progress bar is enabled, this uses spaces and backspaces to
// position the cursor when updating the bar. Examples:
//
//    [                                                  ] 0%
//    [#########################                         ] 50%
//    [##################################################] 100%
//
// Each hash represents 2% and the max length is 57 characters.
//
// If stdout is not a tty then the "plain" progress bar is enabled, which does not write the entire
// bar until it reaches 100%, therefore does not require cursor positioning. Examples:
//
//    [
//    [#########################
//    [##################################################] 100%
//
// Fancy(true) or Fancy(false) can be called before first Update()/Draw() to override the default
// choice.
//
// Label("...") defines a string to be output by Draw() before the bar, default is "". The label can
// also be specified as a second argument of the constructor.
//
// Append("...") defines a string to be output after the bar reachs 100%, default is "\n".

#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>

class ProgressBar
{
    private:
    int expected,           // number of events that will occur
        occurred,           // number of events that have occurred so far
        current;            // current percentage shown, 0-100
    bool drawn;             // true if bar has been drawn
    bool fancy;             // true if fancy bar is enabled
    std::string label;      // string to write before bar, default = ""
    std::string append;     // string to write after bar reaches 100%, default = "\n"

    public:
    ProgressBar(int _expected, std::string _label = "")
    {
        if (_expected < 2) throw std::runtime_error("ProgressBar: expected value must be >= 2");
        expected = _expected;
        label = _label;
        occurred = 0;
        current = 0;
        drawn = false;
        fancy = isatty(1);  // true if stdout is tty
        append = "\n";
    }

    void Fancy(bool _fancy) { if (!drawn) fancy = _fancy; }

    void Label(std::string _label) { label = _label; }

    void Append(std::string _append) { append = _append; }

    void Draw()
    {
        if (fancy)
        {
            std::cout << label << '[' << std::string(current/2, '#') << std::string(50 - current/2, ' ') << "] " << current << '%';
            if (current == 100) std::cout << append;
        }
        else
        {
            std::cout << label << '[' << std::string(current/2, '#');
            if (current == 100) std::cout << "] 100%" << append;
        }
        std::cout << std::flush;
        drawn = true;
    }

    void Update()
    {
        if (current == 100) return; // heh

        int percent = (++occurred * 100.0) / expected;

        if (!drawn)
        {
            current = percent;
            Draw();
            return;
        }

        if (percent > current)
        {
            if (fancy)
            {
                 std::cout << std::string(54 + (current >= 10) - current/2, '\b')
                           << std::string(percent/2 - current/2, '#')
                           << std::string(50 - percent/2, ' ')
                           << "] " << percent << '%';
                 if (percent == 100) std::cout << append;
            }
            else
            {
                 std::cout << std::string(percent/2 - current/2, '#');
                 if (percent == 100) std::cout << "] 100%" << append;
            }
            std::cout << std::flush;
            current = percent;
        }
    }
};
