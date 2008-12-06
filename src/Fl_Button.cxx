//
// "$Id$"
//
// Button widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2008 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>

// There are a lot of subclasses, named Fl_*_Button.  Some of
// them are implemented by setting the type() value and testing it
// here.  This includes Fl_Radio_Button and Fl_Toggle_Button

/**
  Sets the current value of the button.
  A non-zero value sets the button to 1 (ON), and zero sets it to 0 (OFF).
  \param[in] v button value.
  \see set(), clear()
 */
int Fl_Button::value(int v) {
  v = v ? 1 : 0;
  oldval = v;
  clear_changed();
  if (value_ != v) {
    value_ = v;
    if (box()) redraw();
    else redraw_label();
    return 1;
  } else {
    return 0;
  }
}

/**
  Turns on this button and turns off all other radio buttons in the group
  (calling \c value(1) or \c set() does not do this).
 */
void Fl_Button::setonly() { // set this radio button on, turn others off
  value(1);
  Fl_Group* g = (Fl_Group*)parent();
  Fl_Widget*const* a = g->array();
  for (int i = g->children(); i--;) {
    Fl_Widget* o = *a++;
    if (o != this && o->type()==FL_RADIO_BUTTON) ((Fl_Button*)o)->value(0);
  }
}

void Fl_Button::draw() {
  if (type() == FL_HIDDEN_BUTTON) return;
  Fl_Color col = value() ? selection_color() : color();
  draw_box(value() ? (down_box()?down_box():fl_down(box())) : box(), col);
  if (labeltype() == FL_NORMAL_LABEL && value()) {
    Fl_Color c = labelcolor();
    labelcolor(fl_contrast(c, col));
    draw_label();
    labelcolor(c);
  } else draw_label();
  if (Fl::focus() == this) draw_focus();
}

int Fl_Button::handle(int event) {
  int newval;
  switch (event) {
    case FL_ENTER:
    case FL_LEAVE:
      //  if ((value_?selection_color():color())==FL_GRAY) redraw();
      return 1;
    case FL_PUSH:
      if (Fl::visible_focus() && handle(FL_FOCUS)) Fl::focus(this);
    case FL_DRAG:
      if (Fl::event_inside(this)) {
        if (type() == FL_RADIO_BUTTON) newval = 1;
        else newval = !oldval;
      } else
      {
        clear_changed();
        newval = oldval;
      }
      if (newval != value_) {
        value_ = newval;
        set_changed();
        redraw();
        if (when() & FL_WHEN_CHANGED) do_callback();
      }
      return 1;
    case FL_RELEASE:
      if (value_ == oldval) {
        if (when() & FL_WHEN_NOT_CHANGED) do_callback();
        return 1;
      }
      set_changed();
      if (type() == FL_RADIO_BUTTON) setonly();
      else if (type() == FL_TOGGLE_BUTTON) oldval = value_;
      else {
        value(oldval);
        set_changed();
        if (when() & FL_WHEN_CHANGED) do_callback();
      }
      if (when() & FL_WHEN_RELEASE) do_callback();
      return 1;
    case FL_SHORTCUT:
      if (!(shortcut() ?
            Fl::test_shortcut(shortcut()) : test_shortcut())) return 0;

      if (Fl::visible_focus() && handle(FL_FOCUS)) Fl::focus(this);

      if (type() == FL_RADIO_BUTTON && !value_) {
        setonly();
        set_changed();
        if (when() & FL_WHEN_CHANGED) do_callback();
      } else if (type() == FL_TOGGLE_BUTTON) {
        value(!value());
        set_changed();
        if (when() & FL_WHEN_CHANGED) do_callback();
      } else if ( (type() == FL_NORMAL_BUTTON) && when()&FL_WHEN_CHANGED) {
        //  Not already active by GUI push
        if ( !value_ ) {
          value_ = !oldval;
          set_changed();
          do_callback();
        }
      } else if (when() & FL_WHEN_RELEASE) do_callback();
      return 1;
    case FL_FOCUS :
    case FL_UNFOCUS :
      if (Fl::visible_focus()) {
        if (box() == FL_NO_BOX) {
          // Widgets with the FL_NO_BOX boxtype need a parent to
          // redraw, since it is responsible for redrawing the
          // background...
          int X = x() > 0 ? x() - 1 : 0;
          int Y = y() > 0 ? y() - 1 : 0;
          if (window()) window()->damage(FL_DAMAGE_ALL, X, Y, w() + 2, h() + 2);
        } else redraw();
        return 1;
      } else return 0;
    case FL_KEYBOARD :
      if (Fl::focus() == this && Fl::event_key() == ' ' &&
          !(Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META))) {
        set_changed();
        if (type() == FL_RADIO_BUTTON && !value_) {
          setonly();
          if (when() & FL_WHEN_CHANGED) do_callback();
        } else if (type() == FL_TOGGLE_BUTTON) {
          value(!value());
          if (when() & FL_WHEN_CHANGED) do_callback();
        }
        if (when() & FL_WHEN_RELEASE) do_callback();
        return 1;
      }
      return 0;

    case FL_KEYUP :
      if ( (type() == FL_NORMAL_BUTTON) && when()&FL_WHEN_CHANGED) {

        int key = Fl::event_key();

        // Check for a shortcut that includes state keys(FL_SHIFT, FL_CTRL or FL_ALT) and 
        // the state key is released
        if ( (shortcut()&FL_SHIFT && ((key == FL_Shift_L) || (key == FL_Shift_R))) ||
             (shortcut()&FL_CTRL && ((key == FL_Control_L) || (key == FL_Control_R))) || 
             (shortcut()&FL_ALT && ((key == FL_Alt_L) || (key == FL_Alt_R))) ) {
          value(oldval);
          set_changed();
          do_callback();
          return 0;   //  In case multiple shortcut keys are pressed that use state keys
        }
        // shortcut key alone
        else if (  !(shortcut() ?  Fl::test_shortcut(shortcut()) : test_shortcut())) {
          return 0;
        }
        // disable button
        else if ( value_ ) {
          value(oldval);
          set_changed();
          do_callback();
          return 1;
        }
      }

    default:
      return 0;
  }
}

/**
  The constructor creates the button using the given position, size and label.
  \param[in] X, Y, W, H position and size of the widget
  \param[in] L widget label, default is no label
 */
Fl_Button::Fl_Button(int X, int Y, int W, int H, const char *L)
: Fl_Widget(X,Y,W,H,L) {
  box(FL_UP_BOX);
  down_box(FL_NO_BOX);
  value_ = oldval = 0;
  shortcut_ = 0;
  set_flag(SHORTCUT_LABEL);
}

//
// End of "$Id$".
//
