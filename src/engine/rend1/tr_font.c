/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_font.c
//
//
// The font system uses FreeType 2.x to render TrueType fonts for use within the game.
// As of this writing ( Nov, 2000 ) Team Arena uses these fonts for all of the ui and
// about 90% of the cgame presentation. A few areas of the CGAME were left uses the old
// fonts since the code is shared with standard Q3A.
//
// If you include this font rendering code in a commercial product you MUST include the
// following somewhere with your product, see www.freetype.org for specifics or changes.
// The Freetype code also uses some hinting techniques that MIGHT infringe on patents
// held by apple so be aware of that also.
//
// As of Q3A 1.25+ and Team Arena, we are shipping the game with the font rendering code
// disabled. This removes any potential patent issues and it keeps us from having to
// distribute an actual TrueTrype font which is 1. expensive to do and 2. seems to require
// an act of god to accomplish.
//
// What we did was pre-render the fonts using FreeType ( which is why we leave the FreeType
// credit in the credits ) and then saved off the glyph data and then hand touched up the
// font bitmaps so they scale a bit better in GL.
//
// There are limitations in the way fonts are saved and reloaded in that it is based on
// point size and not name. So if you pre-render Helvetica in 18 point and Impact in 18 point
// you will end up with a single 18 point data file and image set. Typically you will want to
// choose 3 sizes to best approximate the scaling you will be doing in the ui scripting system
//
// In the UI Scripting code, a scale of 1.0 is equal to a 48 point font. In Team Arena, we
// use three or four scales, most of them exactly equaling the specific rendered size. We
// rendered three sizes in Team Arena, 12, 16, and 20.
//
// To generate new font data you need to go through the following steps.
// 1. delete the fontImage_x_xx.tga files and fontImage_xx.dat files from the fonts path.
// 2. in a ui script, specificy a font, fontSmall, and fontBig keyword with font name and
//    point size. the original TrueType fonts must exist in fonts at this point.
// 3. run the game, you should see things normally.
// 4. Exit the game and there will be three dat files and at least three tga files. The
//    tga's are in 256x256 pages so if it takes three images to render a 24 point font you
//    will end up with fontImage_0_24.tga through fontImage_2_24.tga
// 5. In future runs of the game, the system looks for these images and data files when a s
//    specific point sized font is rendered and loads them for use.
// 6. Because of the original beta nature of the FreeType code you will probably want to hand
//    touch the font bitmaps.
//
// Currently a define in the project turns on or off the FreeType code which is currently
// defined out. To pre-render new fonts you need enable the define ( BUILD_FREETYPE ) and
// uncheck the exclude from build check box in the FreeType2 area of the Renderer project.


#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../rendc/tr_public.h"
#include "tr_common.h"
#include "tr_local.h"

extern void R_IssuePendingRenderCommands(void);

#if !defined NO_FREETYPE
#  include <ft2build.h>
#  include FT_FREETYPE_H
#  include FT_ERRORS_H
#  include FT_SYSTEM_H
#  include FT_IMAGE_H
#  include FT_OUTLINE_H

//.......................
// Conversion from 26.6 format to pixels (see bottom of the file)
#  define Fx6Floor(x) ((x) & -64)
#  define Fx6Ceil(x) (((x) + 63) & -64)
#  define Fx6Trunc(x) ((x) >> 6)
//.......................

FT_Library ftLibrary = NULL;
#endif  // NO_FREETYPE

#define ATLAS_W 256
#define ATLAS_H ATLAS_W

#define MAX_FONTS 6
static int        registeredFontCount = 0;
static fontInfo_t registeredFont[MAX_FONTS];

