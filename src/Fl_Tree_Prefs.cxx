//
// "$Id$"
//

#include <FL/Fl.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Tree_Prefs.H>
#include <string.h>

//////////////////////
// Fl_Tree_Prefs.cxx
//////////////////////
//
// Fl_Tree -- This file is part of the Fl_Tree widget for FLTK
// Copyright (C) 2009 by Greg Ercolano.
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

// INTERNAL: BUILT IN OPEN/STOW XPMS
//    These can be replaced via prefs.openicon()/closeicon()
//
static const char *L_open_xpm[] = {
  "11 11 3 1",
  ".	c #fefefe",
  "#	c #444444",
  "@	c #000000",
  "###########",
  "#.........#",
  "#.........#",
  "#....@....#",
  "#....@....#",
  "#..@@@@@..#",
  "#....@....#",
  "#....@....#",
  "#.........#",
  "#.........#",
  "###########"};
static Fl_Pixmap L_openpixmap(L_open_xpm);

static const char *L_close_xpm[] = {
  "11 11 3 1",
  ".	c #fefefe",
  "#	c #444444",
  "@	c #000000",
  "###########",
  "#.........#",
  "#.........#",
  "#.........#",
  "#.........#",
  "#..@@@@@..#",
  "#.........#",
  "#.........#",
  "#.........#",
  "#.........#",
  "###########"};
static Fl_Pixmap L_closepixmap(L_close_xpm);

/// Sets the default icon to be used as the 'open' icon
/// when items are add()ed to the tree.
/// This overrides the built in default '[+]' icon.
///
/// \param[in] val -- The new pixmap, or zero to use the default [+] icon.
///
void Fl_Tree_Prefs::openicon(Fl_Pixmap *val) {
  _openpixmap = val ? val : &L_openpixmap;
}

/// Sets the icon to be used as the 'close' icon.
/// This overrides the built in default '[-]' icon.
///
/// \param[in] val -- The new pixmap, or zero to use the default [-] icon.
///
void Fl_Tree_Prefs::closeicon(Fl_Pixmap *val) {
  _closepixmap = val ? val : &L_closepixmap;
}

/// Fl_Tree_Prefs constructor
Fl_Tree_Prefs::Fl_Tree_Prefs() {
  _labelfont              = FL_HELVETICA;
  _labelsize              = FL_NORMAL_SIZE;
  _marginleft             = 6;
  _margintop              = 3;
  //_marginright          = 3;
  //_marginbottom         = 3;
  _openchild_marginbottom = 0;
  _usericonmarginleft     = 3;
  _labelmarginleft        = 3;
  _linespacing            = 0;
  _fgcolor                = FL_BLACK;
  _bgcolor                = FL_WHITE;
  _selectcolor            = FL_DARK_BLUE;
  _inactivecolor          = FL_GRAY;
  _connectorcolor         = Fl_Color(43);
  _connectorstyle         = FL_TREE_CONNECTOR_DOTTED;
  _openpixmap             = &L_openpixmap;
  _closepixmap            = &L_closepixmap;
  _userpixmap             = 0;
  _showcollapse           = 1;
  _showroot               = 1;
  _connectorwidth         = 17;
  _sortorder              = FL_TREE_SORT_NONE;
  _selectbox              = FL_FLAT_BOX;
  _selectmode             = FL_TREE_SELECT_SINGLE;
  // Let fltk's current 'scheme' affect defaults
  if ( Fl::scheme() ) {
    if ( strcmp(Fl::scheme(), "gtk+") == 0 ) {
	    _selectbox = _FL_GTK_THIN_UP_BOX;
    } else if ( strcmp(Fl::scheme(), "plastic") == 0 ) {
	    _selectbox = _FL_PLASTIC_THIN_UP_BOX;
    }
  }
}

//
// End of "$Id$".
//
