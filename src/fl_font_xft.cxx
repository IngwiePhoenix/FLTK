//
// "$Id$"
//
// Xft font code for the Fast Light Tool Kit (FLTK).
//
// Copyright 2001-2007 Bill Spitzak and others.
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

//
// Draw fonts using Keith Packard's Xft library to provide anti-
// aliased text. Yow!
//
// Many thanks to Carl for making the original version of this.
//
// This font code only requires libXft to work.  Contrary to popular
// belief there is no need to have FreeType, or the Xrender extension
// available to use this code.  You will just get normal Xlib fonts
// (Xft calls them "core" fonts) The Xft algorithms for choosing
// these is about as good as the FLTK ones (I hope to fix it so it is
// exactly as good...), plus it can cache it's results and share them
// between programs, so using this should be a win in all cases. Also
// it should be obvious by comparing this file and fl_font_x.cxx that
// it is a lot easier to program with Xft than with Xlib.
//
// Also, Xft supports UTF-8 text rendering directly, which will allow
// us to support UTF-8 on all platforms more easily.
//
// To actually get antialiasing you need the following:
//
//     1. You have XFree86 4
//     2. You have the XRender extension
//     3. Your X device driver supports the render extension
//     4. You have libXft
//     5. Your libXft has FreeType2 support compiled in
//     6. You have the FreeType2 library
//
// Distributions that have XFree86 4.0.3 or later should have all of this...
//
// Unlike some other Xft packages, I tried to keep this simple and not
// to work around the current problems in Xft by making the "patterns"
// complicated. I belive doing this defeats our ability to improve Xft
// itself. You should edit the ~/.xftconfig file to "fix" things, there
// are several web pages of information on how to do this.
//

#include <X11/Xft/Xft.h>

// The predefined fonts that FLTK has:
static Fl_Fontdesc built_in_table[] = {
{" sans"},
{"Bsans"},
{"Isans"},
{"Psans"},
{" mono"},
{"Bmono"},
{"Imono"},
{"Pmono"},
{" serif"},
{"Bserif"},
{"Iserif"},
{"Pserif"},
{" symbol"},
{" screen"},
{"Bscreen"},
{" dingbats"},
};

Fl_Fontdesc* fl_fonts = built_in_table;

#define current_font (fl_fontsize->font)

Fl_Font fl_font_ = 0;
Fl_Fontsize fl_size_ = 0;
XFontStruct* fl_xfont = 0;
void *fl_xftfont = 0;
const char* fl_encoding_ = "iso8859-1";
Fl_FontSize* fl_fontsize = 0;

void fl_font(Fl_Font fnum, Fl_Fontsize size) {
  if (fnum==-1) { // special case to stop font caching
    fl_font_ = 0; fl_size_ = 0;
    return;
  }
  if (fnum == fl_font_ && size == fl_size_
      && fl_fontsize
      && !strcasecmp(fl_fontsize->encoding, fl_encoding_))
    return;
  fl_font_ = fnum; fl_size_ = size;
  Fl_Fontdesc *font = fl_fonts + fnum;
  Fl_FontSize* f;
  // search the fontsizes we have generated already
  for (f = font->first; f; f = f->next) {
    if (f->size == size && !strcasecmp(f->encoding, fl_encoding_))
      break;
  }
  if (!f) {
    f = new Fl_FontSize(font->name);
    f->next = font->first;
    font->first = f;
  }
  fl_fontsize = f;
#if XFT_MAJOR < 2
  fl_xfont    = f->font->u.core.font;
#endif // XFT_MAJOR < 2
  fl_xftfont = (void*)f->font;
}

