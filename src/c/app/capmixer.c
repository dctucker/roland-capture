#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include "lib/capmix.h"
#include "lib/mixer.h"

#define WIDTH 90
#define HEIGHT 20
#define SPACING 5

struct cursor {
	int x;
	int y;
} cursor;

int startx = 0;
int starty = 0;
int quitting = 0;

const capmix_mixer_page_t *page;
const char *page_indicator = "i o [abcd] k l n p r y";
int page_indicator_len = 22;

void  up()
{
	if( cursor.y > 0 )
		cursor.y -= 1;
}

void  down()
{
	if( cursor.y < page->rows - 1)
		cursor.y += 1;
}

void  left()
{
	if( cursor.x > 0 )
		cursor.x -= 1;
}

void  right()
{
	if( cursor.x < page->cols - 1 )
		cursor.x += 1;
}

void  monitor(char mon){}
void  decrement(){}
void  increment(){}
void  zero(){}
void  preamp(){}
void  compressor(){}
void  line(){}
void  inputs(){}
void  outputs(){}
void  toggle_io(){}
void  prev_subpage(){}
void  next_subpage(){}
void  reverb(){}
void  patchbay(){}
void  channel(char ch){}

void  quit()
{
	quitting = 1;
}

void  unrecognized(int c)
{
	mvprintw(24, 0, "Charcter pressed is = %3d Hopefully it can be printed as '%c'", c, c);
	refresh();
}

void  on_keyboard(int c)
{
	switch(c)
	{
		case KEY_UP        :  up();             break;
		case KEY_DOWN      :  down();           break;
		case KEY_LEFT      :  left();           break;
		case KEY_RIGHT     :  right();          break;
		case 'a'           :  monitor('a');     break;
		case 'b'           :  monitor('b');     break;
		case 'c'           :  monitor('c');     break;
		case 'd'           :  monitor('d');     break;
		case '-': case '_' :  decrement();      break;
		case '=': case '+' :  increment();      break;
		case 'z'           :  zero();           break;
		case 'p'           :
		case KEY_BTAB      :  preamp();         break;
		case 's': case 'k' :  compressor();     break;
		case 'l'           :  line();           break;
		case 'i'           :  inputs();         break;
		case 'o'           :  outputs();        break;
		case '\t'          :  toggle_io();      break;
		case '['           :  prev_subpage();   break;
		case ']'           :  next_subpage();   break;
		case 'r': case 'v' :  reverb();         break;
		case 'y': case 'P' :  patchbay();       break;
		case 'q'           :  quit();           break;
		case 'n'           :  channel(0);       break;
		case '1'           :  channel(1);       break;
		case '2'           :  channel(2);       break;
		case '3'           :  channel(3);       break;
		case '4'           :  channel(4);       break;
		case '5'           :  channel(5);       break;
		case '6'           :  channel(6);       break;
		case '7'           :  channel(7);       break;
		case '8'           :  channel(8);       break;
		case '9'           :  channel(9);       break;
		case '0'           :  channel(10);      break;
		case '!'           :  channel(11);      break;
		case '@'           :  channel(12);      break;
		case '#'           :  channel(13);      break;
		case '$'           :  channel(14);      break;
		case '%'           :  channel(15);      break;
		case '^'           :  channel(16);      break;
		default            :  unrecognized(c);
	}
}

#define ON_SUFFIX(X,Y,Z) if( strcmp(suffix, X) == 0 ){ if( yes ) sprintf(str, Y); else sprintf(str, Z); return; }
void  format_boolean( capmix_addr_t addr, int yes, char *str )
{
	if( yes == capmix_Unset )
		yes = 0;
	const char *suffix = capmix_addr_suffix(addr);
	ON_SUFFIX( "mute"   , "MUTE"   , "m"        )
	ON_SUFFIX( "solo"   , "SOLO"   , "s"        )
	ON_SUFFIX( "stereo" , "STEREO" , "--mono--" )
	ON_SUFFIX( "+48"    , "+48V"   , "0VDC"     )
	ON_SUFFIX( "lo-cut" , "LO CUT" , "low"      )
	ON_SUFFIX( "phase"  , "-PHASE" , "+"        )
	ON_SUFFIX( "bypass" , "BYPASS" , "comp"     )
	ON_SUFFIX( "hi-z"   , "HI-Z"   , "lo-z"     )
}

