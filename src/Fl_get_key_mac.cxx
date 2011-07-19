//
// "$Id$"
//
// MacOS keyboard state routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// Return the current state of a key.  Keys are named by fltk symbols,
// which are actually X keysyms.  So this has to translate to macOS
// symbols.

#include <FL/Fl.H>
#include <FL/x.H>
#include <config.h>

// convert an FLTK (X) keysym to a MacOS symbol:
// See also the inverse converter in table macKeyLookUp of Fl_cocoa.mm
// This table is in numeric order by FLTK symbol order for binary search.
// The list of Mac OS virtual keycodes appears with OS 10.5 in
// ...../Carbon.framework/Frameworks/HIToolbox.framework/Headers/Events.h

static const struct {unsigned short vk, fltk;} vktab[] = {
  { 49, ' ' }, { 39, '\'' }, { 43, ',' }, { 27, '-' }, { 47, '.' }, { 44, '/' }, 
  { 29, '0' }, { 18, '1'  }, { 19, '2'  }, { 20, '3'  }, 
  { 21, '4' }, { 23, '5'  }, { 22, '6'  }, { 26, '7'  }, 
  { 28, '8' }, { 25, '9'  }, { 41, ';'  }, { 24, '='  },
  {  0, 'A' }, { 11, 'B'  }, {  8, 'C'  }, {  2, 'D'  }, 
  { 14, 'E' }, {  3, 'F'  }, {  5, 'G'  }, {  4, 'H'  }, 
  { 34, 'I' }, { 38, 'J'  }, { 40, 'K'  }, { 37, 'L'  }, 
  { 46, 'M' }, { 45, 'N'  }, { 31, 'O'  }, { 35, 'P'  }, 
  { 12, 'Q' }, { 15, 'R'  }, {  1, 'S'  }, { 17, 'T'  }, 
  { 32, 'U' }, {  9, 'V'  }, { 13, 'W'  }, {  7, 'X'  }, 
  { 16, 'Y' }, {  6, 'Z'  }, 
  { 33, '[' }, { 30, ']' }, { 50, '`' },  { 42, '\\' },
  { 51, FL_BackSpace }, { 48, FL_Tab }, { 36, FL_Enter }, { 0x7F, FL_Pause },
  { 0x7F, FL_Scroll_Lock }, { 53, FL_Escape }, { 0x73, FL_Home }, { 123, FL_Left },
  { 126, FL_Up }, { 124, FL_Right }, { 125, FL_Down }, { 0x74, FL_Page_Up },
  { 0x79, FL_Page_Down },  { 119, FL_End }, { 0x7F, FL_Print }, { 0x7F, FL_Insert },
  { 0x6e, FL_Menu }, { 114, FL_Help }, { 0x47, FL_Num_Lock },
  { 76, FL_KP_Enter }, { 67, FL_KP+'*' }, { 69, FL_KP+'+'}, { 78, FL_KP+'-' }, { 65, FL_KP+'.' }, { 75, FL_KP+'/' }, 
  { 82, FL_KP+'0' }, { 83, FL_KP+'1' }, { 84, FL_KP+'2' }, { 85, FL_KP+'3' }, 
  { 86, FL_KP+'4' }, { 87, FL_KP+'5' }, { 88, FL_KP+'6' }, { 89, FL_KP+'7' }, 
  { 91, FL_KP+'8' }, { 92, FL_KP+'9' }, { 81, FL_KP+'=' }, 
  { 0x7a, FL_F+1 }, { 0x78, FL_F+2  }, { 0x63, FL_F+3  }, { 0x76, FL_F+4  }, 
  { 0x60, FL_F+5 }, { 0x61, FL_F+6  }, { 0x62, FL_F+7  }, { 0x64, FL_F+8  }, 
  { 0x65, FL_F+9 }, { 0x6D, FL_F+10 }, { 0x67, FL_F+11 }, { 0x6f, FL_F+12 }, 
  { 0x69, FL_F+13 }, { 0x6B, FL_F+14 }, { 0x71, FL_F+15 }, { 0x6A, FL_F+16 }, 
  { 0x38, FL_Shift_L }, { 0x3C, FL_Shift_R }, { 0x3B, FL_Control_L }, { 0x3E, FL_Control_R }, 
  { 0x39, FL_Caps_Lock }, { 0x37, FL_Meta_L }, { 0x36, FL_Meta_R },
  { 0x3A, FL_Alt_L }, { 0x3D, FL_Alt_R }, { 0x75, FL_Delete },
};

static int fltk2mac(int fltk) {
  int a = 0;
  int b = sizeof(vktab)/sizeof(*vktab);
  while (a < b) {
    int c = (a+b)/2;
    if (vktab[c].fltk == fltk) return vktab[c].vk;
    if (vktab[c].fltk < fltk) a = c+1; else b = c;
  }
  return 127;
}

//: returns true, if that key was pressed during the last event
int Fl::event_key(int k) {
  return get_key(k);
}

//: returns true, if that key is pressed right now
int Fl::get_key(int k) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
  if(CGEventSourceKeyState != NULL) {
    return (int)CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, fltk2mac(k) );
    }
  else 
#endif
  {
  typedef UInt32 fl_KeyMap[4];
  fl_KeyMap foo;
  // use the GetKeys Carbon function
  typedef void (*keymap_f)(fl_KeyMap);
  static keymap_f f = NULL;
  if (!f) f = ( keymap_f )Fl_X::get_carbon_function("GetKeys");
  (*f)(foo);
#ifdef MAC_TEST_FOR_KEYCODES
 static int cnt = 0;
 if (cnt++>1024) {
  cnt = 0;
  printf("%08x %08x %08x %08x\n", (ulong*)(foo)[3], (ulong*)(foo)[2], (ulong*)(foo)[1], (ulong*)(foo)[0]);
 }
#endif
  unsigned char *b = (unsigned char*)foo;
  // KP_Enter can be at different locations for Powerbooks vs. desktop Macs
  if (k==FL_KP_Enter) {
    return (((b[0x34>>3]>>(0x34&7))&1)||((b[0x4c>>3]>>(0x4c&7))&1));
  }
  int i = fltk2mac(k);
  return (b[i>>3]>>(i&7))&1;
  }
}

//
// End of "$Id$".
//