static XftFont* fontopen(const char* name, bool core) {
  // Check: does it look like we have been passed an old-school XLFD fontname?
  bool is_xlfd = false;
  int hyphen_count = 0;
  int comma_count = 0;
  unsigned len = strlen(name);
  if (len > 512) len = 512; // ensure we are not passed an unbounded font name
  for(unsigned idx = 0; idx < len; idx++) {
    if(name[idx] == '-') hyphen_count++; // check for XLFD hyphens
    if(name[idx] == ',') comma_count++;  // are there multiple names?
  }
  if(hyphen_count >= 14) is_xlfd = true; // Not a robust check, but good enough?

  fl_open_display();

  if(!is_xlfd) { // Not an XLFD - open as a XFT style name
    XftFont *the_font; // the font we will return;
    XftPattern *fnt_pat = XftPatternCreate(); // the pattern we will use for matching
    int slant = XFT_SLANT_ROMAN;
    int weight = XFT_WEIGHT_MEDIUM;

    /* This "converts" FLTK-style font names back into "regular" names, extracting
     * the BOLD and ITALIC codes as it does so - all FLTK font names are prefixed
     * by 'I' (italic) 'B' (bold) 'P' (bold italic) or ' ' (regular) modifiers.
     * This gives a fairly limited font selection ability, but is retained for
     * compatability reasons. If you really need a more complex choice, you are best
     * calling Fl::set_fonts(*) then selecting the font by font-index rather than by
     * name anyway. Probably.
     * If you want to load a font who's name does actually begin with I, B or P, you
     * MUST use a leading space OR simply use lowercase for the name...
     */
    /* This may be efficient, but it is non-obvious. */
    switch (*name++) {
    case 'I': slant = XFT_SLANT_ITALIC; break; // italic
    case 'P': slant = XFT_SLANT_ITALIC;        // bold-italic (falls-through)
    case 'B': weight = XFT_WEIGHT_BOLD; break; // bold
    case ' ': break;                           // regular
    default: name--;                           // no prefix, restore name
    }

    if(comma_count) { // multiple comma-spearated names were passed
      char *local_name = strdup(name); // duplicate the full name so we can edit the copy
      char *curr = local_name; // points to first name in string
      char *nxt; // next name in string
      do {
        nxt = strchr(curr, ','); // find comma seperator
        if (nxt) {
          *nxt = 0; // terminate first name
          nxt++; // first char of next name
        }

	// Add the current name to the match pattern
	XftPatternAddString(fnt_pat, XFT_FAMILY, curr);

        if(nxt) curr = nxt; // move onto next name (if it exists)
	// Now do a cut-down version of the FLTK name conversion.
	// NOTE: we only use the slant and weight of the first name,
	// subsequent names we ignore this for... But we still need to do the check.
        switch (*curr++) {
        case 'I': break; // italic
        case 'P':        // bold-italic (falls-through)
        case 'B': break; // bold
        case ' ': break; // regular
        default: curr--; // no prefix, restore name
        }

        comma_count--; // decrement name sections count
      } while (comma_count >= 0);
      free(local_name); // release our local copy of font names
    }
    else { // single name was passed - add it directly
      XftPatternAddString(fnt_pat, XFT_FAMILY, name);
    }

    // Construct a match pattern for the font we want...
    XftPatternAddInteger(fnt_pat, XFT_WEIGHT, weight);
    XftPatternAddInteger(fnt_pat, XFT_SLANT, slant);
    XftPatternAddDouble (fnt_pat, XFT_PIXEL_SIZE, (double)fl_size_);
    XftPatternAddString (fnt_pat, XFT_ENCODING, fl_encoding_);
    if (core) {
      XftPatternAddBool(fnt_pat, XFT_CORE, FcTrue);
      XftPatternAddBool(fnt_pat, XFT_RENDER, FcFalse);
    }

    XftPattern *match_pat;  // the best available match on the system
    XftResult match_result; // the result of our matching attempt
    // query the system to find a match for this font
    match_pat = XftFontMatch(fl_display, fl_screen, fnt_pat, &match_result);
    // open the matched font
    the_font = XftFontOpenPattern(fl_display, match_pat);
    // Tidy up the resources we allocated
    XftPatternDestroy(fnt_pat);
//  XftPatternDestroy(match_pat); // FontConfig will destroy this resource for us. We must not!
    return the_font;
  }
  else { // We were passed a font name in XLFD format
    char *local_name = strdup(name);
    if(comma_count) { // This means we were passed multiple XLFD's
      char *pc = strchr(local_name, ',');
      *pc = 0; // terminate the XLFD at the first comma
    }
    XftFont *the_font = XftFontOpenXlfd(fl_display, fl_screen, local_name);
    free(local_name);
   return the_font;
  }
} // end of fontopen