#if !defined NO_FREETYPE
//.......................
// R_GetGlyphInfo
// Fit the glyph into a "box" on a 64 unit grid (64u = 1pixel)
// Each pixel contains 64 units
//   left  = glyph X rounded down to the nearest pixel
//   right = (glyph x + glyph width) rounded up to the nearest pixel
//   width = (right - left) / 64 = number of pixels the glyph spans
//   same for top, bottom, and height
void R_GetGlyphInfo(FT_GlyphSlot glyph, int* left, int* right, int* width, int* top, int* bottom, int* height, int* pitch) {
  *left   = Fx6Floor(glyph->metrics.horiBearingX);
  *right  = Fx6Ceil(glyph->metrics.horiBearingX + glyph->metrics.width);
  *width  = Fx6Trunc(*right - *left);

  *top    = Fx6Ceil(glyph->metrics.horiBearingY);
  *bottom = Fx6Floor(glyph->metrics.horiBearingY - glyph->metrics.height);
  *height = Fx6Trunc(*top - *bottom);
  *pitch  = (*width + 3) & -4;  // *pitch  = ( qtrue ? (*width+3) & -4 : (*width+7) >> 3 );
}
//.......................

FT_Bitmap* R_GetGlyphBitmap(FT_GlyphSlot glyph, glyphInfo_t* glyphOut) {
  int left, right, width, top, bottom, height, pitch, size;
  R_GetGlyphInfo(glyph, &left, &right, &width, &top, &bottom, &height, &pitch);

  FT_Bitmap* bit2;
  if (glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
    size             = pitch * height;
    bit2             = ri.Malloc(sizeof(FT_Bitmap));
    bit2->width      = width;
    bit2->rows       = height;
    bit2->pitch      = pitch;
    bit2->pixel_mode = FT_PIXEL_MODE_GRAY;
    // bit2->pixel_mode = FT_PIXEL_MODE_MONO;
    bit2->buffer     = ri.Malloc(pitch * height);
    bit2->num_grays  = 256;

    Com_Memset(bit2->buffer, 0, size);

    FT_Outline_Translate(&glyph->outline, -left, -bottom);

    FT_Outline_Get_Bitmap(ftLibrary, &glyph->outline, bit2);

    glyphOut->height = height;
    glyphOut->pitch  = pitch;
    glyphOut->top    = (glyph->metrics.horiBearingY >> 6) + 1;
    glyphOut->bottom = bottom;

    return bit2;
  } else {
    ri.Printf(PRINT_ALL, "Non-outline fonts are not supported\n");
  }
  return NULL;
}

typedef struct {
  char  idlength;  // length of a string located after the header.
  char  colormaptype;
  char  datatypecode;
  short colormaporigin;
  short colormaplength;
  char  colormapdepth;
  short x_origin;
  short y_origin;
  short width;
  short height;
  char  bitsperpixel;
  char  imagedescriptor;
} TgaHeader;
//.......................
// TGA header indexes
#  define TGA_DATATYPECODE 2
#  define TGA_WIDTH_B1 12
#  define TGA_WIDTH_B2 13
#  define TGA_HEIGHT_B1 14
#  define TGA_HEIGHT_B2 15
#  define TGA_BITSPERPIXEL 16
#  define TGA_IMGDESCRIPTOR 17
//.......................
#  define TGA_HSIZE 18
//.......................

void WriteTGA(char* filename, byte* data, int width, int height) {
  const int    colorChannels = RGBA;
  const size_t bufsize       = width * height * colorChannels + TGA_HSIZE;
  byte*        buffer        = ri.Malloc(bufsize);  // alloc enough memory for the tga image
  Com_Memset(buffer, 0, TGA_HSIZE);                 // Set all bytes of the buffer header to 0
  buffer[TGA_DATATYPECODE]  = 2;                    // uncompressed type
  buffer[TGA_WIDTH_B1]      = width & 255;          // First byte of width
  buffer[TGA_WIDTH_B2]      = width >> 8;           // Second byte of width
  buffer[TGA_HEIGHT_B1]     = height & 255;         // First byte of height
  buffer[TGA_HEIGHT_B2]     = height >> 8;          // Second byte of width
  buffer[TGA_BITSPERPIXEL]  = 32;                   // pixel size
  buffer[TGA_IMGDESCRIPTOR] = 8;                    // Required as 8 for Targa32

  // swap rgb to bgr
  for (size_t i = 18; i < bufsize; i += colorChannels) {
    buffer[i + R] = data[i - TGA_HSIZE + B];  // blue
    buffer[i + G] = data[i - TGA_HSIZE + G];  // green
    buffer[i + B] = data[i - TGA_HSIZE + R];  // red
    buffer[i + A] = data[i - TGA_HSIZE + A];  // alpha
  }

  // flip upside down
  byte *src, *dst;
  byte* flip = (byte*)ri.Malloc(width * 4);
  for (int row = 0; row < height / 2; row++) {
    src = buffer + TGA_HSIZE + row * colorChannels * width;
    dst = buffer + TGA_HSIZE + (height - row - 1) * colorChannels * width;

    Com_Memcpy(flip, src, width * colorChannels);
    Com_Memcpy(src, dst, width * colorChannels);
    Com_Memcpy(dst, flip, width * colorChannels);
  }
  ri.Free(flip);

  ri.FS_WriteFile(filename, buffer, bufsize);
  ri.Free(buffer);
}

