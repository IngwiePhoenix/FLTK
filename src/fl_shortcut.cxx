//
// "$Id$"
//
// Shortcut support routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2009 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// Code to test and parse fltk shortcut numbers.
//
// A shortcut is a keysym or'd with shift flags.  In the simplest
// sense a shortcut is matched if the shift state is exactly as
// given and the key returning that keysym is pressed.
//
// To make it easier to match some things it is more complex:
//
// Only FL_META, FL_ALT, FL_SHIFT, and FL_CTRL must be "off".  A
// zero in the other shift flags indicates "dont care".
//
// It also checks against the first character of Fl::event_text(),
// and zero for FL_SHIFT means "don't care".
// This allows punctuation shortcuts like "#" to work (rather than
// calling it "shift+3" on a US keyboard)

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <ctype.h>
#include "flstring.h"
#if !defined(WIN32) && !defined(__APPLE__)
#include <FL/x.H>
#endif

/**
    Test the current event, which must be an FL_KEYBOARD or 
    FL_SHORTCUT, against a shortcut value (described in 
    Fl_Button).  Returns non-zero if there is a match.  Not to
    be confused with 
    Fl_Widget::test_shortcut().
*/
int Fl::test_shortcut(unsigned int shortcut) {
  if (!shortcut) return 0;

  unsigned int v = shortcut & FL_KEY_MASK;
  if (fl_tolower(v)!=v) {
    shortcut |= FL_SHIFT;
  }

  int shift = Fl::event_state();
  // see if any required shift flags are off:
  if ((shortcut&shift) != (shortcut&0x7fff0000)) return 0;
  // record shift flags that are wrong:
  int mismatch = (shortcut^shift)&0x7fff0000;
  // these three must always be correct:
  if (mismatch&(FL_META|FL_ALT|FL_CTRL)) return 0;

  unsigned int key = shortcut & FL_KEY_MASK;

  // if shift is also correct, check for exactly equal keysyms:
  if (!(mismatch&(FL_SHIFT)) && key == Fl::event_key()) return 1;

  // try matching utf8, ignore shift:
  unsigned int firstChar = fl_utf8decode(Fl::event_text(), Fl::event_text()+Fl::event_length(), 0);
  if (key==firstChar) return 1;

  // kludge so that Ctrl+'_' works (as opposed to Ctrl+'^_'):
  if ((shift&FL_CTRL) && key >= 0x3f && key <= 0x5F
      && firstChar==(key^0x40)) return 1; // firstChar should be within a-z
  return 0;
}

