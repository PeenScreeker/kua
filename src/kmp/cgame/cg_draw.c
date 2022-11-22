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
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

#ifdef TEAMARENA
#include "../ui_ta/ui_shared.h"

// used for scoreboard
extern displayContextDef_t cgDC;
menuDef_t *menuScoreboard = NULL;
#else
int drawTeamOverlayModificationCount = -1;
#endif

int sortedTeamPlayers[TEAM_MAXOVERLAY];
int	numSortedTeamPlayers;

char systemChat[256];
char teamChat1[256];
char teamChat2[256];


//::KUA.chg
//TODO: Switch to Cvars
qboolean hud_speed = qtrue;
qboolean ui_Vspeed = qtrue;
//::KUA.end


#ifdef TEAMARENA

int CG_Text_Width(const char *text, float scale, int limit) {
  int count,len;
	float out;
	glyphInfo_t *glyph;
	float useScale;
	const char *s = text;
	fontInfo_t *font = &cgDC.Assets.textFont;
	if (scale <= cg_fontSmall.value) {
		font = &cgDC.Assets.fontSmall;
	} else if (scale > cg_fontBig.value) {
		font = &cgDC.Assets.fontBig;
	}
	useScale = scale * font->glyphScale;
  out = 0;
  if (text) {
    len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if ( Q_IsColorString(s) ) {
				s += 2;
				continue;
			} else {
				glyph = &font->glyphs[*s & 255];
				out += glyph->xSkip;
				s++;
				count++;
			}
    }
  }
  return out * useScale;
}

int CG_Text_Height(const char *text, float scale, int limit) {
  int len, count;
	float max;
	glyphInfo_t *glyph;
	float useScale;
	const char *s = text;
	fontInfo_t *font = &cgDC.Assets.textFont;
	if (scale <= cg_fontSmall.value) {
		font = &cgDC.Assets.fontSmall;
	} else if (scale > cg_fontBig.value) {
		font = &cgDC.Assets.fontBig;
	}
	useScale = scale * font->glyphScale;
  max = 0;
  if (text) {
    len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if ( Q_IsColorString(s) ) {
				s += 2;
				continue;
			} else {
				glyph = &font->glyphs[*s & 255];
	      if (max < glyph->height) {
		      max = glyph->height;
			  }
				s++;
				count++;
			}
    }
  }
  return max * useScale;
}

void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader) {
  float w, h;
  w = width * scale;
  h = height * scale;
  CG_AdjustFrom640( &x, &y, &w, &h );
  trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style) {
  int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
	float useScale;
	fontInfo_t *font = &cgDC.Assets.textFont;
	if (scale <= cg_fontSmall.value) {
		font = &cgDC.Assets.fontSmall;
	} else if (scale > cg_fontBig.value) {
		font = &cgDC.Assets.fontBig;
	}
	useScale = scale * font->glyphScale;
  if (text) {
		const char *s = text;
		trap_R_SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
    len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			glyph = &font->glyphs[*s & 255];
      //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
      //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
			if ( Q_IsColorString( s ) ) {
				memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
				newColor[3] = color[3];
				trap_R_SetColor( newColor );
				s += 2;
				continue;
			} else {
				float yadj = useScale * glyph->top;
				if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE) {
					int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
					colorBlack[3] = newColor[3];
					trap_R_SetColor( colorBlack );
					CG_Text_PaintChar(x + ofs, y - yadj + ofs, 
														glyph->imageWidth,
														glyph->imageHeight,
														useScale, 
														glyph->s,
														glyph->t,
														glyph->s2,
														glyph->t2,
														glyph->glyph);
					colorBlack[3] = 1.0;
					trap_R_SetColor( newColor );
				}
				CG_Text_PaintChar(x, y - yadj, 
													glyph->imageWidth,
													glyph->imageHeight,
													useScale, 
													glyph->s,
													glyph->t,
													glyph->s2,
													glyph->t2,
													glyph->glyph);
				// CG_DrawPic(x, y - yadj, scale * cgDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * cgDC.Assets.textFont.glyphs[text[i]].imageHeight, cgDC.Assets.textFont.glyphs[text[i]].glyph);
				x += (glyph->xSkip * useScale) + adjust;
				s++;
				count++;
			}
    }
	  trap_R_SetColor( NULL );
  }
}


#endif

//::::::::::::::::
// TODO: Move out to a header accesible by the rest of the code
#define TEXT_STYLE_SHADOW  3 // Drop shadow
#define TEXT_STYLE_SHADOW2 6 // Drop shadow, twice the size
//.........................................
// CG_TextGetWidth
//   Returns the total width of the given text, when using the selected font
//.........................................
int CG_TextGetWidth(const char *text, fontInfo_t* font, float scale, int limit) {
  float useScale = scale * font->glyphScale;
  float out = 0;
  if (text) {
    int len = strlen(text);
    if (limit > 0 && len > limit) { len = limit; }
    int count = 0;
    const char *s = text;
    while (s && *s && count < len) {
      if ( Q_IsColorString(s) ) {
        s += 2;
        continue;
      } else {
        glyphInfo_t* glyph = &font->glyphs[*s & 255];
        out += glyph->xSkip;
        s++;
        count++;
      }
    }
  }
  return out * useScale;
}
//.........................................
// CG_TextGetHeight
//   Returns the max height of the given text, when using the selected font
//.........................................
int CG_TextGetHeight(const char *text, fontInfo_t* font, float scale, int limit) {
  float useScale = scale * font->glyphScale;
  float max = 0;
  if (text) {
    int len = strlen(text);
    if (limit > 0 && len > limit) {
      len = limit;
    }
    int count = 0;
    const char *s = text;
    while (s && *s && count < len) {
      if ( Q_IsColorString(s) ) {
        s += 2;
        continue;
      } else {
        glyphInfo_t* glyph = &font->glyphs[*s & 255];
        if (max < glyph->height) {
          max = glyph->height;
        }
        s++;
        count++;
      }
    }
  }
  return max * useScale;
}

//.........................................
// CG_DrawTextChar
//   Draws a character in the given shader
//   Coordinates are expected in screen size. No adjusting is done
//.........................................
static void CG_TextDrawChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t glyphShader) {
  float w = width * scale;
  float h = height * scale;
  trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, glyphShader );
}
//.........................................
// CG_DrawTextStr
//   Draws a text string with the given properties
//   Coordinates are expected in screen size. No adjusting is done
//.........................................
void CG_TextDrawStr(const char *text, fontInfo_t *font, float x, float y, float scale, vec4_t color, float adjust, int style, int maxLength) {
  // if (scale <= hud_fontSmall.value)   { font = &cgs.media.fontSmall; } 
  // else if (scale > hud_fontBig.value) { font = &cgs.media.fontBig; }
  float useScale = scale * font->glyphScale;
  if (text) {
    const char *s = text;
    trap_R_SetColor( color );
    vec4_t newColor;
    memcpy(&newColor[0], &color[0], sizeof(vec4_t));
    int len = strlen(text);
    if (maxLength > 0 && len > maxLength) { len = maxLength; }
    int count = 0;
    while (s && *s && count < len) {
      glyphInfo_t* glyph = &font->glyphs[*s & 255];
      //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
      //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
      if ( Q_IsColorString( s ) ) {
        memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
        newColor[3] = color[3];
        trap_R_SetColor( newColor );
        s += 2;
        continue;
      } else {
        float yadj = useScale * glyph->top;
        if (style == TEXT_STYLE_SHADOW || style == TEXT_STYLE_SHADOW2) {
          int ofs = style == TEXT_STYLE_SHADOW2 ? 2 : 1;
          colorBlack[3] = newColor[3];
          trap_R_SetColor( colorBlack );
          CG_TextDrawChar(x + ofs, y - yadj + ofs,
                          glyph->imageWidth, glyph->imageHeight, useScale, 
                          glyph->s, glyph->t, glyph->s2, glyph->t2,
                          glyph->glyph);
          colorBlack[3] = 1.0;
          trap_R_SetColor( newColor );
        }
        CG_TextDrawChar(x, y - yadj,
                        glyph->imageWidth, glyph->imageHeight, useScale, 
                        glyph->s, glyph->t, glyph->s2, glyph->t2,
                        glyph->glyph);
        x += (glyph->xSkip * useScale) + adjust;
        s++;
        count++;
      }
    }
    trap_R_SetColor( NULL );
  }
}
//.........................................
// CG_TextDraw
//   Draws text to screen using the given font properties
//   Expects X, Y in [0-1] percentage range
//.........................................
void CG_TextDraw(const char *text, fontInfo_t *font, float x, float y, float scale, vec4_t color, float adjust, int style, int maxLength, int align) {
  // Convert X-Y from [0-1] to screen resolution
  x *= cgs.glconfig.vidWidth;
  y *= cgs.glconfig.vidHeight;

  int w = CG_TextGetWidth(text, font, scale, maxLength);
  if (!align) { align = TEXT_ALIGN_DEFAULT; }
  switch (align) { 
    case TEXT_ALIGN_LEFT:   x -= 0; break;
    case TEXT_ALIGN_CENTER: x -= (w*0.5); break;
    case TEXT_ALIGN_RIGHT:  x -= w; break;
    default: break;
  }
  CG_TextDrawStr(text, font, x, y, scale, color, adjust, style, maxLength);
}


