#include "../../local.h"

//:::::::::::::::::::
// Menu Item IDs
#define MID_PLAY_CANCEL 10
#define MID_PLAY_LOAD 11
#define MID_PLAY_LIST 12
//:::::::::::::::::::
// Limits and Config
#define MAX_MAPS 1024
#define MAX_MAPNAME_SIZE 32
#define NAME_BUFSIZE (MAX_MAPS * MAX_MAPNAME_SIZE)


//:::::::::::::::::::
typedef struct MenuPlay_s {
  MenuFw     menu;
  MenuList   list;
  MenuText   title;
  MenuImage cancel;
  MenuImage load;
  char*      maplist[MAX_MAPS];
} MenuPlay;
//:::::::::::::::::::
static MenuPlay s_mplay;
//:::::::::::::::::::


//:::::::::::::::::::
static void menuPlay_event(void* ptr, int event) {
  if (event != MS_ACTIVATED) { return; }

  switch (((MenuCommon*)ptr)->id) {
    case MID_PLAY_LOAD: {
      menuForceOff();
      id3Cmd_ExecuteText(EXEC_APPEND, va("map %s\n", s_mplay.list.itemnames[s_mplay.list.curvalue]));
      break;
    }
    case MID_PLAY_CANCEL: {
      menuPop();
      break;
    }
  }
}

//:::::::::::::::::::
// menuPlay_cache
//:::::::::::::::::::
static void menuPlay_cache(void) {}