static glyphInfo_t* RE_CreateGlyphInfo(byte* imageOut, int* xOut, int* yOut, int* maxHeight, FT_Face face, const byte c, qboolean calcHeight) {
  FT_Bitmap*         bitmap = NULL;
  static glyphInfo_t glyph;
  Com_Memset(&glyph, 0, sizeof(glyphInfo_t));
  // make sure everything is here
  if (face != NULL) {
    FT_Load_Glyph(face, FT_Get_Char_Index(face, c), FT_LOAD_DEFAULT);
    bitmap = R_GetGlyphBitmap(face->glyph, &glyph);
    if (!bitmap) { return &glyph; }

    glyph.xSkip = (face->glyph->metrics.horiAdvance >> 6) + 1;
    if (glyph.height > *maxHeight) { *maxHeight = glyph.height; }

    if (calcHeight) {
      ri.Free(bitmap->buffer);
      ri.Free(bitmap);
      return &glyph;
    }

    // Convert to power of 2 sizes, so we dont get any scaling from the gl upload
    // for (scaled_width = 1 ; scaled_width < glyph.pitch ; scaled_width<<=1)    ;
    // for (scaled_height = 1 ; scaled_height < glyph.height ; scaled_height<<=1)    ;

    float scaled_width  = glyph.pitch;
    float scaled_height = glyph.height;
    // we need to make sure we fit
    if (*xOut + scaled_width + 1 >= 255) {
      *xOut = 0;
      *yOut += *maxHeight + 1;
    }

    if (*yOut + *maxHeight + 1 >= 255) {
      *yOut = -1;
      *xOut = -1;
      ri.Free(bitmap->buffer);
      ri.Free(bitmap);
      return &glyph;
    }

    byte* src = bitmap->buffer;
    byte* dst = imageOut + (*yOut * 256) + *xOut;
    // index = (row * width) + column
    // row * width  -> gives the index to the first element of a row
    // + column     -> adds the column number to the index, to get the current position in the row
    //
    // Loop to convert the 1bpp mono source raster to an 8bpp raster
    // The marker effectively tracks the current position within a given octet.
    //   You can think of it as a horizontal translation from a position index into a bit position
    //   such that when you're iterating position j (column), the marker is the bit set at j mod 8.
    if (bitmap->pixel_mode == FT_PIXEL_MODE_MONO) {
      for (int row = 0; row < glyph.height; row++) {
        byte* _src   = src;    // Assign _src to incoming src
        byte* _dst   = dst;    // Assign _dst to incoming dst
        byte  marker = 0x80;   // binary 1000_0000  // Marker/counter for which bit we are checking
        byte  val    = *_src;  // Assign val to the value that src points to
        for (int column = 0; column < glyph.pitch; column++) {
          // Because of iterating over bytes, the current "block of 8 pixels" (val) needs to be advanced every 8 pixels (=byte)
          if (marker == 0x80) { val = *_src++; }  // Assign the value, and move the pointer to the next byte
          // If the column-th pixel is on, set destination to 0xff. This converts 0b1 into 0b1111_1111
          if (val & marker) { *_dst = 0xff; }  // If this bit is active, activate all bits in the destination byte

          marker >>= 1;                        // Increment the bit marker
          if (marker == 0) { marker = 0x80; }  // If we have done a full cycle (bit marker is 0), reset the marker
          _dst++;                              // Move to the next byte of dst
        }
        src += glyph.pitch;  // After one row is processed, move src pointer to the next
        dst += 256;          // Move dst pointer to the next row
      }
    } else {
      for (int i = 0; i < glyph.height; i++) {
        Com_Memcpy(dst, src, glyph.pitch);
        src += glyph.pitch;
        dst += 256;
      }
    }

    // we now have an 8 bit per pixel grey scale bitmap
    // that is width wide and pf->ftSize->metrics.y_ppem tall
    glyph.imageHeight = scaled_height;
    glyph.imageWidth  = scaled_width;
    glyph.s           = (float)*xOut / 256;
    glyph.t           = (float)*yOut / 256;
    glyph.s2          = glyph.s + (float)scaled_width / 256;
    glyph.t2          = glyph.t + (float)scaled_height / 256;

    *xOut += scaled_width + 1;

    ri.Free(bitmap->buffer);
    ri.Free(bitmap);
  }

  return &glyph;
}
#endif  // NO_FREETYPE