// This table must be in numeric order by fltk (X) keysym number:
struct Keyname {int key; const char* name;};
#if defined(WIN32)
static Keyname table[] = {
  {' ', "Space"},
  {FL_BackSpace, "Backspace"},
  {FL_Tab,	"Tab"},
  {0xff0b/*XK_Clear*/, "Clear"},
  {FL_Enter,	"Enter"}, // X says "Enter"
  {FL_Pause,	"Pause"},
  {FL_Scroll_Lock, "Scroll_Lock"},
  {FL_Escape,	"Escape"},
  {FL_Home,	"Home"},
  {FL_Left,	"Left"},
  {FL_Up,	"Up"},
  {FL_Right,	"Right"},
  {FL_Down,	"Down"},
  {FL_Page_Up,	"Page_Up"}, // X says "Prior"
  {FL_Page_Down,"Page_Down"}, // X says "Next"
  {FL_End,	"End"},
  {FL_Print,	"Print"},
  {FL_Insert,	"Insert"},
  {FL_Menu,	"Menu"},
  {FL_Num_Lock,	"Num_Lock"},
  {FL_KP_Enter,	"KP_Enter"},
  {FL_Shift_L,	"Shift_L"},
  {FL_Shift_R,	"Shift_R"},
  {FL_Control_L,"Control_L"},
  {FL_Control_R,"Control_R"},
  {FL_Caps_Lock,"Caps_Lock"},
  {FL_Meta_L,	"Meta_L"},
  {FL_Meta_R,	"Meta_R"},
  {FL_Alt_L,	"Alt_L"},
  {FL_Alt_R,	"Alt_R"},
  {FL_Delete,	"Delete"}
};
#elif defined(__APPLE__) 
static Keyname table[] = {
                                 // v - this column contains utf8 characters
  {' ', "Space"},
  {FL_BackSpace,"\xe2\x8c\xab"}, // ⌫  erase to the left
  {FL_Tab,	"\xe2\x87\xa5"}, // ⇥  rightwards arrow to bar
  {0xff0b,      "\xe2\x8c\xa6"}, // ⌦  erase to the right
  {FL_Enter,	"\xe2\x86\xa9"}, // ↩  leftwards arrow with hook
  {FL_Pause,	"Pause"},
  {FL_Scroll_Lock, "Scroll_Lock"},
  {FL_Escape,	"\xe2\x90\x9b"},
  {FL_Home,	"\xe2\x86\x96"}, // ↖  north west arrow
  {FL_Left,	"\xe2\x86\x90"}, // ←  leftwards arrow
  {FL_Up,	"\xe2\x86\x91"}, // ↑  upwards arrow
  {FL_Right,	"\xe2\x86\x92"}, // →  rightwards arrow
  {FL_Down,	"\xe2\x86\x93"}, // ↓  downwards arrow
  {FL_Page_Up,	"\xe2\x87\x9e"}, // ⇞  upwards arrow with double stroke
  {FL_Page_Down,"\xe2\x87\x9f"}, // ⇟  downward arrow with double stroke
  {FL_End,	"\xe2\x86\x98"}, // ↘  south east arrow
  {FL_Print,	"Print"},
  {FL_Insert,	"Insert"},
  {FL_Menu,	"Menu"},
  {FL_Num_Lock,	"Num_Lock"},
  {FL_KP_Enter,	"\xe2\x8c\xa4"}, // ⌤ up arrow head between two horizontal bars
  {FL_Shift_L,	"Shift_L"},
  {FL_Shift_R,	"Shift_R"},
  {FL_Control_L,"Control_L"},
  {FL_Control_R,"Control_R"},
  {FL_Caps_Lock,"\xe2\x87\xaa"}, // ⇪  upwards white arrow from bar
  {FL_Meta_L,	"Meta_L"},
  {FL_Meta_R,	"Meta_R"},
  {FL_Alt_L,	"Alt_L"},
  {FL_Alt_R,	"Alt_R"},
  {FL_Delete,	"\xe2\x8c\xa7"}  // ⌧  x in a rectangle box
};
#endif

/**
  Get a human-readable string from a shortcut value.

  Unparse a shortcut value as used by Fl_Button or Fl_Menu_Item into
  a human-readable string like "Alt+N". This only works if the shortcut
  is a character key or a numbered function key. If the shortcut is
  zero then an empty string is returned. The return value points at
  a static buffer that is overwritten with each call.

  \param [in] shortcut the integer value containing the ascii charcter or extended keystroke plus modifiers
  \return a pointer to a static buffer containing human readable text for the shortcut
  */
const char* fl_shortcut_label(unsigned int shortcut) {
  return fl_shortcut_label(shortcut, 0L);
}

/** 
  Get a human-readable string from a shortcut value.

  \param [in] shortcut the integer value containing the ascii charcter or extended keystroke plus modifiers
  \param [in] eom if this pointer is set, it will receive a pointer to the end of the modifier text
  \return a pointer to a static buffer containing human readable text for the shortcut
  \see fl_shortcut_label(unsigned int shortcut)
  */