//:::::::::::::::::::
// menuPlay_init
//:::::::::::::::::::
static void menuPlay_init(void) {
  memset(&s_mplay, 0, sizeof(MenuPlay));
  menuPlay_cache();

  float  yMargin                  = 0.009;
  float  xMargin                  = 0.016;
  vec_t* textColor                = (vec_t*)mColor.fg;
  vec_t* focusColor               = (vec_t*)colorRed;
  char*  mapExtension             = ".bsp";

  s_mplay.menu.fullscreen         = true;
  s_mplay.menu.wrapAround         = true;

  s_mplay.title.generic.name      = "Title Label";
  s_mplay.title.string            = "Choose a Map";
  s_mplay.title.generic.type      = MITEM_TEXT;
  s_mplay.title.generic.x         = 0 + xMargin;
  s_mplay.title.generic.y         = 0 + yMargin;
  s_mplay.title.font              = uis.font.actionKey;  // TODO: Probably want   uis.font.labelKey
  s_mplay.title.align             = TEXT_ALIGN_LEFT;
  s_mplay.title.color             = textColor;
  s_mplay.title.generic.flags     = 0;
  s_mplay.title.style             = 0;

  s_mplay.cancel.generic.name     = "Cancel";  // ART_BACK0
  s_mplay.cancel.generic.type     = MITEM_IMAGE;
  s_mplay.cancel.generic.flags    = MFL_CENTER_JUSTIFY | MFL_PULSEIFFOCUS;
  s_mplay.cancel.generic.callback = menuPlay_event;
  s_mplay.cancel.width            = 128;
  s_mplay.cancel.height           = 64;
  s_mplay.cancel.generic.x        = 0.25;
  s_mplay.cancel.generic.y        = 1 - yMargin - ((float)s_mplay.cancel.height / GL_H);  // 480 - 64;
  s_mplay.cancel.generic.id       = MID_PLAY_CANCEL;
  // s_mplay.cancel.focuspic         = ART_BACK1;
  // s_mplay.cancel.errorpic         = ??
  s_mplay.cancel.shader           = 0;
  s_mplay.cancel.focusshader      = 0;
  s_mplay.cancel.focuscolor       = focusColor;  // TODO: What is this value used for?

  s_mplay.load.generic.name       = "Load";  // ART_GO0;
  s_mplay.load.generic.type       = MITEM_IMAGE;
  s_mplay.cancel.generic.flags    = MFL_CENTER_JUSTIFY | MFL_PULSEIFFOCUS;
  s_mplay.load.generic.callback   = menuPlay_event;
  s_mplay.load.width              = 128;
  s_mplay.load.height             = 64;
  s_mplay.load.generic.x          = 1 - 0.25;
  s_mplay.load.generic.y          = 1 - yMargin - ((float)s_mplay.load.height / GL_H);  // 480 - 64;
  s_mplay.load.generic.id         = MID_PLAY_LOAD;
  // s_mplay.load.focuspic           = ART_GO1;
  // s_mplay.load.errorpic         = ??
  s_mplay.load.shader             = uis.icon.cancel;
  s_mplay.load.focusshader        = 0;
  s_mplay.load.focuscolor         = focusColor;  // TODO: What is this value used for?

  s_mplay.load.generic.name       = "MapList";
  s_mplay.list.generic.type       = MITEM_LIST;
  s_mplay.list.generic.flags      = MFL_PULSEIFFOCUS;
  s_mplay.list.generic.callback   = menuPlay_event;
  s_mplay.list.generic.id         = MID_PLAY_LIST;
  s_mplay.list.generic.x          = 128;
  s_mplay.list.generic.y          = 130;
  s_mplay.list.width              = 50;
  s_mplay.list.height             = 14;
  s_mplay.list.itemnames          = (const char**)s_mplay.maplist;
  s_mplay.list.columns            = 1;
  // int          oldvalue;
  // int          curvalue;
  // int          numitems;
  // int          top;
  // int          separation;
  // float        left;      // Left bound of the item (percentage)
  // float        top;       // Top bound of the item (percentage)
  // float        right;     // Right bound of the item (percentage)
  // float        bottom;    // Bottom bound of the item (percentage)
  // MenuFw*      parent;    // Owner of this menu item
  // int          activeId;  // Currently active item (? hovering or opposite ?) (was menuPosition)
  // void (*statusbar)(void* self);
  // void (*ownerdraw)(void* self);

  // id3FS_GetFileList(const char* path, const char* extension, char* listbuf, int bufsize)
  char mapNames[NAME_BUFSIZE];  // Map names buffer. It's not an array of arrays, its an array of characters
  int  mapCount = id3FS_GetFileList("maps", mapExtension, mapNames, ARRAY_LEN(mapNames));

  // Com_Printf("%d", s_mplay.list.curvalue);

  // NOTE: This is assigning a slice of a bigger all-names array as a single mapname.
  char* mapname = mapNames;

  // .........................................
  int map = 0;
  for (int j = 0; j < 2; j++) {  // TODO: Why is this repeating twice ??
    if (mapCount > MAX_MAPS) { mapCount = MAX_MAPS; }

    for (; map < mapCount; map++) {  // TODO: Doesn't this loop just hit the max maps the first time, instead?
      // remove .bsp extension from mapname
      int len = strlen(mapname);
      if (len > strlen(mapExtension)) { mapname[len - strlen(mapExtension)] = '\0'; }
      s_mplay.list.itemnames[map] = mapname;
      mapname += len + 1;  // Move to the next name in the same array of characters (not array of arrays)
    }
  }
  // .........................................

  s_mplay.list.numitems = mapCount;

  if (!mapCount) {
    s_mplay.list.itemnames[0] = "No Maps Found.";
    s_mplay.list.numitems     = 1;
    // degenerate case, not selectable
    s_mplay.load.generic.flags |= MFL_INACTIVE | MFL_HIDDEN;
  }

  menuAddItem(&s_mplay.menu, &s_mplay.title);
  menuAddItem(&s_mplay.menu, &s_mplay.list);
  menuAddItem(&s_mplay.menu, &s_mplay.cancel);
  menuAddItem(&s_mplay.menu, &s_mplay.load);

  menuPush(&s_mplay.menu);
}


//:::::::::::::::::::
// menuPlay
//   Called when starting the menu
//:::::::::::::::::::
void menuPlay(void) { menuPlay_init(); }
// void menuPlay(void) { }
