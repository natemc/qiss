#include <terminal_width.h>
#include <sys/ioctl.h>
#include <sys/ttycom.h>

int terminal_width() {
    winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    // When running in the debugger, ws_col is 0, and that causes headaches.
    // When stdin is connected to a pipe, ws_col is big.
    return 0 < w.ws_col && w.ws_col < 256? w.ws_col : 96;
}