static int   fdOffset;
static byte* fdFile;

typedef unsigned int uint;

int readInt(void) {
  int i = ((uint)fdFile[fdOffset] | ((uint)fdFile[fdOffset + 1] << 8) | ((uint)fdFile[fdOffset + 2] << 16) | ((uint)fdFile[fdOffset + 3] << 24));
  fdOffset += 4;
  return i;
}

typedef union {
  byte  arr[4];
  float fl;
} flUnion;

float readFloat(void) {
  flUnion f;
#if defined Q3_BIG_ENDIAN
  f.arr[0] = fdFile[fdOffset + 3];
  f.arr[1] = fdFile[fdOffset + 2];
  f.arr[2] = fdFile[fdOffset + 1];
  f.arr[3] = fdFile[fdOffset + 0];
#elif defined Q3_LITTLE_ENDIAN
  f.arr[0] = fdFile[fdOffset + 0];
  f.arr[1] = fdFile[fdOffset + 1];
  f.arr[2] = fdFile[fdOffset + 2];
  f.arr[3] = fdFile[fdOffset + 3];
#endif
  fdOffset += 4;
  return f.fl;
}


#define FONT_CACHEDIR "cache/"
void RE_RegisterFont(const char* fontName, int pointSize, fontInfo_t* font) {
  if (!fontName) {
    ri.Printf(PRINT_ALL, "%s: called with empty name\n", __func__);
    return;
  }
  if (pointSize <= 0) { pointSize = 12; }
  // R_IssuePendingRenderCommands();
  if (registeredFontCount >= MAX_FONTS) {
    ri.Printf(PRINT_WARNING, "%s: Too many fonts registered already.\n", __func__);
    return;
  }

  char name[1024];
  Com_sprintf(name, sizeof(name), FONT_CACHEDIR "%s_%i.dat", fontName, pointSize);
  for (int i = 0; i < registeredFontCount; i++) {
    if (Q_stricmp(name, registeredFont[i].name) == 0) {
      Com_Memcpy(font, &registeredFont[i], sizeof(fontInfo_t));
      return;
    }
  }

  void* faceData;
  int   len = ri.FS_ReadFile(name, NULL);
  if (len == sizeof(fontInfo_t)) {
    ri.FS_ReadFile(name, &faceData);
    fdOffset = 0;
    fdFile   = faceData;
    for (int i = 0; i < GLYPHS_PER_FONT; i++) {
      font->glyphs[i].height      = readInt();
      font->glyphs[i].top         = readInt();
      font->glyphs[i].bottom      = readInt();
      font->glyphs[i].pitch       = readInt();
      font->glyphs[i].xSkip       = readInt();
      font->glyphs[i].imageWidth  = readInt();
      font->glyphs[i].imageHeight = readInt();
      font->glyphs[i].s           = readFloat();
      font->glyphs[i].t           = readFloat();
      font->glyphs[i].s2          = readFloat();
      font->glyphs[i].t2          = readFloat();
      font->glyphs[i].glyph       = readInt();
      Q_strncpyz(font->glyphs[i].shaderName, (const char*)&fdFile[fdOffset], sizeof(font->glyphs[i].shaderName));
      fdOffset += sizeof(font->glyphs[i].shaderName);
    }
    font->glyphScale = readFloat();
    Com_Memcpy(font->name, &fdFile[fdOffset], MAX_QPATH);

    // Com_Memcpy(font, faceData, sizeof(fontInfo_t));
    Q_strncpyz(font->name, name, sizeof(font->name));
    for (int i = GLYPH_START; i <= GLYPH_END; i++) { font->glyphs[i].glyph = RE_RegisterShaderNoMip(font->glyphs[i].shaderName); }
    Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));
    ri.FS_FreeFile(faceData);
    return;
  }

