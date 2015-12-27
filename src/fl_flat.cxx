#include <FL/Fl.H>
#include <FL/fl_draw.H>

static void fltk_color(Fl_Color c) {
  Fl::set_box_color(c);
}

static void frame_rect(int x, int y, int w, int h, Fl_Color c) {
  fl_rect(x, y, w, h, c);
}

static void up_frame(int x, int y, int w, int h, Fl_Color c) {
  frame_rect(x, y, w, h, c);
}

static void up_box(int x, int y, int w, int h, Fl_Color c) {
  fl_rectf(x+1, y+1, w-2, h-2, c);
  c = fl_rgb_color(0xb8, 0xb8, 0xb8);
  frame_rect(x, y, w, h, c);
}

static void thin_up_box(int x, int y, int w, int h, Fl_Color c) {
  up_box(x, y, w, h, c);
}

static void down_frame(int x, int y, int w, int h, Fl_Color c) {
  frame_rect(x, y, w, h, c);
}

static void down_box(int x, int y, int w, int h, Fl_Color c) {
  fl_rectf(x+1, y+1, w-2, h-2, c);
  frame_rect(x, y, w, h, FL_SELECTION_COLOR);
}

static void thin_down_box(int x, int y, int w, int h, Fl_Color c) {
  down_box(x, y, w, h, c);
}

static void round_up_box(int x, int y, int w, int h, Fl_Color c) {
  fltk_color(c);
  fl_pie(x, y, w, h, 0.0, 360.0);
  c = fl_rgb_color(0xb8, 0xb8, 0xb8);
  fltk_color(c);
  fl_arc(x, y, w, h, 0.0, 360.0);
}

static void round_down_box(int x, int y, int w, int h, Fl_Color c) {
  fltk_color(c);
  fl_pie(x, y, w, h, 0.0, 360.0);
  c = fl_rgb_color(0xb8, 0xb8, 0xb8);
  fltk_color(c);
  fl_arc(x, y, w, h, 0.0, 360.0);
}

extern void fl_internal_boxtype(Fl_Boxtype, Fl_Box_Draw_F*);

Fl_Boxtype fl_define_FL_FLAT_UP_BOX() {
  fl_internal_boxtype(_FL_FLAT_UP_BOX, up_box);
  fl_internal_boxtype(_FL_FLAT_DOWN_BOX, down_box);
  fl_internal_boxtype(_FL_FLAT_UP_FRAME, up_frame);
  fl_internal_boxtype(_FL_FLAT_DOWN_FRAME, down_frame);
  fl_internal_boxtype(_FL_FLAT_THIN_UP_BOX, thin_up_box);
  fl_internal_boxtype(_FL_FLAT_THIN_DOWN_BOX, thin_down_box);
  fl_internal_boxtype(_FL_FLAT_ROUND_UP_BOX, round_up_box);
  fl_internal_boxtype(_FL_FLAT_ROUND_DOWN_BOX, round_down_box);
  return _FL_FLAT_UP_BOX;
}
