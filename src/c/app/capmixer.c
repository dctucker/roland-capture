#include <unistd.h>
#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include "lib/capmix.h"
#include "lib/mixer.h"

#define WIDTH 120
#define HEIGHT 20
#define SPACING 5

#define log(...) { wprintw(log_win, __VA_ARGS__); wprintw(log_win, "\n"); wrefresh(log_win); }

WINDOW *menu_win;
WINDOW *log_win;
cursor_t cursor;

int startx = 0;
int starty = 0;
int quitting = 0;

static const capmix_mixer_page_t *page;
static const char page_indicator[] = "i o [abcd] k l n p r y";
int page_indicator_len = sizeof(page_indicator);

void set_page( enum capmix_pages_e );

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

void  monitor(char mon)
{
}
void  decrement(){}
void  increment(){}
void  zero(){}
void  preamp        (){ set_page(PPreamp); }
void  compressor    (){ set_page(PCompressor); }
void  line          (){ set_page(PLine); }
void  inputs        (){ set_page(PInputA); }
void  outputs       (){ set_page(POutputA); }
void  toggle_io     (){ }
void  prev_subpage  (){ }
void  next_subpage  (){ }
void  reverb        (){ set_page(PReverb); }
void  patchbay      (){ set_page(PPatchbay); }
void  channel       (char ch)
{
}

void  quit()
{
	quitting = 1;
}

void  unrecognized(int c)
{
	log("Charcter pressed is = %3d Hopefully it can be printed as '%c'", c, c);
}

int  on_keyboard(int c)
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
		case ERR           :  return 0;
		default            :  unrecognized(c);
	}
	return 1;
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
	//sprintf(str, "%s", capmix_type_name(type)); return;
	switch(type)
	{
		case TBoolean:
			format_boolean(addr, unpacked.discrete, str);
			break;
		default:
			capmix_format_type(type, unpacked, str);
	}
}

void  request_control_data(capmix_addr_t addr, int i, int j)
{
	int delay_usec = 20e3;
	if( addr )
	{
		capmix_get(addr);
		usleep(delay_usec);
	}
	capmix_listen();
}

void  request_mixer_data()
{
	capmix_mixer_foreach(page, request_control_data);
}

void  render_control(WINDOW *menu_win, capmix_addr_t addr)
{
	char text [SPACING*2+1];
	char value[SPACING*2+1];
	int start_x = 0, start_y = 3;
	cursor_t pos = capmix_mixer_addr_xy(page, addr);

	int rowlen = capmix_mixer_rowlen(page, pos.y);
	int dx = SPACING;
	if( rowlen > 0 && rowlen == page->cols / 2 )
		dx *= 2;

	if( addr != 0 )
	{
		format_value( addr, capmix_memory_get_unpacked(addr), value ); // print formatted
		//capmix_unpacked_t v = capmix_memory_get_unpacked(addr); sprintf(value, "%x", v); // print raw value
		//sprintf(value, "%s", capmix_addr_suffix(addr)); // print suffix
	}
	else
		sprintf(value, " ");

	int len = strlen(value);
	int pad_l = (dx - len) / 2;
	int pad_r = (dx - len) / 2 + ((dx - len) % 2);
	sprintf(text, "%*s%s%*s", pad_l, "", value, pad_r, "");

	if( pos.y == cursor.y && pos.x == cursor.x )
	{
		wattron(menu_win, A_REVERSE);
		mvwprintw(menu_win, start_y + pos.y, start_x + pos.x * dx, text);
		wattroff(menu_win, A_REVERSE);
	}
	else
	{
		wattron(menu_win, COLOR_PAIR(1));
		mvwprintw(menu_win, start_y + pos.y, start_x + pos.x * dx, text);
		wattroff(menu_win, COLOR_PAIR(1));
	}
	wrefresh(menu_win);
	wmove(menu_win, pos.y + start_y, dx * pos.x + start_x);
}

