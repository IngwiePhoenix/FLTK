//
// "$Id$"
//
// FLUID undo definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
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

#ifndef undo_h
#  define undo_h

extern int undo_current;		// Current undo level in buffer
extern int undo_last;			// Last undo level in buffer
extern int undo_save;			// Last undo level that was saved

void redo_cb(Fl_Widget *, void *);	// Redo menu callback
void undo_cb(Fl_Widget *, void *);	// Undo menu callback
void undo_checkpoint();			// Save current file to undo buffer
void undo_clear();			// Clear undo buffer
void undo_resume();			// Resume undo checkpoints
void undo_suspend();			// Suspend undo checkpoints

#endif // !undo_h

//
// End of "$Id$".
//