/*
==============
CG_DrawField

Draws large numbers for status bar and powerups
==============
*/
#ifndef TEAMARENA
static void CG_DrawField(int x, int y, int width, int value) {
	if (width < 1) { return; }

	// draw number string
	if ( width > 5 ) { width = 5; }

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	char num[16];
	Com_sprintf (num, sizeof(num), "%i", value);
	int l = strlen(num);
	if (l > width) { l = width; }
	x += 2 + CHAR_WIDTH*(width - l);

	int		frame;
	char* ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-') { frame = STAT_MINUS; }
		else             { frame = *ptr -'0'; }

		CG_DrawPic(x, y, CHAR_WIDTH, CHAR_HEIGHT, cgs.media.numberShaders[frame]);
		x += CHAR_WIDTH;
		ptr++;
		l--;
	}
}
#endif // TEAMARENA

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles ) {
	if (!cg_draw3dIcons.integer || !cg_drawIcons.integer) { return; }

	// CG_AdjustFrom640( &x, &y, &w, &h );

	refdef_t		refdef;
	memset( &refdef, 0, sizeof( refdef ) );

	refEntity_t		ent;
	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;  // no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene( &ent );
	trap_R_RenderScene( &refdef );
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles ) {
	clientInfo_t* ci = &cgs.clientinfo[ clientNum ];

	if ( cg_draw3dIcons.integer ) {
		clipHandle_t cm = ci->headModel;
		if (!cm) { return; }

		// offset the origin y and z to center the head
		vec3_t mins, maxs;
		trap_R_ModelBounds( cm, mins, maxs );

		vec3_t origin;
		origin[2] = -0.5 * ( mins[2] + maxs[2] );
		origin[1] = 0.5 * ( mins[1] + maxs[1] );

		// calculate distance so the head nearly fills the box
		// assume heads are taller than wide
		float len = 0.7 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		// allow per-model tweaking
		VectorAdd( origin, ci->headOffset, origin );

		CG_Draw3DModel( x, y, w, h, ci->headModel, ci->headSkin, origin, headAngles );
	} else if ( cg_drawIcons.integer ) {
		CG_DrawPic( x, y, w, h, ci->modelIcon );
	}

	// if they are deferred, draw a cross out
	if ( ci->deferred ) {
		CG_DrawPic( x, y, w, h, cgs.media.deferShader );
	}
}

/*
================
CG_DrawFlagModel

Used for both the status bar and the scoreboard
================
*/
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D ) {
	if ( !force2D && cg_draw3dIcons.integer ) {

		vec3_t angles;
		VectorClear( angles );

		qhandle_t cm = cgs.media.redFlagModel;

		// offset the origin y and z to center the flag
		vec3_t mins, maxs;
		trap_R_ModelBounds( cm, mins, maxs );

		vec3_t origin;
		origin[2] = -0.5 * ( mins[2] + maxs[2] );
		origin[1] = 0.5 * ( mins[1] + maxs[1] );

		// calculate distance so the flag nearly fills the box
		// assume flags are taller than wide
		float len = 0.5 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		angles[YAW] = 60 * sin( cg.time / 2000.0 );;

		qhandle_t handle;
		if( team == TEAM_RED ) {
			handle = cgs.media.redFlagModel;
		} else if( team == TEAM_BLUE ) {
			handle = cgs.media.blueFlagModel;
		} else if( team == TEAM_FREE ) {
			handle = cgs.media.neutralFlagModel;
		} else {
			return;
		}
		CG_Draw3DModel( x, y, w, h, handle, 0, origin, angles );
	} else if ( cg_drawIcons.integer ) {
		gitem_t *item;

		if( team == TEAM_RED ) {
			item = BG_FindItemForPowerup( PW_REDFLAG );
		} else if( team == TEAM_BLUE ) {
			item = BG_FindItemForPowerup( PW_BLUEFLAG );
		} else if( team == TEAM_FREE ) {
			item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
		} else {
			return;
		}
		if (item) {
		  CG_DrawPic( x, y, w, h, cg_items[ ITEM_INDEX(item) ].icon );
		}
	}
}

/*
================
CG_DrawStatusBarHead
================
*/
#ifndef TEAMARENA

static void CG_DrawStatusBarHead( float x ) {

	vec3_t angles;
	VectorClear( angles );

	float size, frac;
	if ( cg.damageTime && cg.time - cg.damageTime < DAMAGE_TIME ) {
		frac = (float)(cg.time - cg.damageTime ) / DAMAGE_TIME;
		size = ICON_SIZE * 1.25 * ( 1.5 - frac * 0.5 );

		float stretch = size - ICON_SIZE * 1.25;
		// kick in the direction of damage
		x -= stretch * 0.5 + cg.damageX * stretch * 0.5;

		cg.headStartYaw = 180 + cg.damageX * 45;

		cg.headEndYaw = 180 + 20 * cos( crandom()*M_PI );
		cg.headEndPitch = 5 * cos( crandom()*M_PI );

		cg.headStartTime = cg.time;
		cg.headEndTime = cg.time + 100 + random() * 2000;
	} else {
		if ( cg.time >= cg.headEndTime ) {
			// select a new head angle
			cg.headStartYaw = cg.headEndYaw;
			cg.headStartPitch = cg.headEndPitch;
			cg.headStartTime = cg.headEndTime;
			cg.headEndTime = cg.time + 100 + random() * 2000;

			cg.headEndYaw = 180 + 20 * cos( crandom()*M_PI );
			cg.headEndPitch = 5 * cos( crandom()*M_PI );
		}

		size = ICON_SIZE * 1.25;
	}

	// if the server was frozen for a while we may have a bad head start time
	if (cg.headStartTime > cg.time) { cg.headStartTime = cg.time; }

	frac = ( cg.time - cg.headStartTime ) / (float)( cg.headEndTime - cg.headStartTime );
	frac = frac * frac * ( 3 - 2 * frac );
	angles[YAW] = cg.headStartYaw + ( cg.headEndYaw - cg.headStartYaw ) * frac;
	angles[PITCH] = cg.headStartPitch + ( cg.headEndPitch - cg.headStartPitch ) * frac;

	CG_DrawHead( x, GL_H - size, size, size, 
				cg.snap->ps.clientNum, angles );
}
#endif // TEAMARENA

/*
================
CG_DrawStatusBarFlag
================
*/
#ifndef TEAMARENA
static void CG_DrawStatusBarFlag( float x, int team ) {
	CG_DrawFlagModel( x, GL_H - ICON_SIZE, ICON_SIZE, ICON_SIZE, team, qfalse );
}
#endif // TEAMARENA

/*
================
CG_DrawTeamBackground
================
*/
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team ) {
	vec4_t hcolor;
	hcolor[3] = alpha;
	if ( team == TEAM_RED ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	} else if ( team == TEAM_BLUE ) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	} else {
		return;
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );
}

