#ifndef CONSOLE_H
#define CONSOLE_H

#define ASCII_ESC 27

void console_init();
void console_draw();
int console_print(const char *fmt, ...);

#endif // CONSOLE_H