const char* fl_shortcut_label(unsigned int shortcut, const char **eom) {
  static char buf[20];
  char *p = buf;
  if (eom) *eom = p;
  if (!shortcut) {*p = 0; return buf;}
  // fix upper case shortcuts
  unsigned int v = shortcut & FL_KEY_MASK;
  if (fl_tolower(v)!=v) {
    shortcut |= FL_SHIFT;
  }
#ifdef __APPLE__
  //                        this column contains utf8 characters - v
  if (shortcut & FL_SHIFT) {strcpy(p,"\xe2\x87\xa7"); p += 3;}  // ⇧  upwards white arrow
  if (shortcut & FL_CTRL)  {strcpy(p,"\xe2\x8c\x83"); p += 3;}  // ⌃  up arrowhead
  if (shortcut & FL_ALT)   {strcpy(p,"\xe2\x8c\xa5"); p += 3;}  // ⌥  alternative key symbol
  if (shortcut & FL_META)  {strcpy(p,"\xe2\x8c\x98"); p += 3;}  // ⌘  place of interest sign
#else
  if (shortcut & FL_META) {strcpy(p,"Meta+"); p += 5;}
  if (shortcut & FL_ALT) {strcpy(p,"Alt+"); p += 4;}
  if (shortcut & FL_SHIFT) {strcpy(p,"Shift+"); p += 6;}
  if (shortcut & FL_CTRL) {strcpy(p,"Ctrl+"); p += 5;}
#endif // __APPLE__
  if (eom) *eom = p;
  unsigned int key = shortcut & FL_KEY_MASK;
#if defined(WIN32) || defined(__APPLE__) // if not X
  if (key >= FL_F && key <= FL_F_Last) {
    *p++ = 'F';
    if (key > FL_F+9) *p++ = (key-FL_F)/10+'0';
    *p++ = (key-FL_F)%10 + '0';
  } else {
    // binary search the table for a match:
    int a = 0;
    int b = sizeof(table)/sizeof(*table);
    while (a < b) {
      int c = (a+b)/2;
      if (table[c].key == key) {
        if (p > buf) {
          strcpy(p,table[c].name); 
          return buf;
        } else {
          const char *sp = table[c].name;
          if (eom) *eom = sp;
          return sp;
        }
      }
      if (table[c].key < key) a = c+1;
      else b = c;
    }
    if (key >= FL_KP && key <= FL_KP_Last) {
      // mark keypad keys with KP_ prefix
      strcpy(p,"KP_"); p += 3;
      *p++ = uchar(key & 127);
    } else {
      // if none found, use the keystroke as a match:
      p += fl_utf8encode(fl_toupper(key), p); 
    }
  }
  *p = 0;
  return buf;
#else
  const char* q;
  if (key == FL_Enter || key == '\r') q="Enter";  // don't use Xlib's "Return":
  else if (key > 32 && key < 0x100) q = 0;
  else q = XKeysymToString(key);
  if (!q) {
    p += fl_utf8encode(fl_toupper(key), p); 
    *p = 0; 
    return buf;
  }
  if (p > buf) {
    strcpy(p,q); 
    return buf;
  } else {
    if (eom) *eom = q;
    return q;
  }
#endif
}

// Emulation of XForms named shortcuts
#include <stdlib.h>
/**
  Emulation of XForms named shortcuts.
*/
unsigned int fl_old_shortcut(const char* s) {
  if (!s || !*s) return 0;
  unsigned int n = 0;
  if (*s == '#') {n |= FL_ALT; s++;}
  if (*s == '+') {n |= FL_SHIFT; s++;}
  if (*s == '^') {n |= FL_CTRL; s++;}
  if (*s && s[1]) return n | (int)strtol(s,0,0); // allow 0xf00 to get any key
  return n | *s;
}

// Tests for &x shortcuts in button labels:

unsigned int Fl_Widget::label_shortcut(const char *t) {
  if (!t) return 0;
  for (;;) {
    if (*t==0) return 0;
    if (*t=='&') {
      unsigned int s = fl_utf8decode(t+1, 0, 0);
      if (s==0) return 0;
      else if (s==(unsigned int)'&') t++;
      else return s;
    }
    t++;
  }
}

int Fl_Widget::test_shortcut(const char *t) {
  #ifdef WIN32
  // on MSWindows, users expect shortcuts to work only when the Alt modifier is pressed
  if (Fl::event_state(FL_ALT)==0) return 0;
  #endif
  if (!t) return 0;
  unsigned int c = fl_utf8decode(Fl::event_text(), Fl::event_text()+Fl::event_length(), 0);
  if (!c) return 0;
  if (c == label_shortcut(t))
    return 1;
  return 0;
}

int Fl_Widget::test_shortcut() {
  if (!(flags()&SHORTCUT_LABEL)) return 0;
  return test_shortcut(label());
}

//
// End of "$Id$".
//
