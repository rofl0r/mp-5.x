/*

    Minimum Profit - Programmer Text Editor

    Copyright (C) 1991-2012 Angel Ortega <angel@triptico.com>

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

#ifdef __cplusplus
extern "C" {
#endif

extern int mp_exit_requested;
#define MP mpdm_hget_s(mpdm_root(), L"mp")

mpdm_t mp_draw(mpdm_t doc, int optimize);

mpdm_t mp_active(void);
void mp_process_action(mpdm_t action);
void mp_process_event(mpdm_t keycode);
void mp_set_y(mpdm_t doc, int y);
mpdm_t mp_build_status_line(void);
mpdm_t mp_get_history(mpdm_t key);
mpdm_t mp_menu_label(mpdm_t action);
mpdm_t mp_get_doc_names(void);
int mp_keypress_throttle(int keydown);
mpdm_t mp_load_save_state(const char *m);

#ifdef __cplusplus
}
#endif
