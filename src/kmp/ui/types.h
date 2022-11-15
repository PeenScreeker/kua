#ifndef UI_TYPES_H
#define UI_TYPES_H
//:::::::::::::::::

#include "shared.h"
#include "../rendc/tr_types.h"
#include "config.h"

//:::::::::::::::::
// Menu Data types
typedef struct menuframework_s {
  int   cursor;
  int   cursor_prev;
  int   nitems;
  void* items[MAX_MENUITEMS];
  bool  wrapAround;
  bool  fullscreen;
  bool  showlogo;
  void (*draw)(void);
  sfxHandle_t (*key)(int key);
} MenuFw;
//:::::::::::::::::
typedef struct menucommon_s {
  int          type;
  const char*  name;
  int          id;
  float        x, y;  // Position of the menu
  int          left;
  int          top;
  int          right;
  int          bottom;
  MenuFw*      parent;
  int          menuPosition;
  unsigned int flags;
  void (*callback)(void* self, int event);
  void (*statusbar)(void* self);
  void (*ownerdraw)(void* self);
} MenuCommon;  // Properties common between items
//:::::::::::::::::
typedef struct mfield_s {
  int  cursor;
  int  scroll;
  int  widthInChars;
  char buffer[MAX_EDIT_LINE];
  int  maxchars;
} Field;
//:::::::::::::::::
typedef struct menufield_s {
  MenuCommon generic;  // Properties common between items
  Field      field;
} MenuField;
//:::::::::::::::::
typedef struct menuslider_s {
  MenuCommon generic;  // Properties common between items
  float      minvalue;
  float      maxvalue;
  float      curvalue;
  float      range;
} MenuSlider;
//:::::::::::::::::
typedef struct menulist_s {
  MenuCommon   generic;  // Properties common between items
  int          oldvalue;
  int          curvalue;
  int          numitems;
  int          top;
  const char** itemnames;
  int          width;
  int          height;
  int          columns;
  int          separation;
} MenuList;
//:::::::::::::::::
typedef struct menuaction_s {
  MenuCommon generic;  // Properties common between items
} MenuAction;
//:::::::::::::::::
typedef struct menuradiobutton_s {
  MenuCommon generic;  // Properties common between items
  int        curvalue;
} MenuRadioBtn;
//:::::::::::::::::
typedef struct menubitmap_s {
  MenuCommon generic;  // Properties common between items
  char*      focuspic;
  char*      errorpic;
  qhandle_t  shader;
  qhandle_t  focusshader;
  int        width;
  int        height;
  float*     focuscolor;
} MenuBitmap;
//:::::::::::::::::
typedef struct {
  MenuCommon generic;  // Properties common between items
  char*      string;   // Text to draw
  int        style;    // UI styling flags
  float*     color;    // Text color
  fontInfo_t font;     // Font data
  int        align;    // Text alignment enum id
} MenuText;
//:::::::::::::::::

//:::::::::::::::::
typedef struct Fonts_s {
  fontInfo_t small;
  fontInfo_t normal;
  fontInfo_t label;
  fontInfo_t action;
  fontInfo_t actionKey;
  fontInfo_t mono;
  fontInfo_t number;
} Fonts;
//:::::::::::::::::
typedef struct {
  int        frametime;
  int        realtime;
  int        cursorx;               // Mouse Cursor horizontal position
  int        cursory;               // Mouse cursor vertical position
  int        menusp;                // Menus counter and id ("pointer")
  MenuFw*    activemenu;            // Active menu data pointer
  MenuFw*    stack[MAX_MENUDEPTH];  // Menu data array
  glconfig_t glconfig;
  bool       debug;
  qhandle_t  whiteShader;
  qhandle_t  menuBackShader;
  qhandle_t  menuBackNoLogoShader;
  qhandle_t  charset;
  qhandle_t  charsetProp;
  qhandle_t  charsetPropGlow;
  qhandle_t  charsetPropB;
  qhandle_t  logoQ3;  // TODO: Remove, and change to Kua logo
  qhandle_t  cursor;
  qhandle_t  rb_on;
  qhandle_t  rb_off;
  float      xscale;
  float      yscale;
  float      bias;
  bool       demoversion;
  bool       firstdraw;
  Fonts      font;
} uiStatic_t;
//:::::::::::::::::
typedef struct q3sound_s {
  sfxHandle_t menu_in;
  sfxHandle_t menu_out;
  sfxHandle_t menu_move;
  sfxHandle_t menu_buzz;
  sfxHandle_t menu_null;
  sfxHandle_t weaponChange;
} Q3sound;
//:::::::::::::::::

//:::::::::::::::::
#endif  // UI_TYPES_H
