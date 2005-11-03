/*

    Minimum Profit - Programmer Text Editor

    Copyright (C) 1991-2005 Angel Ortega <angel@triptico.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    http://www.triptico.com

*/

#include "config.h"

#include <stdio.h>
#include <wchar.h>
#include <curses.h>

#include "mpdm.h"
#include "mpsl.h"

#include <stdlib.h>
#include <string.h>

/*******************
	Data
********************/

int _attrs[10];

int mpi_preread_lines = 60;

/*******************
	Code
********************/

static struct {
	int n_lines;	/* number of lines */
	int voffset;	/* offset of first visible line */
	int * offsets;	/* offsets of lines */
	char * attrs;	/* attributes */
} drw = { 0, 0, NULL, NULL };

static mpdm_t drw_prepare(mpdm_t lines, int vy)
{
	mpdm_t v;
	int n, o;

	/* @#@ */
	LINES=24;

	/* get the maximum prereadable lines */
	drw.voffset = vy > mpi_preread_lines ? mpi_preread_lines : vy;

	/* maximum lines */
	drw.n_lines = LINES + drw.voffset;

	/* create an array for joining */
	v=MPDM_A(drw.n_lines);

	/* alloc space for line offsets */
	drw.offsets = realloc(drw.offsets, drw.n_lines * sizeof(int));

	/* transfer all lines and offsets */
	for(n=o=0;n < drw.n_lines;n++)
	{
		mpdm_t t;

		t=mpdm_aget(lines, n + vy - drw.voffset);

		drw.offsets[n] = o;
		o += mpdm_size(t);

		mpdm_aset(v, t, n);
	}

	/* join all lines now */
	v=mpdm_ajoin(MPDM_LS(L"\n"), v);

	/* alloc and init space for the attributes */
	drw.attrs = realloc(drw.attrs, mpdm_size(v) + 1);
	memset(drw.attrs, 'A', mpdm_size(v) + 1);

	return(v);
}


static int drw_fill_attr(int attr)
/* fill an attribute */
{
	if(attr != -1)
		memset(drw.attrs + mpdm_regex_offset,
			attr, mpdm_regex_size);

	return(mpdm_regex_offset + mpdm_regex_size);
}


static void drw_multiline_regex(mpdm_t a, mpdm_t v, int attr)
/* sets the attribute to all matching (possibly multiline) regexes */
{
	int n;

	for(n=0;n < mpdm_size(a);n++)
	{
		mpdm_t r = mpdm_aget(a, n);
		int o = 0;

		/* while the regex matches, fill attributes */
		while(mpdm_regex(r, v, o))
			o = drw_fill_attr(attr);
	}
}


mpdm_t mpi_draw_1(mpdm_t a)
/* first stage of draw */
{
	mpdm_t txt = mpdm_aget(a, 0);
	mpdm_t lines = mpdm_hget_s(txt, L"lines");
	int x = mpdm_ival(mpdm_hget_s(txt, L"x"));
	int y = mpdm_ival(mpdm_hget_s(txt, L"y"));
	int vx = mpdm_ival(mpdm_hget_s(txt, L"vx"));
	int vy = mpdm_ival(mpdm_hget_s(txt, L"vy"));
	mpdm_t v, r, t;
	int n, o;
	mpdm_t hl_words = NULL;
	mpdm_t comments = NULL;
	mpdm_t quotes = NULL;
	mpdm_t tags = NULL;

	v=drw_prepare(lines, vy);

	r=MPDM_LS(L"/\\w+/");

	/* @#@ add the word 'config' to the highlighted words */
	hl_words = MPDM_H(0);
	mpdm_hset(hl_words, MPDM_LS(L"config"), MPDM_I(8));

	/* loop all words, starting from the first visible line */
	for(o=drw.offsets[vy + drw.voffset];(t = mpdm_regex(r, v, o));)
	{
		mpdm_t c;
		int attr = -1;

		mpdm_dump(t);

		/* is the word among the tokens or variables? */
		if((c = mpdm_hget(hl_words, t)) != NULL)
			attr = mpdm_ival(c);
		else
		/* is the word among current tags? */
		if((mpdm_hget(tags, t)) != NULL)
			attr = 16;	/* tag attribute */
		else
		/* is the word correctly spelled? */
		if(0)
			attr = 32;	/* mispelling attribute */

		o=drw_fill_attr(attr);
	}

	/* fill attributes for quotes (strings) */
	drw_multiline_regex(quotes, v, 64);

	/* fill attributes for comments */
	drw_multiline_regex(comments, v, 80);

	/* now set the marked block (if any) */
	/* ... */

	/* and finally the cursor */
	drw.attrs[drw.offsets[y - vy + drw.voffset] + x] = 128;

	return(NULL);
}