/*
================
CG_DrawStatusBar
================
*/
#ifndef TEAMARENA
static void CG_DrawStatusBar( void ) {
	if ( cg_drawStatus.integer == 0 ) { return;	}

	int    color;
	vec4_t hcolor;
	static float colors[4][4] = { 
//		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1} };
		{ 1.00f, 1.00f, 1.00f, 1.00f },    // normal
		{ 1.00f, 0.20f, 0.20f, 1.00f },    // low health
		{ 0.50f, 0.50f, 0.50f, 1.00f },    // weapon firing
		{ 0.00f, 0.50f, 1.00f, 1.00f } };  // health > 125

	int xAdj = (int)GL_W*0.3;       //::KUA.chg -> was 185 hardcoded
  int yAdj = (int)GL_H-ICON_SIZE; //::KUA.chg -> was 432 hardcoded

	// draw the team background
	CG_DrawTeamBackground( 0, GL_H-GL_H*0.1, GL_W, 60, 0.33f, cg.snap->ps.persistant[PERS_TEAM] );  //::KUA.chg -> was 420 and 640

	centity_t* cent = &cg_entities[cg.snap->ps.clientNum];
	playerState_t* ps = &cg.snap->ps;

	vec3_t angles;
	VectorClear( angles );

	vec3_t origin;
	VectorClear( origin );
	// draw any 3D icons first, so the changes back to 2D are minimized
	if ( cent->currentState.weapon && cg_weapons[ cent->currentState.weapon ].ammoModel ) {
		origin[0] = 70;
		angles[YAW] = 90 + 20 * sin( cg.time / 1000.0 );
		CG_Draw3DModel( CHAR_WIDTH*3 + TEXT_ICON_SPACE, yAdj, ICON_SIZE, ICON_SIZE,
					   cg_weapons[ cent->currentState.weapon ].ammoModel, 0, origin, angles );
	}

	CG_DrawStatusBarHead( xAdj + CHAR_WIDTH*3 + TEXT_ICON_SPACE );

	if( cg.predictedPlayerState.powerups[PW_REDFLAG] ) {
		CG_DrawStatusBarFlag( xAdj + CHAR_WIDTH*3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_RED );
	} else if( cg.predictedPlayerState.powerups[PW_BLUEFLAG] ) {
		CG_DrawStatusBarFlag( xAdj + CHAR_WIDTH*3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_BLUE );
	} else if( cg.predictedPlayerState.powerups[PW_NEUTRALFLAG] ) {
		CG_DrawStatusBarFlag( xAdj + CHAR_WIDTH*3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_FREE );
	}

	if ( ps->stats[ STAT_ARMOR ] ) {
		origin[0] = 90;
		origin[1] = 0;
		origin[2] = -10;
		angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;
		CG_Draw3DModel( xAdj*2 + CHAR_WIDTH*3 + TEXT_ICON_SPACE, yAdj, ICON_SIZE, ICON_SIZE,
					   cgs.media.armorModel, 0, origin, angles ); //::KUA.chg -> was 370, 185 hardcoded
	}
	//
	// ammo
	int value;
	if ( cent->currentState.weapon ) {
		value = ps->ammo[cent->currentState.weapon];
		if ( value > -1 ) {
			if ( cg.predictedPlayerState.weaponstate == WEAPON_FIRING
				&& cg.predictedPlayerState.weaponTime > 100 ) {
				// draw as dark grey when reloading
				color = 2;	// dark grey
			} else {
				if (value >= 0) { color = 0; }  // green
				else { color = 1; }  // red
			}
			trap_R_SetColor( colors[color] );
			
			CG_DrawField (0, yAdj, 3, value);
			trap_R_SetColor( NULL );

			// if we didn't draw a 3D icon, draw a 2D icon for ammo
			if ( !cg_draw3dIcons.integer && cg_drawIcons.integer ) {
				qhandle_t	icon;

				icon = cg_weapons[ cg.predictedPlayerState.weapon ].ammoIcon;
				if ( icon ) {
					CG_DrawPic( CHAR_WIDTH*3 + TEXT_ICON_SPACE, yAdj, ICON_SIZE, ICON_SIZE, icon );
				}
			}
		}
	}

	//
	// health
	//
	value = ps->stats[STAT_HEALTH];
	if ( value > 125 ) {
		trap_R_SetColor( colors[3] ); // High
	} else if (value > 25) {
		trap_R_SetColor( colors[0] ); // Normal
	} else if (value > 0) {
		color = (cg.time >> 8) & 1;   // flash
		trap_R_SetColor( colors[color] );
	} else {
		trap_R_SetColor( colors[1] ); // red
	}

	// stretch the health up when taking damage
	CG_DrawField ( xAdj, yAdj, 3, value);
	CG_ColorForHealth( hcolor );
	trap_R_SetColor( hcolor );


	//
	// armor
	//
	value = ps->stats[STAT_ARMOR];
	if (value > 0 ) {
		trap_R_SetColor( colors[0] );
		CG_DrawField (xAdj*2, yAdj, 3, value);
		trap_R_SetColor( NULL );
		// if we didn't draw a 3D icon, draw a 2D icon for armor
		if ( !cg_draw3dIcons.integer && cg_drawIcons.integer ) {
			CG_DrawPic( xAdj*2 + CHAR_WIDTH*3 + TEXT_ICON_SPACE, yAdj, ICON_SIZE, ICON_SIZE, cgs.media.armorIcon );
		}

	}
}
#endif

/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

/*
================
CG_DrawAttacker

================
*/
static float CG_DrawAttacker(float y) {

  // Return input value cases
	if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) { return y; }
	if (!cg.attackerTime) { return y; }
	int clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if (clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum ) { return y; }
	if (!cgs.clientinfo[clientNum].infoValid) {
		cg.attackerTime = 0;
		return y;
	}

	int t = cg.time - cg.attackerTime;
	if ( t > ATTACKER_HEAD_TIME ) {
		cg.attackerTime = 0;
		return y;
	}

	float size = ICON_SIZE * 1.25;

	vec3_t angles;
  VectorClear(angles);
	angles[YAW] = 180;
	CG_DrawHead( GL_W - size, y, size, size, clientNum, angles );

	const char* info = CG_ConfigString(CS_PLAYERS + clientNum);
	const char* name = Info_ValueForKey(info, "n");
	y += size;
	CG_DrawBigString(GL_W - (Q_PrintStrlen(name) * BIGCHAR_WIDTH), y, name, 0.5);

	return y + BIGCHAR_HEIGHT + 2;
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot( float y ) {
	char* s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime, cg.latestSnapshotNum, cgs.serverCommandSequence );
	int w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	CG_DrawBigString( 635 - w, y + 2, s, 1.0F);
	return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float CG_DrawFPS(float y) {
	// Don't use serverTime, It will drift to correct for lag, timescales, timedemos, etc
	static	int	previous;
	int t = trap_Milliseconds();
	int frameTime = t - previous;
	previous = t;

	static int index;
	static int previousTimes[FPS_FRAMES];
	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if (index > FPS_FRAMES) {
		// average multiple frames together to smooth changes out a bit
		int total = 0;
		for (int i = 0 ; i < FPS_FRAMES ; i++ ) { total += previousTimes[i]; }
		if (!total) { total = 1; }
		int fps = 1000 * FPS_FRAMES / total;

		char* s = va( "%ifps", fps );
		int w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

		// CG_DrawBigString( 635 - w, y + 2, s, 1.0F);
		CG_TextDraw(s, &cgs.media.font, 1, 0.02, 1, colorWhite, 0, 0, strlen(s), TEXT_ALIGN_RIGHT);
	}

	return y + BIGCHAR_HEIGHT + 4;
}

/*
=================
CG_DrawTimelimit
//::KUA.chg name from timer to timelimit.
=================
*/
static float CG_DrawTimelimit(float y) {
	int msec     = cg.time - cgs.levelStartTime;
	int seconds  = msec / 1000;
	int mins     = seconds / 60;
	seconds     -= mins * 60;
	int tens     = seconds / 10;
	seconds     -= tens * 10;

	char* s = va( "%i:%i%i", mins, tens, seconds );
	// int w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	// CG_DrawBigString( 635 - w, y + 2, s, 1.0F);
	// return y + BIGCHAR_HEIGHT + 4;
	CG_TextDraw(s, &cgs.media.font, 1, 0.04, 1, colorWhite, 0, 3, strlen(s), TEXT_ALIGN_RIGHT);
  return y + cgs.media.font.glyphs->height + 4;
}


/*
=================
CG_DrawTeamOverlay
=================
*/