Fl_FontSize::Fl_FontSize(const char* name) {
  encoding = fl_encoding_;
  size = fl_size_;
#if HAVE_GL
  listbase = 0;
#endif // HAVE_GL
  font = fontopen(name, false);
}

Fl_FontSize::~Fl_FontSize() {
  if (this == fl_fontsize) fl_fontsize = 0;
//  XftFontClose(fl_display, font);
}

int fl_height() {
  if (current_font) return current_font->ascent + current_font->descent;
  else return -1;
}

int fl_descent() {
  if (current_font) return current_font->descent;
  else return -1;
}

double fl_width(const char *str, int n) {
  if (!current_font) return -1.0;
  XGlyphInfo i;
  XftTextExtents8(fl_display, current_font, (XftChar8 *)str, n, &i);
  return i.xOff;
}

double fl_width(uchar c) {
  return fl_width((const char *)(&c), 1);
}

#if HAVE_GL
/* This code is used by opengl to get a bitmapped font. The original XFT-1 code
 * used XFT's "core" fonts methods to load an XFT font that was actually a
 * X-bitmap font, that could then be readily used with GL.
 * But XFT-2 does not provide that ability, and there is no easy method to use
 * an XFT font directly with GL. So...
*/

#  if XFT_MAJOR > 1
// This function attempts, on XFT2 systems, to find a suitable "core" Xfont
// for GL to use, since we dont have an XglUseXftFont(...) function.
// There's probably a better way to do this. I can't believe it is this hard...
// Anyway... This code attempts to make an XLFD out of the fltk-style font
// name it is passed, then tries to load that font. Surprisingly, this quite
// often works - boxes that have XFT generally also have a fontserver that
// can serve TTF and other fonts to X, and so the font name that fltk makes
// from the XFT name often also "exists" as an "core" X font...
// If this code fails to load the requested font, it falls back through a
// series of tried 'n tested alternatives, ultimately resorting to what the
// original fltk code did.
// NOTE:
// On my test boxes (FC6, FC7) this works well for the fltk "built-in" font names.
static XFontStruct* load_xfont_for_xft2(void) {
  XFontStruct* xgl_font = 0;
  int size = fl_size_;
  char *weight = "medium"; // no specifc weight requested - accept any
  char slant = 'r';   // regular non-italic by default
  char xlfd[128];     // we will put our synthetic XLFD in here
  char *pc = strdup(fl_fonts[fl_font_].name); // what font were we asked for?
  char *name = pc;    // keep a handle to the original name for freeing later
  // Parse the "fltk-name" of the font
  switch (*name++) {
  case 'I': slant = 'i'; break;     // italic
  case 'P': slant = 'i';            // bold-italic (falls-through)
  case 'B': weight = "bold"; break; // bold
  case ' ': break;                  // regular
  default: name--;                  // no prefix, restore name
  }

  // first, we do a query with no prefered size, to see if the font exists at all
  snprintf(xlfd, 128, "-*-*%s*-%s-%c-*--*-*-*-*-*-*-*-*", name, weight, slant); // make up xlfd style name
  xgl_font = XLoadQueryFont(fl_display, xlfd);
  if(xgl_font) { // the face exists, but can we get it in a suitable size?
    XFreeFont(fl_display, xgl_font); // release the non-sized version
    snprintf(xlfd, 128, "-*-*%s*-%s-%c-*--*-%d-*-*-*-*-*-*", name, weight, slant, (size*10));
    xgl_font = XLoadQueryFont(fl_display, xlfd); // attempt to load the font at the right size
  }
  free(pc); // release our copy of the font name

  // if we have nothing loaded, try a generic proportional font
  if(!xgl_font) {
    snprintf(xlfd, 128, "-*-helvetica-*-%c-*--*-%d-*-*-*-*-*-*", slant, (size*10));
    xgl_font = XLoadQueryFont(fl_display, xlfd);
  }
  // If that still didn't work, try this instead
  if(!xgl_font) {
    snprintf(xlfd, 128, "-*-courier-medium-%c-*--*-%d-*-*-*-*-*-*", slant, (size*10));
    xgl_font = XLoadQueryFont(fl_display, xlfd);
  }
  // Last chance fallback - this usually loads something...
  if (!xgl_font) xgl_font = XLoadQueryFont(fl_display, "fixed");

  return xgl_font;
} // end of load_xfont_for_xft2
#  endif

