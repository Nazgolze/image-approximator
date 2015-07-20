#include "common.h"
#include "console.h"

static void _print_VT100(const char *code);

static void _print_generation()
{
	if(!ia_cfg.cur_gen) {
		printf("Generation: Initializing....\n");
		return;
	}
	printf("Generation: %lu. Score: %ld\n",
		ia_cfg.cur_gen,
		ia_cfg.cur_gen_score);
}

static void _print_user_input()
{
	printf(">> ");
}

/**
 * Initialize the console
 */
void console_init()
{
	printf("======= Image Approximator =======\n");
	printf("\n\n\n");
}

/**
 * Print to the console a message
 */
int console_print(const char *fmt, ...)
{
	int error = SUCCESS;
	char str[2048] = {0};
	va_list ap;
	va_start(ap, fmt);
	error = vsnprintf(str, sizeof(str), fmt, ap);
	if(error == ERROR)
		return error;
	va_end(ap);
	printfi("%s", str);
	console_draw(str);
	fflush(stdout);
	return error;
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


/**
 * Draw the console
 */
void console_draw(const char *msg)
{
	// If a message is being displayed then the user has not
	// hit enter yet so we need to go up fewer lines
	if(!msg) {
		_cursor_up_clear(3);
	} else {
		_cursor_up_clear(2);
	}
	_print_generation();
	if(msg) {
		printf("Message: %s\n", msg);
	} else {
		printf("Message: N/A\n");
	}
	_print_user_input();
	fflush(stdout);
}