static float CG_DrawTeamOverlay( float y, qboolean right, qboolean upper ) {
	int x, w, h, xx;
	int i, j, len;
	const char *p;
	vec4_t		hcolor;
	int pwidth, lwidth;
	int plyrs;
	char st[16];
	clientInfo_t *ci;
	gitem_t	*item;
	int ret_y, count;

	if ( !cg_drawTeamOverlay.integer ) {
		return y;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE ) {
		return y; // Not on any team
	}

	plyrs = 0;

	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
			plyrs++;
			len = CG_DrawStrlen(ci->name);
			if (len > pwidth)
				pwidth = len;
		}
	}

	if (!plyrs)
		return y;

	if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;

	// max location name width
	lwidth = 0;
	for (i = 1; i < MAX_LOCATIONS; i++) {
		p = CG_ConfigString(CS_LOCATIONS + i);
		if (p && *p) {
			len = CG_DrawStrlen(p);
			if (len > lwidth)
				lwidth = len;
		}
	}

	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;

	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;

	x = (right) ? GL_W - w : 0;

	h = plyrs * TINYCHAR_HEIGHT;

	if ( upper ) {
		ret_y = y + h;
	} else {
		y -= h;
		ret_y = y;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
		hcolor[0] = 1.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 0.0f;
		hcolor[3] = 0.33f;
	} else { // if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
		hcolor[0] = 0.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 1.0f;
		hcolor[3] = 0.33f;
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );

	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {

			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

			xx = x + TINYCHAR_WIDTH;

			CG_DrawStringExt( xx, y,
				ci->name, hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);

			if (lwidth) {
				p = CG_ConfigString(CS_LOCATIONS + ci->location);
				if (!p || !*p)
					p = "unknown";
//				len = CG_DrawStrlen(p);
//				if (len > lwidth)
//					len = lwidth;

//				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth + 
//					((lwidth/2 - len/2) * TINYCHAR_WIDTH);
				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
				CG_DrawStringExt( xx, y,
					p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
					TEAM_OVERLAY_MAXLOCATION_WIDTH);
			}

			CG_GetColorForHealth( ci->health, ci->armor, hcolor );

			Com_sprintf (st, sizeof(st), "%3i %3i", ci->health,	ci->armor);

			xx = x + TINYCHAR_WIDTH * 3 + 
				TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

			CG_DrawStringExt( xx, y,
				st, hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );

			// draw weapon icon
			xx += TINYCHAR_WIDTH * 3;

			if ( cg_weapons[ci->curWeapon].weaponIcon ) {
				CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, cg_weapons[ci->curWeapon].weaponIcon );
			} else {
				CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, cgs.media.deferShader );
			}

			// Draw powerup icons
			xx = (right) ? x : x + w - TINYCHAR_WIDTH;
			for (j = 0; j <= PW_NUM_POWERUPS; j++) {
				if (ci->powerups & (1 << j)) {

					item = BG_FindItemForPowerup( j );

					if (item) {
						CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, trap_R_RegisterShader( item->icon ) );
						if (right) {
							xx -= TINYCHAR_WIDTH;
						} else {
							xx += TINYCHAR_WIDTH;
						}
					}
				}
			}

			y += TINYCHAR_HEIGHT;
		}
	}

	return ret_y;
//#endif
}


/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight(stereoFrame_t stereoFrame) {
	float y = 0;
	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1 ) {
		y = CG_DrawTeamOverlay( y, qtrue, qtrue );
	} 
	if ( cg_drawSnapshot.integer ) {
		y = CG_DrawSnapshot( y );
	}
	if (cg_drawFPS.integer && (stereoFrame == STEREO_CENTER || stereoFrame == STEREO_RIGHT)) {
		y = CG_DrawFPS( y );
	}
	if ( cg_drawTimelimit.integer ) {
		y = CG_DrawTimelimit( y );  //::KUA.chg name from timer to timelimit.
	}
	if ( cg_drawAttacker.integer ) {
		CG_DrawAttacker( y );
	}

}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/

/*
=================
CG_DrawScores
Draw the small two score display
=================
*/
#ifndef TEAMARENA
static float CG_DrawScores_old(float y) {
	const char* s;
	int         x, w;
	vec4_t      color;
	gitem_t*    item;

	int s1 = cgs.scores1;
	int s2 = cgs.scores2;

	y -= BIGCHAR_HEIGHT + 8;

	float y1 = y;

	// draw from the right side to left
	if ( cgs.gametype >= GT_TEAM ) {
		x = GL_W;
		color[0] = 0.0f;
		color[1] = 0.0f;
		color[2] = 1.0f;
		color[3] = 0.33f;
		s = va( "%2i", s2 );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
			CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
		}
		CG_DrawBigString( x + 4, y, s, 1.0F);

		if ( cgs.gametype == GT_CTF ) {
			// Display flag status
			item = BG_FindItemForPowerup( PW_BLUEFLAG );

			if (item) {
				y1 = y - BIGCHAR_HEIGHT - 8;
				if( cgs.blueflag >= 0 && cgs.blueflag <= 2 ) {
					CG_DrawPic( x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.blueFlagShader[cgs.blueflag] );
				}
			}
		}
		color[0] = 1.0f;
		color[1] = 0.0f;
		color[2] = 0.0f;
		color[3] = 0.33f;
		s = va( "%2i", s1 );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
			CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
		}
		CG_DrawBigString( x + 4, y, s, 1.0F);

		if (cgs.gametype == GT_CTF) {
			// Display flag status
			item = BG_FindItemForPowerup( PW_REDFLAG );

			if (item) {
				y1 = y - BIGCHAR_HEIGHT - 8;
				if( cgs.redflag >= 0 && cgs.redflag <= 2 ) {
					CG_DrawPic( x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.redFlagShader[cgs.redflag] );
				}
			}
		}

		int v = (cgs.gametype >= GT_CTF) ? cgs.capturelimit : cgs.fraglimit;
		if (v) {
			s = va( "%2i", v );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

	} else {
		x = GL_W;
		int score = cg.snap->ps.persistant[PERS_SCORE];
		qboolean spectator = ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR );

		// always show your score in the second box if not in first place
		if (s1 != score) { s2 = score; }
		if (s2 != SCORE_NOT_PRESENT) {
			s = va( "%2i", s2 );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			if (!spectator && score == s2 && score != s1) {
				color[0] = 1.0f;
				color[1] = 0.0f;
				color[2] = 0.0f;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
				CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
			} else {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
			}	
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

		// first place
		if (s1 != SCORE_NOT_PRESENT) {
			s = va( "%2i", s1 );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
      x -= w;
			if ( !spectator && score == s1 ) {
				color[0] = 0.0f;
				color[1] = 0.0f;
				color[2] = 1.0f;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
				CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
			} else {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
			}	
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

		if (cgs.fraglimit >= 0) {
			s = va( "%2i", cgs.fraglimit );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
      x -= w;
			CG_DrawBigString( x - 4, y, s, 1.0F);
		}
	}
	return y1 - 8;
}

//::KUA.new -> Simpler version for kua gamemodes, without the unnecessary q3a modes noise
static void CG_DrawScore(float x, float y, int align) {
  //::KUA.tmp -> Skip if not gamemode 0. TODO: Implement for other modes
	if (!(cgs.gametype >= GT_RUN) ) { return; }
	int score     = cg.snap->ps.persistant[PERS_SCORE];
  if (!score) { return; }
  int fraglimit = cgs.fraglimit;
  const char* title = "Score:";
	const char* s = (fraglimit) ? va("%s %2i | %2i", title, score, fraglimit) : va("%s %2i", title, score);
	CG_TextDraw(s, &cgs.media.font, x, y, FONT_SCALE_DEFAULT, colorWhite, 0, 3, strlen(s), align);
}
#endif // TEAMARENA

/*
================
CG_DrawPowerups
================
*/
#ifndef TEAMARENA
static float CG_DrawPowerups( float x, float y ) {
	playerState_t* ps = &cg.snap->ps;
	if (ps->stats[STAT_HEALTH] <= 0) { return y; }

	int		sorted[MAX_POWERUPS];
	int		sortedTime[MAX_POWERUPS];
	gitem_t	*item;
	float	size;
	float	f;
	int		color;
	static float colors[2][4] = {
    { 0.2f, 1.0f, 0.2f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f }
  };
	// Start point: Draw icon centered, and numbers to the left if > 99
  x *= GL_W;
  y *= GL_H;
  int xPos = x + ICON_SIZE*0.5;

	// sort the list by time remaining
	int active = 0;
	for (int i = 0 ; i < MAX_POWERUPS ; i++) {
		if (!ps->powerups[i]) { continue; }

		// ZOID--don't draw if the power up has unlimited time. True for CTF flags
		if (ps->powerups[ i ] == INT_MAX) { continue; }

		int t = ps->powerups[i] - cg.time;
		if (t <= 0) { continue; }

		// insert into the list
		int j;
		for (j = 0 ; j < active ; j++ ) {
			if (sortedTime[j] >= t) {
				for (int k = active - 1 ; k >= j ; k--) {
					sorted[k+1] = sorted[k];
					sortedTime[k+1] = sortedTime[k];
				}
				break;
			}
		}
		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}

	// draw the icons and timers
	x = xPos;// + ICON_SIZE*0.5; // - CHAR_WIDTH * 2;
	for ( int i = 0 ; i < active ; i++ ) {
		item = BG_FindItemForPowerup( sorted[i] );

    if (item) {
			color = 1;
			y -= ICON_SIZE;

		  trap_R_SetColor( colors[color] );
      int drawTime = sortedTime[i]/1000;
			if (drawTime < 99) { CG_DrawField( x, y, 2, drawTime); }  // Skip drawing if > 99

			int t = ps->powerups[ sorted[i] ];
			if (t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME) {
			  trap_R_SetColor( NULL );
		  } else {
			  f = (float)( t - cg.time ) / POWERUP_BLINK_TIME;
			  f -= (int)f;

				vec4_t modulate;
				modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;
				trap_R_SetColor( modulate );
		  }

		  if ( cg.powerupActive == sorted[i] && 
			  cg.time - cg.powerupTime < PULSE_TIME ) {
			  f = 1.0 - ( ( (float)cg.time - cg.powerupTime ) / PULSE_TIME );
			  size = ICON_SIZE * ( 1.0 + ( PULSE_SCALE - 1.0 ) * f );
		  } else {
			  size = ICON_SIZE;
		  }

		  CG_DrawPic( xPos - size, y + (int)(ICON_SIZE*0.5) - size / 2, 
			  size, size, trap_R_RegisterShader( item->icon ) );
    }
	}
	trap_R_SetColor( NULL );

	return y;
}
#endif // TEAMARENA