#if defined NO_FREETYPE
  ri.Printf(PRINT_WARNING, "%s: FreeType code not available\n", __func__);
#else
  if (ftLibrary == NULL) {
    ri.Printf(PRINT_WARNING, "%s: FreeType not initialized.\n", __func__);
    return;
  }

  len = ri.FS_ReadFile(fontName, &faceData);
  if (len <= 0) {
    ri.Printf(PRINT_WARNING, "%s: Unable to read font file '%s'\n", __func__, fontName);
    return;
  }

  // allocate on the stack first in case we fail
  FT_Face face;
  if (FT_New_Memory_Face(ftLibrary, faceData, len, 0, &face)) {
    ri.Printf(PRINT_WARNING, "%s: FreeType, unable to allocate new face.\n", __func__);
    return;
  }

  float dpi = 72;
  if (FT_Set_Char_Size(face, pointSize << 6, pointSize << 6, dpi, dpi)) {
    ri.Printf(PRINT_WARNING, "%s: FreeType, unable to set face char size.\n", __func__);
    return;
  }

  //*font = &registeredFonts[registeredFontCount++];

  // make a 256x256 image buffer
  // once it is full, register it, clean it and keep going until all glyphs are rendered
  byte* out = ri.Malloc(256 * 256);
  if (out == NULL) {
    ri.Printf(PRINT_WARNING, "%s: ri.Malloc failure during output image creation.\n", __func__);
    return;
  }
  Com_Memset(out, 0, 256 * 256);

  int maxHeight = 0;

  int xOut, yOut;
  for (int i = GLYPH_START; i <= GLYPH_END; i++) { RE_CreateGlyphInfo(out, &xOut, &yOut, &maxHeight, face, (byte)i, qtrue); }
  glyphInfo_t* glyph;
  xOut            = 0;
  yOut            = 0;
  int i           = GLYPH_START;
  int lastStart   = i;
  int imageNumber = 0;
  while (i <= GLYPH_END + 1) {
    if (i == GLYPH_END + 1) {
      // upload/save current image buffer
      xOut = yOut = -1;
    } else {
      glyph = RE_CreateGlyphInfo(out, &xOut, &yOut, &maxHeight, face, (byte)i, qfalse);
    }

    if (xOut == -1 || yOut == -1) {
      // Ran out of room
      // Need to create an image from the bitmap, set all the handles in the glyphs to this point
      int   scaledSize = 256 * 256;
      int   newSize    = scaledSize * 4;
      byte* imageBuff  = ri.Malloc(newSize);
      int   left       = 0;
      float max        = 0;
      for (int k = 0; k < (scaledSize); k++) {
        if (max < out[k]) { max = out[k]; }
      }

      if (max > 0) { max = 255 / max; }

      for (int k = 0; k < (scaledSize); k++) {
        imageBuff[left++] = 255;
        imageBuff[left++] = 255;
        imageBuff[left++] = 255;

        imageBuff[left++] = ((float)out[k] * max);
      }

      Com_sprintf(name, sizeof(name), FONT_CACHEDIR "%s_%i_%i.tga", fontName, imageNumber++, pointSize);
      if (r_saveFontData->integer) { WriteTGA(name, imageBuff, 256, 256); }

      // Com_sprintf (name, sizeof(name), FONT_CACHEDIR "%s_%i_%i", fontName, imageNumber++, pointSize);
      image_t*  image = R_CreateImage(name, NULL, imageBuff, 256, 256, IMGFLAG_CLAMPTOEDGE);
      qhandle_t h     = RE_RegisterShaderFromImage(name, LIGHTMAP_2D, image, qfalse);
      for (int j = lastStart; j < i; j++) {
        font->glyphs[j].glyph = h;
        Q_strncpyz(font->glyphs[j].shaderName, name, sizeof(font->glyphs[j].shaderName));
      }
      lastStart = i;
      Com_Memset(out, 0, 256 * 256);
      xOut = 0;
      yOut = 0;
      ri.Free(imageBuff);
      if (i == GLYPH_END + 1) i++;
    } else {
      Com_Memcpy(&font->glyphs[i], glyph, sizeof(glyphInfo_t));
      i++;
    }
  }

  // change the scale to be relative to 1 based on 72 dpi ( so dpi of 144 means a scale of .5 )
  float glyphScale = 72.0f / dpi;
  // we also need to adjust the scale based on point size relative to 48 points as the ui scaling is based on a 48 point font
  // glyphScale *= 48.0f / pointSize;

  registeredFont[registeredFontCount].glyphScale = glyphScale;
  font->glyphScale                               = glyphScale;
  Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));

  if (r_saveFontData->integer) { ri.FS_WriteFile(va("fonts/fontImage_%i.dat", pointSize), font, sizeof(fontInfo_t)); }

  ri.Free(out);

  // FIXME: Put this in the correct place
  Com_Memcpy(font->name, fontName, strlen(fontName));

  ri.FS_FreeFile(faceData);
