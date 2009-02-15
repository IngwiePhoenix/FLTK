//
// "$Id$"
//
// Common menu code for the Fast Light Tool Kit (FLTK).
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
// This is a base class for all items that have a menu:
//	Fl_Menu_Bar, Fl_Menu_Button, Fl_Choice
// This provides storage for a menu item, functions to add/modify/delete
// items, and a call for when the user picks a menu item.

// More code in Fl_Menu_add.cxx

#include <FL/Fl.H>
#include <FL/Fl_Menu_.H>
#include "flstring.h"
#include <stdio.h>
#include <stdlib.h>

#define SAFE_STRCAT(s) { len += strlen(s); if ( len >= namelen ) { *name='\0'; return(-2); } else strcat(name,(s)); }

/** Set 'pathname' of specified menuitem
    If finditem==NULL, mvalue() is used (the most recently picked menuitem)
    Returns:
    -   0 : OK
    -  -1 : item not found (name="")
    -  -2 : 'name' not large enough (name="")
*/
int Fl_Menu_::item_pathname(char *name, int namelen, const Fl_Menu_Item *finditem) const {
    int len = 0;
    finditem = finditem ? finditem : mvalue();    
    name[0] = '\0';
    for ( int t=0; t<size(); t++ ) {
        const Fl_Menu_Item *m = &(menu()[t]);
	if ( m->submenu() ) {				// submenu? descend
	    if (*name) SAFE_STRCAT("/");
	    if (m->label()) SAFE_STRCAT(m->label());
	} else {
	    if (m->label()) {				// menu item?
		if ( m == finditem ) {			// found? tack on itemname, done.
		    SAFE_STRCAT("/");
		    SAFE_STRCAT(m->label());
		    return(0);
		}
	    } else {					// end of submenu? pop
	        char *ss = strrchr(name, '/');
		if ( ss ) { *ss = 0; len = strlen(name); }	// "File/Edit" -> "File"
		else { name[0] = '\0'; len = 0; }		// "File" -> ""
		continue;
	    }
	}
    }
    *name = '\0';
    return(-1);						// item not found
}

/**
  Find menu item index, given menu pathname
     eg. "Edit/Copy"
     Will also return submenus, eg. "Edit"
     Returns NULL if not found.
*/
const Fl_Menu_Item * Fl_Menu_::find_item(const char *name) {
  char menupath[1024] = "";	// File/Export

  for ( int t=0; t < size(); t++ ) {
    Fl_Menu_Item *m = menu_ + t;

    if (m->flags&FL_SUBMENU) {
      // IT'S A SUBMENU
      // we do not support searches through FL_SUBMENU_POINTER links
      if (menupath[0]) strlcat(menupath, "/", sizeof(menupath));
      strlcat(menupath, m->label(), sizeof(menupath));
      if (!strcmp(menupath, name)) return m;
    } else {
      if (!m->label()) {
	// END OF SUBMENU? Pop back one level.
	char *ss = strrchr(menupath, '/');
	if ( ss ) *ss = 0;
	else menupath[0] = '\0';
	continue;
      }

      // IT'S A MENU ITEM
      char itempath[1024];	// eg. Edit/Copy
      strcpy(itempath, menupath);
      if (itempath[0]) strlcat(itempath, "/", sizeof(itempath));
      strlcat(itempath, m->label(), sizeof(itempath));
      if (!strcmp(itempath, name)) return m;
    }
  }

  return (const Fl_Menu_Item *)0;
}

/**
  The value is the index into menu() of the last item chosen by
  the user.  It is zero initially.  You can set it as an integer, or set
  it with a pointer to a menu item.  The set routines return non-zero if
  the new value is different than the old one.
*/
int Fl_Menu_::value(const Fl_Menu_Item* m) {
  clear_changed();
  if (value_ != m) {value_ = m; return 1;}
  return 0;
}