XFontStruct* fl_xxfont() {
#  if XFT_MAJOR > 1
  // kludge! XFT 2 and later does not provide core fonts for us to use with GL
  // try to load a bitmap X font instead
  static XFontStruct* xgl_font = 0;
  static int glsize = 0;
  static int glfont = -1;
  // Do we need to load a new font?
  if ((!xgl_font) || (glsize != fl_size_) || (glfont != fl_font_)) {
    if (xgl_font) XFreeFont(fl_display, xgl_font); // font already loaded, free it
    glsize = fl_size_; // record current font size
    glfont = fl_font_; // and face
    // create a dummy XLFD for some font of the appropriate size...
    xgl_font = load_xfont_for_xft2();
  }
  return xgl_font;
#  else // XFT-1 provides a means to load a "core" font directly
  if (current_font->core) return current_font->u.core.font; // is the current font a "core" font? If so, use it.
  static XftFont* xftfont;
  if (xftfont) XftFontClose (fl_display, xftfont);
  xftfont = fontopen(fl_fonts[fl_font_].name, true); // else request XFT to load a suitable "core" font instead.
  return xftfont->u.core.font;
#  endif // XFT_MAJOR > 1
}
#endif // HAVE_GL

#if USE_OVERLAY
// Currently Xft does not work with colormapped visuals, so this probably
// does not work unless you have a true-color overlay.
extern bool fl_overlay;
extern Colormap fl_overlay_colormap;
extern XVisualInfo* fl_overlay_visual;
#endif

// For some reason Xft produces errors if you destroy a window whose id
// still exists in an XftDraw structure. It would be nice if this is not
// true, a lot of junk is needed to try to stop this:

static XftDraw* draw;
static Window draw_window;
#if USE_OVERLAY
static XftDraw* draw_overlay;
static Window draw_overlay_window;
#endif

void fl_destroy_xft_draw(Window id) {
  if (id == draw_window)
    XftDrawChange(draw, draw_window = fl_message_window);
#if USE_OVERLAY
  if (id == draw_overlay_window)
    XftDrawChange(draw_overlay, draw_overlay_window = fl_message_window);
#endif
}

void fl_draw(const char *str, int n, int x, int y) {
#if USE_OVERLAY
  XftDraw*& draw = fl_overlay ? draw_overlay : ::draw;
  if (fl_overlay) {
    if (!draw)
      draw = XftDrawCreate(fl_display, draw_overlay_window = fl_window,
			   fl_overlay_visual->visual, fl_overlay_colormap);
    else //if (draw_overlay_window != fl_window)
      XftDrawChange(draw, draw_overlay_window = fl_window);
  } else
#endif
  if (!draw)
    draw = XftDrawCreate(fl_display, draw_window = fl_window,
			 fl_visual->visual, fl_colormap);
  else //if (draw_window != fl_window)
    XftDrawChange(draw, draw_window = fl_window);

  Region region = fl_clip_region();
  if (region && XEmptyRegion(region)) return;
  XftDrawSetClip(draw, region);

  // Use fltk's color allocator, copy the results to match what
  // XftCollorAllocValue returns:
  XftColor color;
  color.pixel = fl_xpixel(fl_color_);
  uchar r,g,b; Fl::get_color(fl_color_, r,g,b);
  color.color.red   = ((int)r)*0x101;
  color.color.green = ((int)g)*0x101;
  color.color.blue  = ((int)b)*0x101;
  color.color.alpha = 0xffff;

  XftDrawString8(draw, &color, current_font, x, y, (XftChar8 *)str, n);
}

void fl_draw(const char* str, int n, float x, float y) {
  fl_draw(str, n, (int)x, (int)y);
}

//
// End of "$Id$"
//