#endif // NO_FREETYPE
}


void R_InitFreeType(void) {
#if !defined NO_FREETYPE
  if (FT_Init_FreeType(&ftLibrary)) { ri.Printf(PRINT_WARNING, "%s: Unable to initialize FreeType.\n", __func__); }
#endif // NO_FREETYPE
  registeredFontCount = 0;
}


void R_DoneFreeType(void) {
#if !defined NO_FREETYPE
  if (ftLibrary) {
    FT_Done_FreeType(ftLibrary);
    ftLibrary = NULL;
  }
#endif // NO_FREETYPE
  registeredFontCount = 0;
}

// Why 26.6 ?
//   FreeType's fixed point float format
// 26.6 refers to the placement of a pixel on a grid with fractional accuracy - in this case the fraction is 2 ^ 64.
// If the 26 refers to pixels (as opposed to inches, centimeters, etc) , this simply means that you can position text to any of 64 possible sub-pixel positions.
// Levels of anti-aliasing has to do with sub-pixel positioning.
// You can position text to a fractional position and then render it as 1 bit per pixel monochrome.
// This will effectively round off your positioning to the nearest whole pixel, but you can still specify the positioning as a fractional pixel.
// If you render with 256 levels of gray, then you have that many levels of anti-aliasing available !
// Suppose that FreeType only allowed you to position text in whole pixel increments, but you rendered the text with 256 levels of anti-aliasing.
// Does that mean that suddenly the text would become monochrome? Of course not.
// The positioning accuracy and the anti-aliasing are completely different aspects of graphics that are not connected.
//
// In creating a graphics rendering system you can
// 1 - Have 256 levels of anti-aliasing and no fractional positioning
// 2 - No anti-aliasing with fractional positioning
// 3 - Both
// 4 - Neither
// The choice is up to you when you design the renderer.
//
// Among other choices, FreeType happens to allow fractional positioning with 256 levels of anti-aliasing.
// The sole relationship between anti-aliasing and sub-pixel positioning is that if you expect to see your fractional positioning,
// then you better have some anti-aliasing going on.
// FreeType uses 256 levels of grayscale anti-aliasing which is as much as virtually all common display systems on PCs allow.
// The 26.6 convention is necessary to implement TrueType hinting correctly (with the bytecode interpreter),
// so using 6 bits of fractional pixel units was a minimum.
// Experience shows that it is also largely sufficient to hint other kind of bezier curves, so it staid in FreeType.
// And the reason why we don't use floats are multiple:
// most embedded systems don't have efficient FPUs, so forget about using floats and doubles on these platforms
// TrueType hinting requires and extensively uses 26.6 fixed point computations,
// using floats/doubles has strictly no advantage here, except slower operations, even on modern PCs