/*
=====================
CG_DrawLowerRight
=====================
*/
#ifndef TEAMARENA
static void CG_DrawLowerRight( void ) {
	float y = GL_H*0.97;

	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 2 ) {
		y = CG_DrawTeamOverlay( y, qtrue, qfalse );
	} 

	// y = CG_DrawScores_old( y );
	// CG_DrawPowerups( y );
}
#endif // TEAMARENA

/*
===================
CG_DrawPickupItem
===================
*/
#ifndef TEAMARENA
static int CG_DrawPickupItem( float x, float y ) {
	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) { return y; }

  float xIcon = (GL_W*x) - ICON_SIZE*0.5;
  float yIcon = (GL_H*y) - ICON_SIZE*0.5;
  float xText = x;
  float yRelIcon = (float)ICON_SIZE/GL_H; // Icon height in %, proportional to the screen height
  float yText = y + yRelIcon;

	int value = cg.itemPickup;
	if (value) {
		float* fadeColor = CG_FadeColor( cg.itemPickupTime, 3000 );
		if (fadeColor) {
			CG_RegisterItemVisuals( value );
			trap_R_SetColor( fadeColor );
			CG_DrawPic( xIcon, yIcon, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
			char* s = bg_itemlist[ value ].pickup_name;
			// CG_DrawBigString( ICON_SIZE + 16, y + (ICON_SIZE/2 - BIGCHAR_HEIGHT/2), s, fadeColor[0] );
			CG_TextDraw(s, &cgs.media.font, xText, yText, FONT_SCALE_DEFAULT, colorWhite, 0, 3, strlen(s), TEXT_ALIGN_CENTER);
			trap_R_SetColor( NULL );
		}
	}
	return y;
}
#endif // TEAMARENA

/*
=====================
CG_DrawLowerLeft

=====================
*/
#ifndef TEAMARENA
static void CG_DrawLowerLeft( void ) {
	float y = GL_H - ICON_SIZE;
	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 3 ) {
		y = CG_DrawTeamOverlay( y, qfalse, qfalse );
	} 
	// CG_DrawPickupItem( GL_H*0.5 - ICON_SIZE); //::KUA.chg -> Moved to center
}
#endif // TEAMARENA


//===========================================================================================

/*
=================
CG_DrawTeamInfo
=================
*/
#ifndef TEAMARENA
static void CG_DrawTeamInfo( void ) {
	int h;
	int i;
	vec4_t		hcolor;
	int		chatHeight;

#define CHATLOC_Y 420 // bottom end
#define CHATLOC_X 0

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
		chatHeight = cg_teamChatHeight.integer;
	else
		chatHeight = TEAMCHAT_HEIGHT;
	if (chatHeight <= 0)
		return; // disabled

	if (cgs.teamLastChatPos != cgs.teamChatPos) {
		if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer) {
			cgs.teamLastChatPos++;
		}

		h = (cgs.teamChatPos - cgs.teamLastChatPos) * TINYCHAR_HEIGHT;

		if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED ) {
			hcolor[0] = 1.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		} else if ( cgs.clientinfo[cg.clientNum].team == TEAM_BLUE ) {
			hcolor[0] = 0.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 1.0f;
			hcolor[3] = 0.33f;
		} else {
			hcolor[0] = 0.0f;
			hcolor[1] = 1.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		}

		trap_R_SetColor( hcolor );
		CG_DrawPic( CHATLOC_X, CHATLOC_Y - h, GL_W, h, cgs.media.teamStatusBar );
		trap_R_SetColor( NULL );

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--) {
			CG_DrawStringExt( CHATLOC_X + TINYCHAR_WIDTH, 
				CHATLOC_Y - (cgs.teamChatPos - i)*TINYCHAR_HEIGHT, 
				cgs.teamChatMsgs[i % chatHeight], hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
		}
	}
}
#endif // TEAMARENA

