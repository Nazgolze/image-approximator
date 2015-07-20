#include "common.h"
#include "console.h"

static void _print_generation()
{
	printf("Generation: %lu\n", ia_cfg.cur_gen);
}

static void _print_user_input()
{
	printf(">> ");
}

void console_init()
{
	printf("======= Image Approximator =======\n");
	printf("\n\n\n");
}

void console_print(const char *msg)
{
	console_draw(msg);
}

/**
 * Print a VT100 sequence
 *
 * @param code The code sequence after ^[
 */
static void _print_VT100(const char *code)
{
	printf("%c%s", ASCII_ESC, code);
}

/**
 * Clear the current line
 */
static void _cursor_clear_line()
{
	_print_VT100("[2K");
}

/**
 * Move up one line
 */
static void _cursor_up(int num)
{
	int ix;
	for(ix = 0; ix < num; ix++) {
		_print_VT100("[F");
	}
}

/**
 * Move down one line
 */
static void _cursor_down(int num)
{
	int ix;
	for(ix = 0; ix < num; ix++) {
		_print_VT100("[B");
	}
}

/**
 * Move up one line and clear it
 */
static void _cursor_up_clear(int num)
{
	int ix;
	for(ix = 0; ix < num; ix++) {
		_cursor_up(1);
		_cursor_clear_line();
	}
}


void console_draw(const char *msg)
{
	if(msg) {
		_cursor_up_clear(2);
	} else {
		_cursor_up_clear(3);
	}
	_print_generation();
	if(msg) {
		printf("Message: %s\n", msg);
	} else {
		printf("Message: N/A\n");
	}
	_print_user_input();
}
