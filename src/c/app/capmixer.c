#include <unistd.h>
#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <math.h>
#include "lib/capmix.h"
#include "lib/mixer.h"

#define WIDTH 120
#define HEIGHT 15
#define METER_COLOR 20

#define log(...) { \
	fprintf(log_file, __VA_ARGS__); fprintf(log_file, "\n"); fflush(log_file); \
	wprintw(log_win, __VA_ARGS__); wprintw(log_win, "\n"); wrefresh(log_win); \
}

FILE *log_file;
WINDOW *menu_win;
WINDOW *log_win;
cursor_t cursor = { .x=0, .y=0 };

int startx = 0;
int starty = 0;
int quitting = 0;

static uint8_t current_monitor = 0;
static uint8_t current_channel = 1;
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

void  decrement(){}
void  increment(){}
void  zero(){}
void  monitor(uint8_t mon)
{
	current_monitor = mon % 4;
	if( page->id >= PInputA && page->id <= PInputD )
		set_page(PInputA + current_monitor);
	else if( page->id >= POutputA && page->id <= POutputD )
		set_page(POutputA + current_monitor);
}
void  preamp        (){ set_page(PPreamp); }
void  compressor    (){ set_page(PCompressor); }
void  line          (){ set_page(PLine); }
void  inputs        (){ set_page(PInputA + current_monitor); }
void  outputs       (){ set_page(POutputA + current_monitor); }
void  reverb        (){ set_page(PReverb); }
void  patchbay      (){ set_page(PPatchbay); }
void  prev_subpage  (){ monitor(current_monitor+3); }
void  next_subpage  (){ monitor(current_monitor+1); }
void  toggle_io     ()
{
	if( page->id >= PInputA  && page->id <= PInputD  )
		outputs();
	else inputs();
}
void  channel       (char ch)
{
	if( ch == '\0' )
	{
		set_page(PChannel1 + current_channel - 1);
	}
	else
	{
		current_channel = ch % 16;
		if( page->id >= PChannel1 )
			set_page(PChannel1 + current_channel - 1);
	}
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
		case 'a'           :  monitor(0);     break;
		case 'b'           :  monitor(1);     break;
		case 'c'           :  monitor(2);     break;
		case 'd'           :  monitor(3);     break;
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




int  row_spacing(int i)
{
	int dx = COLS / (2 + page->cols);
	int rowlen = capmix_mixer_rowlen(page, i);
	if( dx > 10 ) dx = 10;
	if( rowlen > 0 && rowlen == page->cols / 2 )
		dx *= 2;
	return dx;
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
	int rowlen = capmix_mixer_rowlen(page, i);
	int delay_usec = 10e3;
	if( addr )
	{
		int rs = row_spacing(i);
		wmove(menu_win, 3+i, j * rs + rs/2);
		wrefresh(menu_win);
		capmix_get(addr);
		usleep(delay_usec);
	}
	capmix_listen();
}

void  request_mixer_data()
{
	capmix_mixer_foreach(page, request_control_data);
}

void  render_control(WINDOW *menu_win, capmix_addr_t addr, cursor_t pos)
{
	char text [64];
	char value[64];
	int start_x = 0, start_y = 3;

	int dx = row_spacing(pos.y);

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
	//wmove(menu_win, pos.y + start_y, dx * pos.x + start_x);
	//wrefresh(menu_win);
}

int  meter_color(float value)
{
	if( value == capmix_UnsetInt.continuous )
		return METER_COLOR;
	if( value >= 0. )
		return METER_COLOR+23;
	else if( value <= -70 )
		return METER_COLOR;
	for(int i=0; i < 22; i++)
	{
		if( value >= -i*3. )
			return METER_COLOR+23-i;
	}
	return METER_COLOR+1;
}

void  render_meter(WINDOW *win, capmix_addr_t addr, cursor_t pos)
{
	char text[64];
	char value[64];
	int start_x = 0, start_y = 3;
	int dx = row_spacing( page->rows - 1 );

	cursor_t prev_cursor; getyx(win, prev_cursor.y, prev_cursor.x);

	capmix_unpacked_t unpacked = capmix_memory_get_unpacked(addr);
	int color = meter_color(unpacked.continuous);

	capmix_format_type(TMeter, unpacked, value);
	int len = strlen(value);
	int pad_l = (dx - len) / 2;
	int pad_r = (dx - len) / 2 + ((dx - len) % 2);
	sprintf(text, "%*s%s%*s", pad_l, "", value, pad_r, "");

	//wattr_on(menu_win, COLOR_PAIR(20+(int)(value/12.)), NULL);
	wattr_on(menu_win, COLOR_PAIR(color), NULL);
	mvwprintw(menu_win, start_y + pos.y, start_x + pos.x * dx, "%s", text);
	wattr_off(menu_win, COLOR_PAIR(color), NULL);
	//wattr_off(menu_win, COLOR_PAIR(20+(int)(value/12.)), NULL);

	wmove(win, prev_cursor.y, prev_cursor.x);
}

void render_clipmask(WINDOW *win, int ch, cursor_t pos)
{
	cursor_t prev_cursor; getyx(win, prev_cursor.y, prev_cursor.x);

	int dx = row_spacing( page->rows - 1 );
	int start_x = dx/2-2, start_y = 2;
	capmix_unpacked_t clip = capmix_memory_get_unpacked(0xa0101 + ch/4);
	bool ch_clip = clip.discrete & (1 << (ch % 4));
	
	if( ch_clip )
	{
		wattr_on(menu_win, COLOR_PAIR(METER_COLOR+23), NULL);
		mvwprintw(menu_win, start_y + pos.y, start_x + pos.x * dx, "!");
		wattr_off(menu_win, COLOR_PAIR(METER_COLOR+23), NULL);
	}
	else
	{
		mvwprintw(menu_win, start_y + pos.y, start_x + pos.x * dx, " ");
	}

	wmove(win, prev_cursor.y, prev_cursor.x);
}

void  render_page_indicator(WINDOW *menu_win, int start_x, int start_y)
{
	for(int j=0; j < page_indicator_len; j++)
	{
		char c = page_indicator[j];
		int highlight = 0;
		switch(page->id)
		{
			case PInputA: case PInputB: case PInputC: case PInputD:
				if( c == 'i' )
					highlight = 1;
				if( c == current_monitor + 'a' )
					highlight = 1;
				break;
			case POutputA: case POutputB: case POutputC: case POutputD:
				if( c == 'o' )
					highlight = 1;
				if( c == current_monitor + 'a' )
					highlight = 1;
				break;
			case PLine       : if( c == 'l' ) highlight = 1; break;
			case PReverb     : if( c == 'r' ) highlight = 1; break;
			case PPatchbay   : if( c == 'y' ) highlight = 1; break;
			case PPreamp     : if( c == 'p' ) highlight = 1; break;
			case PCompressor : if( c == 'k' ) highlight = 1; break;
			default:
				if( page->id >= PChannel1 && page->id <= PChannel16 )
				   if( c == 'n' )
					   highlight = 1;
		}
		if( highlight )
			wattron(menu_win, COLOR_PAIR(3) | A_BOLD);
		else
			wattroff(menu_win, COLOR_PAIR(3) | A_BOLD);

		mvwprintw(menu_win, start_y, start_x + j, "%c", c);
	}
}

void  interface_refresh(WINDOW *win)
{
	char text [64];
	char value[64];
	int pad_l, pad_r;
	int main_dx = row_spacing(page->rows-1);
	int start_x = 0, start_y = 0;
	cursor_t pos;
	cursor_t prev_cursor; getyx(win, prev_cursor.y, prev_cursor.x);

	//box(menu_win, 0, 0);
	render_page_indicator(win, start_x, start_y);

	start_y += 2;
	wattron(win, A_BOLD);
	wattron(win, COLOR_PAIR(2));
	for(int j=0; j < page->cols; j++)
	{
		const char *header = page->headers[j];
		int o = (main_dx - strlen(header))/2;
		mvwprintw(win, start_y, start_x + j * main_dx + o, header);
	}
	wattroff(win, COLOR_PAIR(2));
	wattroff(win, A_BOLD);

	start_y += 1;
	for(int i=0; i < page->rows; i++)
	{
		pos.y = i;
		int dx = row_spacing(i);
		for(int j=0; j < page->cols; j++)
		{
			pos.x = j;
			capmix_addr_t addr = page->controls[i][j];
			render_control(win, addr, pos);
		}
		const char *label = page->labels[i];
		wattron(win, A_BOLD);
		wattron(win, COLOR_PAIR(2));
		mvwprintw(win, start_y + i, start_x + page->cols * main_dx + 1, label);
		wattroff(win, COLOR_PAIR(2));
		wattroff(win, A_BOLD);
	}
	for(int i=0; i < page->meter_rows; i++)
	{
		pos.y = i + page->rows;
		for(int j=0; j < page->cols; j++)
		{
			pos.x = j;
			capmix_addr_t addr = page->meters[i][j];
			render_meter(win, addr, pos);
		}
	}
	//wmove(win, cursor.y + start_y, row_spacing(cursor.y) * cursor.x + start_x);
	wmove(win, prev_cursor.y, prev_cursor.x);
	wrefresh(win);
}

void  on_capmix_event(capmix_event_t event)
{
	char name[128];
	char value[16];

	capmix_format_addr(event.addr, name);
	capmix_format_type(event.type_info->type, event.unpacked, value);

	if( event.addr >> 16 == 0x0a )
	{
		for(int i=0; i < page->meter_rows; i++)
		{
			for(int j=0; j < page->cols; j++)
			{
				capmix_addr_t addr = page->meters[i][j];
				cursor_t pos = { .x=j, .y=i + page->rows };
				render_meter(menu_win, addr, pos);
			}
		}
		switch(page->id){
			case PInputA: case PInputB: case PInputC: case PInputD:
				for(int j=0; j < page->cols; j++)
				{
					cursor_t pos = { .x=j, .y=0 };
					render_clipmask(menu_win, j, pos);
				}
				break;
			default:
		}
		//wrefresh(menu_win);
	}
	else
	{
		log("cmd=%x addr=%08x name=%s type=%s unpacked=0x%x value=%s",
			event.sysex->cmd, event.addr, name, event.type_info->name,
			event.unpacked.discrete, value
		);
		wrefresh(log_win);

		cursor_t pos = capmix_mixer_addr_xy(page, event.addr);
		render_control(menu_win, event.addr, pos);
		wrefresh(menu_win);
	}
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
	log_file = fopen("capmixer.log", "w");
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

	init_pair(1, 15, 233);
	init_pair(2,  8, 232);
	init_pair(3,  6, 0);
	init_pair(4, 25, 0);

	//int meter_colors[] = {  16,  17,  18,  19,  20,  21,  26,  31,  42,  77, 119, 190, 220, 214, 208, 196 };
	//int meter_colors[] = { 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };
	int meter_colors[]   = { 232,  16,  17,  18,  19,  20,  27,  33,  39,  81, 116, 121, 120, 119, 155, 154, 190, 191, 227, 226, 220, 214, 208, 196 };
	for(int i=0; i < 24; i++)
	{
		init_pair(METER_COLOR+i, 8, meter_colors[i]);
	}


	startx = 0;
	starty = 0;

	log_win = newwin(LINES-HEIGHT, COLS, starty+HEIGHT, startx);
	scrollok(log_win, 1);
	wattron(log_win, COLOR_PAIR(4));
	wbkgd(log_win, COLOR_PAIR(4));

	menu_win = newwin(HEIGHT, COLS, starty, startx);
	keypad(menu_win, TRUE);
	nodelay(menu_win, 1);

	set_page(PInputA);
	capmix_put(0xa0000, capmix_UnpackedInt(1));


	interface_refresh(menu_win);

	int c;
	while( ! quitting )
	{
		c = wgetch(menu_win);
		if( on_keyboard(c) )
		{
			//log("refreshing");
			interface_refresh(menu_win);
		}
		capmix_listen();
	}
	capmix_put(0xa0000, capmix_UnpackedInt(0));
	capmix_disconnect();

	clrtoeol();
	curs_set(1);
	refresh();

	//set_term(previous_screen);
	//delscreen(screen);

	endwin();
	fclose(log_file);
	return 0;
}