mpdm_t nc_startup(mpdm_t v)
{
	initscr();
	start_color();
	keypad(stdscr, TRUE);
	nonl();
	raw();
	noecho();

	mpdm_hset_s(mpdm_root(), L"COLS", MPDM_I(COLS));
	mpdm_hset_s(mpdm_root(), L"LINES", MPDM_I(LINES));

	init_pair(1, COLOR_BLACK, COLOR_WHITE);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);

	_attrs[0] = COLOR_PAIR(1);
	_attrs[1] = COLOR_PAIR(2) | A_REVERSE;

	bkgdset(' ' | _attrs[0]);

	return(NULL);
}


mpdm_t nc_shutdown(mpdm_t v)
{
	endwin();
	return(NULL);
}


mpdm_t nc_getkey(mpdm_t v)
{
	wchar_t c[2];
	wchar_t * f = c;

	get_wch(c);
	c[1] = L'\0';

	switch(c[0]) {
	case L'\e': f = L"escape"; break;
	case KEY_LEFT: f = L"cursor-left"; break;
	case KEY_RIGHT: f = L"cursor-right"; break;
	case KEY_UP: f = L"cursor-up"; break;
	case KEY_DOWN: f = L"cursor-down"; break;
	case KEY_HOME: f = L"home"; break;
	case KEY_END: f = L"end"; break;
	}

	return(MPDM_LS(f));
}


#define MP_REAL_TAB_SIZE(x) (8 - ((x) % 8))

static int mp_wcwidth(int x, wchar_t c)
{
	int r;

	if(c == L'\t')
		r=MP_REAL_TAB_SIZE(x);
	else
		r=mpdm_wcwidth(c);

	return(r);
}


mpdm_t nc_draw(mpdm_t a)
{
	int n;
	wchar_t buf[1024];
	mpdm_t txt = mpdm_aget(a, 0);
	mpdm_t lines = mpdm_hget_s(txt, L"lines");
	int x = mpdm_ival(mpdm_hget_s(txt, L"x"));
	int y = mpdm_ival(mpdm_hget_s(txt, L"y"));
	int vx = mpdm_ival(mpdm_hget_s(txt, L"vx"));
	int vy = mpdm_ival(mpdm_hget_s(txt, L"vy"));
	wchar_t * ptr;

	move(0, 0);
	erase();

	for(n = 0;n < LINES;n++)
	{
		int m, w, o, k;
		mpdm_t l;

		/* gets the line */
		if((l = mpdm_aget(lines, vy + n)) == NULL)
			break;

		ptr=(wchar_t *) l->data;

		for(m = w = o = k = 0;m < vx + COLS;)
		{
			wchar_t c = ptr[k];
			int i = mp_wcwidth(m, c);

			if(c == L'\0') break;

			if(m >= vx)
			{
				if(c == L'\t')
				{
					int t;

					for(t = 0;t < i;t++)
						buf[o++] = L'#';
				}
				else
				{
					if(n + vy == y && k == x)
						buf[o++] = L'_';
					else
						buf[o++] = c;
				}

				w += i;
			}
			else
			{
				/* will this char cross the left margin? */
				if(m + i > vx)
				{
					int t;

					for(t = m + i - vx;t;t--, w++)
						buf[o++] = L'~';
				}
			}

			m += i;
			k++;
		}

		/* fill with spaces to the end of the line */
/*		while(w < COLS)
		{
			buf[o++] = L'-';
			w++;
		}
*/
		/* null terminate */
		buf[o] = L'\0';

		/* and draw */
		move(n, 0);
		addwstr(buf);
	}

	refresh();

	return(NULL);
}


void mp_4_startup(int argc, char * argv[])
{
	mpdm_startup();

	mpsl_argv(argc, argv);

	mpdm_hset_s(mpdm_root(), L"nc_startup", MPDM_X(nc_startup));
	mpdm_hset_s(mpdm_root(), L"nc_shutdown", MPDM_X(nc_shutdown));
	mpdm_hset_s(mpdm_root(), L"nc_getkey", MPDM_X(nc_getkey));
	mpdm_hset_s(mpdm_root(), L"nc_draw", MPDM_X(nc_draw));

	mpdm_hset_s(mpdm_root(), L"mpi_draw_1", MPDM_X(mpi_draw_1));
}


void mp_4_mpsl(void)
{
	mpdm_t v;

	v=mpsl_compile_file(MPDM_LS(L"mp-4.mpsl"));
	mpdm_exec(v, NULL);
}


void mp_4_shutdown(void)
{
	mpdm_shutdown();
}


int main(int argc, char * argv[])
{
	mp_4_startup(argc, argv);

	mp_4_mpsl();

	mp_4_shutdown();

	return(0);
}