void  interface_refresh(WINDOW *menu_win)
{
	char text [SPACING*2+1];
	char value[SPACING*2+1];
	int pad_l, pad_r;
	int dx = SPACING;
	int cdx;
	int start_x = 0, start_y = 0;

	//box(menu_win, 0, 0);
	for(int j=0; j < page_indicator_len; j++)
	{
		char c = page_indicator[j];
		int highlight = 0;
		switch(page->id)
		{
			case PInputA: case PInputB: case PInputC: case PInputD:
				if( c == 'i' )
					highlight = 1;
				break;
			case POutputA: case POutputB: case POutputC: case POutputD:
				if( c == 'o' )
					highlight = 1;
				break;
			case PLine       : if( c == 'l' ) highlight = 1; break;
			case PReverb     : if( c == 'r' ) highlight = 1; break;
			case PPatchbay   : if( c == 'y' ) highlight = 1; break;
			case PPreamp     : if( c == 'p' ) highlight = 1; break;
			case PCompressor : if( c == 'k' ) highlight = 1; break;
			//case PChannelN   : if( c == 'n' ) highlight = 1; break;
		}
		if( highlight )
			wattron(menu_win, COLOR_PAIR(3) | A_BOLD);
		else
			wattroff(menu_win, COLOR_PAIR(3) | A_BOLD);

		mvwprintw(menu_win, start_y, start_x + j, "%c", c);
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
		int rowlen = capmix_mixer_rowlen(page, i);
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
				cdx = dx;
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
	wmove(menu_win, cursor.y + start_y, cdx * cursor.x + start_x);
}

void  on_capmix_event(capmix_event_t event)
{
	char name[128];
	char value[16];

	capmix_format_addr(event.addr, name);
	capmix_format_type(event.type_info->type, event.unpacked, value);

	//wmove(log_win, 9, 0);
	wprintw(log_win, "cmd=%x addr=%08x data=", event.sysex->cmd, event.addr);
	wprintw(log_win, "name=%s ", name);
	wprintw(log_win, "type=%s ", event.type_info->name);
	wprintw(log_win, "unpacked=0x%x ", event.unpacked.discrete);
	wprintw(log_win, "value=%s ", value);
	wprintw(log_win, "\n");
	wrefresh(log_win);

	render_control(menu_win, event.addr);
	capmix_listen();
}

void set_page( enum capmix_pages_e p )
{
	int cursor_prev_y = cursor.y;
	cursor.y = -1;

	page = capmix_get_page(p);

	wclear(menu_win);
	interface_refresh(menu_win);

	request_mixer_data();

	cursor.y = cursor_prev_y;
}

int   main(int argc, char ** argv)
{
	capmix_connect(on_capmix_event);

	//FILE *f = fopen("/dev/tty", "r+");
	//SCREEN *screen = newterm(NULL, f, f);
	//SCREEN *previous_screen = set_term(screen);

	////this goes to stdout
	//fprintf(stdout, "\nhello\n");
	////this goes to the console
	//fprintf(stderr, "\r\nsome error\r\nanother error\r\n");
	////this goes to display
	//mvprintw(0, 0, "hello ncurses");

	initscr();
	start_color();
	clear();
	noecho();
	cbreak();
	//curs_set(0);

	init_pair(1, 15, 0);
	init_pair(2, 8, 0);
	init_pair(3, 6, 0);

	startx = 0;
	starty = 0;

	log_win = newwin(10, WIDTH, starty+HEIGHT, startx);
	scrollok(log_win, 1);

	menu_win = newwin(HEIGHT, WIDTH, starty, startx);
	keypad(menu_win, TRUE);
	nodelay(menu_win, 1);

	set_page(PInputA);

	interface_refresh(menu_win);

	int c;
	while( ! quitting )
	{
		c = wgetch(menu_win);
		if( on_keyboard(c) )
		{
			interface_refresh(menu_win);
			//log("refreshing");
		}
		capmix_listen();
	}

	clrtoeol();
	curs_set(1);
	refresh();

	//set_term(previous_screen);
	//delscreen(screen);

	endwin();
	return 0;
}