void  format_value( capmix_addr_t addr, capmix_unpacked_t unpacked, char *str )
{
	capmix_type_t type = capmix_addr_type(addr);
	sprintf(str, "%s", capmix_type_name(type));
	return;
	switch(type)
	{
		case TBoolean:
			format_boolean(addr, unpacked.discrete, str);
			break;
		default:
			capmix_format_type(addr, unpacked, str);
	}
}

void  interface_refresh(WINDOW *menu_win)
{
	char text [SPACING*2+1];
	char value[SPACING*2+1];
	int pad_l, pad_r;
	int dx = SPACING;
	int start_x = 0, start_y = 0;

	//box(menu_win, 0, 0);
	for(int j=0; j < page_indicator_len; j++)
	{
		mvwprintw(menu_win, start_y, start_x + j, "%c", page_indicator[j]);
	}

	start_y += 2;
	wattron(menu_win, A_BOLD);
	wattron(menu_win, COLOR_PAIR(2));
	for(int j=0; j < page->cols; j++)
	{
		const char *header = page->headers[j];
		int o = (SPACING - strlen(header))/2;
		mvwprintw(menu_win, start_y, start_x + j * SPACING + o, header);
	}
	wattroff(menu_win, COLOR_PAIR(2));
	wattroff(menu_win, A_BOLD);

	start_y += 1;
	for(int i=0; i < page->rows; i++)
	{
		int rowlen = 0;
		for(int j=0; j < page->cols; j++)
		{
			if( page->controls[i][j] != 0 )
				rowlen++;
		}
		dx = SPACING;
		if( rowlen > 0 && rowlen == page->cols / 2 )
			dx *= 2;
		for(int j=0; j < page->cols; j++)
		{
			capmix_addr_t addr = page->controls[i][j];
			if( addr != 0 )
			{
				format_value( addr, capmix_memory_get_unpacked(addr), value ); // print formatted
				//capmix_unpacked_t v = capmix_memory_get_unpacked(addr); sprintf(value, "%x", v); // print raw value
				//sprintf(value, "%s", capmix_addr_suffix(addr)); // print suffix
			}
			else
				sprintf(value, " ");

			int len = strlen(value);
			pad_l = (dx - len) / 2;
			pad_r = (dx - len) / 2 + ((dx - len) % 2);
			sprintf(text, "%*s%s%*s", pad_l, "", value, pad_r, "");

			if( i == cursor.y && j == cursor.x )
			{
				wattron(menu_win, A_REVERSE);
				mvwprintw(menu_win, start_y + i, start_x + j * dx, text);
				wattroff(menu_win, A_REVERSE);
			}
			else
			{
				wattron(menu_win, COLOR_PAIR(1));
				mvwprintw(menu_win, start_y + i, start_x + j * dx, text);
				wattroff(menu_win, COLOR_PAIR(1));
			}
		}
		const char *label = page->labels[i];
		wattron(menu_win, A_BOLD);
		wattron(menu_win, COLOR_PAIR(2));
		mvwprintw(menu_win, start_y + i, start_x + page->cols * SPACING + 1, label);
		wattroff(menu_win, COLOR_PAIR(2));
		wattroff(menu_win, A_BOLD);
	}
	wrefresh(menu_win);
}

void  on_capmix_event(capmix_event_t event)
{
}

int   main(int argc, char ** argv)
{
	capmix_connect(on_capmix_event);
	page = capmix_get_page(PInputB);

	//FILE *f = fopen("/dev/tty", "r+");
	//SCREEN *screen = newterm(NULL, f, f);
	//SCREEN *previous_screen = set_term(screen);

	////this goes to stdout
	//fprintf(stdout, "\nhello\n");
	////this goes to the console
	//fprintf(stderr, "\r\nsome error\r\nanother error\r\n");
	////this goes to display
	//mvprintw(0, 0, "hello ncurses");

	WINDOW *menu_win;

	initscr();
	start_color();
	clear();
	noecho();
	cbreak();
	//curs_set(0);

	init_pair(1, 15, 0);
	init_pair(2, 8, 0);

	startx = 0;
	starty = 0;

	menu_win = newwin(HEIGHT, WIDTH, starty, startx);
	keypad(menu_win, TRUE);
	refresh();

	int c;
	while( ! quitting )
	{
		interface_refresh(menu_win);
		c = wgetch(menu_win);
		on_keyboard(c);
	}

	clrtoeol();
	curs_set(1);
	refresh();

	//set_term(previous_screen);
	//delscreen(screen);

	endwin();
	return 0;
}