/** 
 When user picks a menu item, call this.  It will do the callback.
 Unfortunately this also casts away const for the checkboxes, but this
 was necessary so non-checkbox menus can really be declared const...
*/
const Fl_Menu_Item* Fl_Menu_::picked(const Fl_Menu_Item* v) {
  if (v) {
    if (v->radio()) {
      if (!v->value()) { // they are turning on a radio item
	set_changed();
	((Fl_Menu_Item*)v)->setonly();
      }
      redraw();
    } else if (v->flags & FL_MENU_TOGGLE) {
      set_changed();
      ((Fl_Menu_Item*)v)->flags ^= FL_MENU_VALUE;
      redraw();
    } else if (v != value_) { // normal item
      set_changed();
    }
    value_ = v;
    if (when()&(FL_WHEN_CHANGED|FL_WHEN_RELEASE)) {
      if (changed() || when()&FL_WHEN_NOT_CHANGED) {
	if (value_ && value_->callback_) value_->do_callback((Fl_Widget*)this);
	else do_callback();
      }
    }
  }
  return v;
}

/** Turns the radio item "on" for the menu item and turns off adjacent radio items set. */
void Fl_Menu_Item::setonly() {
  flags |= FL_MENU_RADIO | FL_MENU_VALUE;
  Fl_Menu_Item* j;
  for (j = this; ; ) {	// go down
    if (j->flags & FL_MENU_DIVIDER) break; // stop on divider lines
    j++;
    if (!j->text || !j->radio()) break; // stop after group
    j->clear();
  }
  for (j = this-1; ; j--) { // go up
    if (!j->text || (j->flags&FL_MENU_DIVIDER) || !j->radio()) break;
    j->clear();
  }
}

Fl_Menu_::Fl_Menu_(int X,int Y,int W,int H,const char* l)
/**
  Creates a new Fl_Menu_ widget using the given position, size,
  and label string.  menu() is initialized to null.
*/
: Fl_Widget(X,Y,W,H,l) {
  set_flag(SHORTCUT_LABEL);
  box(FL_UP_BOX);
  when(FL_WHEN_RELEASE_ALWAYS);
  value_ = menu_ = 0;
  alloc = 0;
  selection_color(FL_SELECTION_COLOR);
  textfont(FL_HELVETICA);
  textsize(FL_NORMAL_SIZE);
  textcolor(FL_FOREGROUND_COLOR);
  down_box(FL_NO_BOX);
}

/**
  This returns the number of Fl_Menu_Item structures that make up the
  menu, correctly counting submenus.  This includes the "terminator"
  item at the end.  To copy a menu array you need to copy
  size()*sizeof(Fl_Menu_Item) bytes.  If the menu is
  NULL this returns zero (an empty menu will return 1).
*/
int Fl_Menu_::size() const {
  if (!menu_) return 0;
  return menu_->size();
}

/**
    Sets the menu array pointer directly.  If the old menu is private it is
    deleted.  NULL is allowed and acts the same as a zero-length
    menu.  If you try to modify the array (with add(), replace(), or
    delete()) a private copy is automatically done.
*/
void Fl_Menu_::menu(const Fl_Menu_Item* m) {
  clear();
  value_ = menu_ = (Fl_Menu_Item*)m;
}

// this version is ok with new Fl_Menu_add code with fl_menu_array_owner:

/** 
  Sets the menu array pointer with a copy of m that will be automatically deleted. 
  If ud is not NULL, then all user data pointers are changed in the menus as well.
  See void Fl_Menu_::menu(const Fl_Menu_Item* m). 
*/
void Fl_Menu_::copy(const Fl_Menu_Item* m, void* ud) {
  int n = m->size();
  Fl_Menu_Item* newMenu = new Fl_Menu_Item[n];
  memcpy(newMenu, m, n*sizeof(Fl_Menu_Item));
  menu(newMenu);
  alloc = 1; // make destructor free array, but not strings
  // for convenience, provide way to change all the user data pointers:
  if (ud) for (; n--;) {
    if (newMenu->callback_) newMenu->user_data_ = ud;
    newMenu++;
  }
}

Fl_Menu_::~Fl_Menu_() {
  clear();
}

// Fl_Menu::add() uses this to indicate the owner of the dynamically-
// expanding array.  We must not free this array:
Fl_Menu_* fl_menu_array_owner = 0;

/**
  Same as menu(NULL), set the array pointer to null, indicating
  a zero-length menu.
  
  Menus must not be cleared during a callback to the same menu.
*/
void Fl_Menu_::clear() {
  if (alloc) {
    if (alloc>1) for (int i = size(); i--;)
      if (menu_[i].text) free((void*)menu_[i].text);
    if (this == fl_menu_array_owner)
      fl_menu_array_owner = 0;
    else
      delete[] menu_;
    menu_ = 0;
    value_ = 0;
    alloc = 0;
  }
}

//
// End of "$Id$".
//