/*
===================
CG_DrawHoldableItem
===================
*/
#ifndef TEAMARENA
static void CG_DrawHoldableItem( void ) { 
	int		value;

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if ( value ) {
		CG_RegisterItemVisuals( value );
		CG_DrawPic( GL_W-ICON_SIZE, (GL_H-ICON_SIZE)/2, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
	}

}
#endif // TEAMARENA

#ifdef TEAMARENA
/*
===================
CG_DrawPersistantPowerup
===================
*/
#if 0 // sos001208 - DEAD
static void CG_DrawPersistantPowerup( void ) { 
	int		value;

	value = cg.snap->ps.stats[STAT_PERSISTANT_POWERUP];
	if ( value ) {
		CG_RegisterItemVisuals( value );
		CG_DrawPic( GL_W-ICON_SIZE, (SCREEN_HEIGHT-ICON_SIZE)/2 - ICON_SIZE, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
	}
}
#endif
#endif // TEAMARENA


/*
===================
CG_DrawReward
===================
*/
static void CG_DrawReward( void ) { 
	if ( !cg_drawRewards.integer ) { return; }
	int		i, count;
	float	x, y;
	char	buf[32];

	float	*color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
	if (!color) {
		if (cg.rewardStack > 0) {
			for(i = 0; i < cg.rewardStack; i++) {
				cg.rewardSound[i] = cg.rewardSound[i+1];
				cg.rewardShader[i] = cg.rewardShader[i+1];
				cg.rewardCount[i] = cg.rewardCount[i+1];
			}
			cg.rewardTime = cg.time;
			cg.rewardStack--;
			color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
			trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
		} else {
			return;
		}
	}

	trap_R_SetColor( color );

	/*
	count = cg.rewardCount[0]/10;				// number of big rewards to draw

	if (count) {
		y = 4;
		x = 320 - count * ICON_SIZE;
		for ( i = 0 ; i < count ; i++ ) {
			CG_DrawPic( x, y, (ICON_SIZE*2)-4, (ICON_SIZE*2)-4, cg.rewardShader[0] );
			x += (ICON_SIZE*2);
		}
	}

	count = cg.rewardCount[0] - count*10;		// number of small rewards to draw
	*/

	if ( cg.rewardCount[0] >= 10 ) {
		y = 56;
		x = 320 - ICON_SIZE/2;
		CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
		Com_sprintf(buf, sizeof(buf), "%d", cg.rewardCount[0]);
		x = ( SCREEN_WIDTH - SMALLCHAR_WIDTH * CG_DrawStrlen( buf ) ) / 2;
		CG_DrawStringExt( x, y+ICON_SIZE, buf, color, qfalse, qtrue,
								SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
	}
	else {

		count = cg.rewardCount[0];

		y = 56;
		x = 320 - count * ICON_SIZE/2;
		for ( i = 0 ; i < count ; i++ ) {
			CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
			x += ICON_SIZE;
		}
	}
	trap_R_SetColor( NULL );
}


/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	LAG_SAMPLES		128


typedef struct {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
} lagometer_t;

lagometer_t		lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) {
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( void ) {
	float		x, y;
	int			cmdNum;
	usercmd_t	cmd;
	const char		*s;
	int			w;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &cmd );
	if ( cmd.serverTime <= cg.snap->ps.commandTime
		|| cmd.serverTime > cg.time ) {	// special check for map_restart
		return;
	}

	// also add text in center of screen
	s = "Connection Interrupted";
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawBigString( 320 - w/2, 100, s, 1.0F);

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

#ifdef TEAMARENA
	x = GL_W - 48;
	y = GL_H - 144;
#else
	x = GL_W - 48;
	y = GL_H - 48;
#endif

	CG_DrawPic( x, y, 48, 48, trap_R_RegisterShader("gfx/2d/net.tga" ) );
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer( void ) {
	int		a, x, y, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	if ( !cg_lagometer.integer || cgs.localServer ) {
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
#ifdef TEAMARENA
	x = GL_W - 48;
	y = GL_H - 144;
#else
	x = GL_W - 48;
	y = GL_H - 48;
#endif

	trap_R_SetColor( NULL );
	CG_DrawPic( x, y, 48, 48, cgs.media.lagometerShader );

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
			}
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;		// RED for dropped snapshots
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	trap_R_SetColor( NULL );

	if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
		CG_DrawBigString( x, y, "snc", 1.0 );
	}

	CG_DrawDisconnect();
}



/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint( const char *str, int y, int charWidth ) {
	char	*s;

	Q_strncpyz( cg.centerPrint, str, sizeof(cg.centerPrint) );

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while( *s ) {
		if (*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString( void ) {
	char	*start;
	int		l;
	int		x, y, w;
#ifdef TEAMARENA
	int h;
#endif
	float	*color;

	if ( !cg.centerPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );
	if ( !color ) {
		return;
	}

	trap_R_SetColor( color );

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while ( 1 ) {
		char linebuffer[1024];

		for ( l = 0; l < 50; l++ ) {
			if ( !start[l] || start[l] == '\n' ) { break; }
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

#ifdef TEAMARENA
		w = CG_Text_Width(linebuffer, 0.5, 0);
		h = CG_Text_Height(linebuffer, 0.5, 0);
		x = (GL_W - w) / 2;
		CG_Text_Paint(x, y + h, 0.5, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
		y += h + 6;
#else
		w = cg.centerPrintCharWidth * CG_DrawStrlen( linebuffer );

		x = ( GL_W - w ) / 2;

		CG_DrawStringExt( x, y, linebuffer, color, qfalse, qtrue,
			cg.centerPrintCharWidth, (int)(cg.centerPrintCharWidth * 1.5), 0 );

		y += cg.centerPrintCharWidth * 1.5;
#endif
		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if (!*start) { break; }
		start++;
	}

	trap_R_SetColor( NULL );
}



/*
================================================================================

CROSSHAIR

================================================================================
*/


/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair(void)
{
	float		w, h;
	qhandle_t	hShader;
	float		f;
	float		x, y;
	int			ca;

	if (!cg_drawCrosshair.integer) { return; }
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) { return; }
	if (cg.renderingThirdPerson) { return; }

	// set color based on health
	if ( cg_crosshairHealth.integer ) {
		vec4_t		hcolor;

		CG_ColorForHealth( hcolor );
		trap_R_SetColor( hcolor );
	} else {
		trap_R_SetColor( NULL );
	}

	w = h = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if ( f > 0 && f < ITEM_BLOB_TIME ) {
		f /= ITEM_BLOB_TIME;
		w *= ( 1 + f );
		h *= ( 1 + f );
	}

	x = GL_W * cg_crosshairX.value+0.5;
	y = GL_H * cg_crosshairY.value+0.5;
	// CG_AdjustFrom640( &x, &y, &w, &h );

	ca = cg_drawCrosshair.integer;
	if (ca < 0) { ca = 0; }
	hShader = cgs.media.crosshairShader[ ca % NUM_CROSSHAIRS ];

	trap_R_DrawStretchPic( x + cg.refdef.x + 0.5 * (cg.refdef.width - w), 
		y + cg.refdef.y + 0.5 * (cg.refdef.height - h), 
		w, h, 0, 0, 1, 1, hShader );

	trap_R_SetColor( NULL );
}

/*
=================
CG_DrawCrosshair3D
=================
*/
static void CG_DrawCrosshair3D(void) {
  // Don't draw cases
	if (!cg_drawCrosshair.integer) { return; }
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) { return; }
	if (cg.renderingThirdPerson) { return; }

	float w = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	float f = cg.time - cg.itemPickupBlendTime;
	if ( f > 0 && f < ITEM_BLOB_TIME ) {
		f /= ITEM_BLOB_TIME;
		w *= ( 1 + f );
	}

	int ca = cg_drawCrosshair.integer;
	if (ca < 0) { ca = 0; }
	qhandle_t hShader = cgs.media.crosshairShader[ ca % NUM_CROSSHAIRS ];

	// Use a different method rendering the crosshair so players don't see two of them when
	// focusing their eyes at distant objects with high stereo separation
	// We are going to trace to the next shootable object and place the crosshair in front of it.

	// first get all the important renderer information
	char rendererinfos[128];
	trap_Cvar_VariableStringBuffer("r_zProj", rendererinfos, sizeof(rendererinfos));
	float zProj = atof(rendererinfos);
	trap_Cvar_VariableStringBuffer("r_stereoSeparation", rendererinfos, sizeof(rendererinfos));
	float stereoSep = zProj / atof(rendererinfos);
	
	float xmax = zProj * tan(cg.refdef.fov_x * M_PI / 360.0f);
	
	// let the trace run through until a change in stereo separation of the crosshair becomes less than one pixel.
	trace_t trace;
	vec3_t  endpos;
	float maxdist = cgs.glconfig.vidWidth * stereoSep * zProj / (2 * xmax);
	VectorMA(cg.refdef.vieworg, maxdist, cg.refdef.viewaxis[0], endpos);
	CG_Trace(&trace, cg.refdef.vieworg, NULL, NULL, endpos, 0, MASK_SHOT);
	
	refEntity_t ent;
	memset(&ent, 0, sizeof(ent));
	ent.reType = RT_SPRITE;
	ent.renderfx = RF_DEPTHHACK | RF_CROSSHAIR;
	
	VectorCopy(trace.endpos, ent.origin);
	
	// scale the crosshair so it appears the same size for all distances
	ent.radius = w / GL_W * xmax * trace.fraction * maxdist / zProj;
	ent.customShader = hShader;

	trap_R_AddRefEntityToScene(&ent);
}



/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity(void) {
	vec3_t start, end;
	VectorCopy(cg.refdef.vieworg, start);
	VectorMA(start, 131072, cg.refdef.viewaxis[0], end);

	trace_t trace;
	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY );
	if ( trace.entityNum >= MAX_CLIENTS ) { return; }

	// if the player is in fog, don't show it
	int content = CG_PointContents( trace.endpos, 0 );
	if (content & CONTENTS_FOG) { return; }
	// if the player is invisible, don't show it
	if (cg_entities[ trace.entityNum ].currentState.powerups & ( 1 << PW_INVIS )) { return; }

	// update the fade timer
	cg.crosshairClientNum  = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}


/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames( void ) {
  // Don't draw cases
	if (!cg_drawCrosshair.integer ) { return; }
	if (!cg_drawCrosshairNames.integer ) { return; }
	if (cg.renderingThirdPerson ) { return; }

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	float* color = CG_FadeColor( cg.crosshairClientTime, 1000 );
	if ( !color ) {
		trap_R_SetColor( NULL );
		return;
	}

	char* name = cgs.clientinfo[ cg.crosshairClientNum ].name;
#ifdef TEAMARENA
	color[3] *= 0.5f;
	float w = CG_Text_Width(name, 0.3f, 0);
	CG_Text_Paint( 320 - w / 2, 190, 0.3f, color, name, 0, 0, ITEM_TEXTSTYLE_SHADOWED);
#else
	float w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;
	CG_DrawBigString( 320 - w / 2, 170, name, color[3] * 0.5f );
#endif
	trap_R_SetColor( NULL );
}


//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void) {
	CG_DrawBigString(320 - 9 * 8, 440, "SPECTATOR", 1.0F);
	if (cgs.gametype == GT_TOURNAMENT) {
		CG_DrawBigString(320 - 15 * 8, 460, "waiting to play", 1.0F);
	}
	else if (cgs.gametype >= GT_TEAM) {
		CG_DrawBigString(320 - 39 * 8, 460, "press ESC and use the JOIN menu to play", 1.0F);
	}
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void) {
	if (!cgs.voteTime) { return; }

	// play a talk beep whenever it is modified
	if (cgs.voteModified) {
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	int sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;
	if (sec < 0) { sec = 0; }
#ifdef TEAMARENA
	char* s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
	CG_DrawSmallString( 0, 58, s, 1.0F );
	s = "or press ESC then click Vote";
	CG_DrawSmallString( 0, 58 + SMALLCHAR_HEIGHT + 2, s, 1.0F );
#else
	char* s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo );
	CG_DrawSmallString( 0, 58, s, 1.0F );
#endif
}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote(void) {
	int cs_offset;
	if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED )
		cs_offset = 0;
	else if ( cgs.clientinfo[cg.clientNum].team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if (!cgs.teamVoteTime[cs_offset]) { return; }

	// play a talk beep whenever it is modified
	if ( cgs.teamVoteModified[cs_offset] ) {
		cgs.teamVoteModified[cs_offset] = qfalse;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	int sec = (VOTE_TIME - (cg.time - cgs.teamVoteTime[cs_offset])) / 1000;
	if (sec < 0) { sec = 0; }
	char* s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
							cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset] );
	CG_DrawSmallString(0, 90, s, 1.0F);
}


static qboolean CG_DrawScoreboard( void ) {
#ifdef TEAMARENA
	static qboolean firstTime = qtrue;

	if (menuScoreboard) { menuScoreboard->window.flags &= ~WINDOW_FORCED; }
	if (cg_paused.integer) {
		cg.deferredPlayerLoading = 0;
		firstTime = qtrue;
		return qfalse;
	}

	// should never happen in Team Arena
	if (cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		cg.deferredPlayerLoading = 0;
		firstTime = qtrue;
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if ( cg.warmup && !cg.showScores ) { return qfalse; }

	if ( cg.showScores || cg.predictedPlayerState.pm_type == PM_DEAD || cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
	} else {
		if ( !CG_FadeColor( cg.scoreFadeTime, FADE_TIME ) ) {
			// next time scoreboard comes up, don't print killer
			cg.deferredPlayerLoading = 0;
			cg.killerName[0] = 0;
			firstTime = qtrue;
			return qfalse;
		}
	}

	if (menuScoreboard == NULL) {
		if ( cgs.gametype >= GT_TEAM ) {
			menuScoreboard = Menus_FindByName("teamscore_menu");
		} else {
			menuScoreboard = Menus_FindByName("score_menu");
		}
	}

	if (menuScoreboard) {
		if (firstTime) {
			CG_SetScoreSelection(menuScoreboard);
			firstTime = qfalse;
		}
		Menu_Paint(menuScoreboard, qtrue);
	}

	// load any models that have been deferred
	if ( ++cg.deferredPlayerLoading > 10 ) {
		CG_LoadDeferredPlayers();
	}

	return qtrue;
#else
  // TODO: Fix broken scoreboard
	return CG_DrawOldScoreboard();
#endif
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void ) {
//	int key;
#ifdef TEAMARENA
	//if (cg_singlePlayer.integer) {
	//	CG_DrawCenterString();
	//	return;
	//}
#else
	if (cgs.gametype == GT_SINGLE_PLAYER) {
		CG_DrawCenterString();
		return;
	}
#endif
	cg.scoreFadeTime = cg.time;
	cg.scoreBoardShowing = CG_DrawScoreboard();
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow(void) {
	if (!(cg.snap->ps.pm_flags & PMF_FOLLOW)) { return qfalse; }

	vec4_t		color;
  Vector4Set(&color, 1, 1, 1, 1);

	CG_DrawBigString( 320 - 9 * 8, 24, "following", 1.0F );

	const char* name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

	float x = 0.5 * ( GL_W - GIANT_WIDTH * CG_DrawStrlen( name ) );

	CG_DrawStringExt( x, 40, name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );

	return qtrue;
}



/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning( void ) {
	if (cg_drawAmmoWarning.integer == 0) { return; }
	if (!cg.lowAmmoWarning) { return; }

	const char* s = (cg.lowAmmoWarning == 2) ? "OUT OF AMMO" : "LOW AMMO WARNING";
	int w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawBigString(320 - w / 2, 64, s, 1.0F);
}


#ifdef TEAMARENA
/*
=================
CG_DrawProxWarning
=================
*/
static void CG_DrawProxWarning( void ) {
  static int proxTime;

	if( !(cg.snap->ps.eFlags & EF_TICKING ) ) {
    proxTime = 0;
		return;
	}

  if (proxTime == 0) { proxTime = cg.time; }

  int proxTick = 10 - ((cg.time - proxTime) / 1000);

	char s [32];
  if (proxTick > 0 && proxTick <= 5) {
    Com_sprintf(s, sizeof(s), "INTERNAL COMBUSTION IN: %i", proxTick);
  } else {
    Com_sprintf(s, sizeof(s), "YOU HAVE BEEN MINED");
  }

	int w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawBigStringColor( 320 - w / 2, 64 + BIGCHAR_HEIGHT, s, g_color_table[ColorIndex(COLOR_RED)] );
}
#endif


/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup( void ) {
	int			w;
	int			sec;
	int			i;
#ifdef TEAMARENA
	float		scale;
#else
	int			cw;
#endif
	clientInfo_t	*ci1, *ci2;
	const char	*s;

	sec = cg.warmup;
	if ( !sec ) {
		return;
	}

	if ( sec < 0 ) {
		s = "Waiting for players";		
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		CG_DrawBigString(320 - w / 2, 24, s, 1.0F);
		cg.warmupCount = 0;
		return;
	}

	if (cgs.gametype == GT_TOURNAMENT) {
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;
		for ( i = 0 ; i < cgs.maxclients ; i++ ) {
			if ( cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE ) {
				if ( !ci1 ) {
					ci1 = &cgs.clientinfo[i];
				} else {
					ci2 = &cgs.clientinfo[i];
				}
			}
		}

		if ( ci1 && ci2 ) {
			s = va( "%s vs %s", ci1->name, ci2->name );
#ifdef TEAMARENA
			w = CG_Text_Width(s, 0.6f, 0);
			CG_Text_Paint(320 - w / 2, 60, 0.6f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
#else
			w = CG_DrawStrlen( s );
			if ( w > GL_W / GIANT_WIDTH ) {
				cw = GL_W / w;
			} else {
				cw = GIANT_WIDTH;
			}
			CG_DrawStringExt( 320 - w * cw/2, 20,s, colorWhite, 
					qfalse, qtrue, cw, (int)(cw * 1.5f), 0 );
#endif
		}
	} else {
		if ( cgs.gametype == GT_RUN ) {
			s = "Run";
		} else if ( cgs.gametype == GT_TEAM ) {
			s = "Team Deathmatch";
		} else if ( cgs.gametype == GT_CTF ) {
			s = "Capture the Flag";
#ifdef TEAMARENA
		} else if ( cgs.gametype == GT_1FCTF ) {
			s = "One Flag CTF";
		} else if ( cgs.gametype == GT_OBELISK ) {
			s = "Overload";
		} else if ( cgs.gametype == GT_HARVESTER ) {
			s = "Harvester";
#endif
		} else {
			s = "";
		}
#ifdef TEAMARENA
		w = CG_Text_Width(s, 0.6f, 0);
		CG_Text_Paint(320 - w / 2, 90, 0.6f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
#else
		w = CG_DrawStrlen( s );
		if ( w > GL_W / GIANT_WIDTH ) {
			cw = GL_W / w;
		} else {
			cw = GIANT_WIDTH;
		}
		CG_DrawStringExt( 320 - w * cw/2, 25,s, colorWhite, 
				qfalse, qtrue, cw, (int)(cw * 1.1f), 0 );
#endif
	}

	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		cg.warmup = 0;
		sec = 0;
	}
	s = va( "Starts in: %i", sec + 1 );
	if ( sec != cg.warmupCount ) {
		cg.warmupCount = sec;
		switch ( sec ) {
		case 0:
			trap_S_StartLocalSound( cgs.media.count1Sound, CHAN_ANNOUNCER );
			break;
		case 1:
			trap_S_StartLocalSound( cgs.media.count2Sound, CHAN_ANNOUNCER );
			break;
		case 2:
			trap_S_StartLocalSound( cgs.media.count3Sound, CHAN_ANNOUNCER );
			break;
		default:
			break;
		}
	}

#ifdef TEAMARENA
	switch ( cg.warmupCount ) {
	case 0:
		scale = 0.54f;
		break;
	case 1:
		scale = 0.51f;
		break;
	case 2:
		scale = 0.48f;
		break;
	default:
		scale = 0.45f;
		break;
	}

	w = CG_Text_Width(s, scale, 0);
	CG_Text_Paint(320 - w / 2, 125, scale, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
#else
	switch ( cg.warmupCount ) {
	case 0:
		cw = 28;
		break;
	case 1:
		cw = 24;
		break;
	case 2:
		cw = 20;
		break;
	default:
		cw = 16;
		break;
	}

	w = CG_DrawStrlen( s );
	CG_DrawStringExt( 320 - w * cw/2, 70, s, colorWhite, 
			qfalse, qtrue, cw, (int)(cw * 1.5), 0 );
#endif
}

//==================================================================================
#ifdef TEAMARENA
/* 
=================
CG_DrawTimedMenus
=================
*/
void CG_DrawTimedMenus( void ) {
	if (cg.voiceTime) {
		int t = cg.time - cg.voiceTime;
		if ( t > 2500 ) {
			Menus_CloseByName("voiceMenu");
			trap_Cvar_Set("cl_conXOffset", "0");
			cg.voiceTime = 0;
		}
	}
}
#endif


//::KUA.add
//:::::::::::::::::
// Speedometer
//   Uses 0-1 range for each coordinate, instead of 640 virtual stuff
//   ie: 0.5 == middle of the screen
static void CG_DrawSpeed(float x, float y, float alpha){
  // Calculate current speed, based on ps->velocity
  float velX  = cg.snap->ps.velocity[0];           // Store X component of velocity
  float velY  = cg.snap->ps.velocity[1];           // Store Y component of velocity
  int   sint  = (int)sqrt(velX*velX + velY*velY);  // Calculate speed as length2D of velocity, and convert to integer
  char* speed = va("%i", sint);                    // Convert int speed to string
  CG_TextDraw(speed, &cgs.media.font, x, y, FONT_SCALE_DEFAULT, colorWhite, 0, 3, strlen(speed), TEXT_ALIGN_CENTER);
  // CG_DrawBigString(x-w, y, speed, alpha);             // Draw speed string to screen
}

// Replaces Naming of default function. Default renamed to CG_DrawTimelimit
static void CG_DrawTimer( float x, float y, float alpha ) {
  // Timers set to -1 are considered disabled. On player_die(), ClientSpawn(), etc
  int timer;
  if      (cg.timer_stop  >= 0) { timer = cg.timer_stop; }                        // If there is a timer_stop active, draw it and ignore active timer 
  else if (cg.timer_start >= 0) { timer = cg.snap->serverTime - cg.timer_start; } // Draw active timer instead
  else                          { timer = 0; }                                    // None of them is active, so draw a static 0 timer

  int msec = timer % 1000;
  int sec = timer / 1000;
  int min = sec / 60;
  sec -= min * 60;
  int ten = sec / 10;
  sec -= ten * 10;

  char* s = (min > 0) ? va("%i:%i%i:%03i", min, ten, sec, msec) : va("%i%i:%03i", ten, sec, msec);
  CG_TextDraw(s, &cgs.media.font, x, y, FONT_SCALE_DEFAULT, colorWhite, 0, 3, strlen(s), TEXT_ALIGN_CENTER);
}

static void CG_DrawTimerBest( float x, float y, float alpha ) {
  // Don't draw the best timer when its 0 or negative
  if (cg.timer_best <= 0) { return; }

  int timer = cg.timer_best; 

  int msec = timer % 1000;
  int sec = timer / 1000;
  int min = sec / 60;
  sec -= min * 60;
  int ten = sec / 10;
  sec -= ten * 10;

  char* s = (min > 0) ? va("%i:%i%i:%03i", min, ten, sec, msec) : va("%i%i:%03i", ten, sec, msec);
  CG_TextDraw(s, &cgs.media.font, x, y, FONT_SCALE_DEFAULT, colorWhite, 0, 3, strlen(s), TEXT_ALIGN_CENTER);
}
static void CG_DrawSmallIntCentered(int num, float x, float y, float alpha){
  char* snum = va("%i", num);
  CG_TextDraw(snum, &cgs.media.font, x, y, FONT_SCALE_DEFAULT, colorWhite, 0, 3, strlen(snum), TEXT_ALIGN_CENTER);
}
static void CG_DrawCrouchslideTime(float x, float y, float alpha){
  CG_DrawSmallIntCentered(cg.snap->ps.stats[STAT_TIME_CROUCHSLIDE], x, y, alpha);
}
static void CG_DrawPMTime(float x, float y, float alpha){
  CG_DrawSmallIntCentered(cg.snap->ps.pm_time, x, y, alpha);
}
static void CG_DrawHoldboost(float x, float y, float alpha){
  CG_DrawSmallIntCentered(cg.snap->ps.stats[STAT_JUMP_HOLDBOOST], x, y, alpha);
}
//:::::::::::::::::
static void CG_DrawDevVersion(qboolean active) {
  if (!active) {return;}
  char* vers = G_FULLNAME SEP G_VERSION "  |  Engine: " E_VERSION;
  float x = 0.995;
  float y = 0.99;
  CG_TextDraw(vers, &cgs.media.font, x, y, 0.8, colorWhite, 0, 0, strlen(vers), TEXT_ALIGN_RIGHT);
}
//:::::::::::::::::
// Test Font Drawing
static void CG_TestDrawStr() {
  //................................................
  char* line1 = "Old system..";
  // char* line2 = "Next line is Font support:";
  int   lineHeight = SMALLCHAR_HEIGHT;
  CG_DrawSmallString(SCREEN_WIDTH*0.1, SCREEN_HEIGHT*0.5-(lineHeight), line1, 1.0);
  // CG_DrawSmallString(SCREEN_WIDTH*0.5, SCREEN_HEIGHT*0.5-(lineHeight*2), line2, 1.0);
  //................................................
  float x         = cgs.glconfig.vidWidth  * 0.1;
  float y         = cgs.glconfig.vidHeight * 0.8;
  float adjust    = 0;
  int   style     = 3;
  char* text = "font Is WORKING!  1[;]()^'_";
  int   maxLength = strlen(text);
  CG_TextDrawStr(text, &cgs.media.font, x, y, FONT_SCALE_DEFAULT, colorWhite, adjust, style, maxLength);
}
//::::::::::::::
//::KUA.end



/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D(stereoFrame_t stereoFrame)
{
#ifdef TEAMARENA
	if (cgs.orderPending && cg.time > cgs.orderTime) {
		CG_CheckOrderPending();
	}
#endif
	if (cg.levelShot) { return; }  // taking a levelshot for the menu, don't draw anything
	if (cg_draw2D.integer == 0) { return; }  // 2D drawing is disabled
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		CG_DrawIntermission();
		return;
	}

/*
	if (cg.cameraMode) {
		return;
	}
*/
	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		CG_DrawSpectator();
		if(stereoFrame == STEREO_CENTER)
			CG_DrawCrosshair();
		CG_DrawCrosshairNames();
	} else {
		// don't draw any status if dead or the scoreboard is being explicitly shown
		if ( !cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0 ) {
#ifdef TEAMARENA
			if ( cg_drawStatus.integer ) {
				Menu_PaintAll();
				CG_DrawTimedMenus();
			}
#else
			CG_DrawStatusBar();
#endif
			CG_DrawAmmoWarning();

#ifdef TEAMARENA
			CG_DrawProxWarning();
#endif      
			if(stereoFrame == STEREO_CENTER)
				CG_DrawCrosshair();
			CG_DrawCrosshairNames();
			CG_DrawWeaponSelect();

#ifndef TEAMARENA
			CG_DrawHoldableItem();
#else
			//CG_DrawPersistantPowerup();
#endif
			CG_DrawReward();
		}
	}

	if ( cgs.gametype >= GT_TEAM ) {
#ifndef TEAMARENA
		CG_DrawTeamInfo();
#endif
	}

	CG_DrawVote();
	CG_DrawTeamVote();

	CG_DrawLagometer();

#ifdef TEAMARENA
	if (!cg_paused.integer) {
		CG_DrawUpperRight(stereoFrame);
	}
#else
	CG_DrawUpperRight(stereoFrame);
#endif

#ifndef TEAMARENA
	CG_DrawLowerRight();
	CG_DrawLowerLeft();
	//::KUA.chg -> Taken out of the lowerleft function
	CG_DrawPickupItem(0.5, 0.75);
	CG_DrawPowerups(0.5, 1);
	CG_DrawScore(0.35, 0.02, TEXT_ALIGN_RIGHT);
  //::KUA.end
#endif

	if (!CG_DrawFollow()) {
		CG_DrawWarmup();
	}

	// don't draw center string if scoreboard is up
	cg.scoreBoardShowing = CG_DrawScoreboard();
	if ( !cg.scoreBoardShowing) {
		CG_DrawCenterString();
	}
	
    //::KUA.chg
    //::::::::::::::
    if (!cg.scoreBoardShowing) {    //FIXME: Segmentation fault when drawing the scoreboard while speed is being drawn
      //TODO: Proper screen allignment
      CG_DrawSpeed(hud_speed_x.value, hud_speed_y.value, 1.0); //TODO: Add cvar conditional and settings
      CG_DrawTimer(hud_timerActive_x.value, hud_timerActive_y.value, 1.0F);
      CG_DrawTimerBest(hud_timerBest_x.value, hud_timerBest_y.value, 1.0F);
      if (cg.snap->ps.stats[STAT_TIME_CROUCHSLIDE]) { CG_DrawCrouchslideTime(0.5, 0.65, 1.0F); }
      if (cg.snap->ps.stats[STAT_JUMP_HOLDBOOST])   { CG_DrawHoldboost      (0.5, 0.65, 1.0F); }
      if (cg.snap->ps.pm_time)                      { CG_DrawPMTime         (0.5, 0.60, 1.0F); }
      CG_DrawDevVersion(qtrue);
      // CG_TestDrawStr();
    }
    //::::::::::::::
    //::KUA.end
}


/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView ) {
	// optionally draw the info screen instead
	if ( !cg.snap ) {
		CG_DrawInformation();
		return;
	}

	// optionally draw the tournement scoreboard instead
	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR &&
		( cg.snap->ps.pm_flags & PMF_SCOREBOARD ) ) {
		CG_DrawTourneyScoreboard();
		return;
	}

	// clear around the rendered view if sized down
	CG_TileClear();

	if(stereoView != STEREO_CENTER) { CG_DrawCrosshair3D(); }

	// draw 3D view
	trap_R_RenderScene( &cg.refdef );

	// draw status bar and other floating elements
 	CG_Draw2D(stereoView);
}



