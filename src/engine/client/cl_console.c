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
// console.c

#include "client.h"

#define  DEFAULT_CONSOLE_WIDTH 78
#define  MAX_CONSOLE_WIDTH 120

#define  NUM_CON_TIMES  64

#define  CON_TEXTSIZE   65536

int bigchar_width;
int bigchar_height;
int smallchar_width;
int smallchar_height;

typedef struct {
	qboolean	initialized;

	short	text[CON_TEXTSIZE];
	int		current;		// line where next message will be printed
	int		x;				// offset in current line for next print
	int		display;		// bottom of console displays this line

	int 	linewidth;		// characters across screen
	int		totallines;		// total lines in console scrollback

	float	xadjust;		// for wide aspect screens

	float	displayFrac;	// aproaches finalFrac at scr_conspeed
	float	finalFrac;		// 0.0 to 1.0 lines of console to display
	float	userFrac;		// based on con_height* cvars, different with modifier keys

	int		vislines;		// in scanlines

	int		times[NUM_CON_TIMES];	// cls.realtime time the line was generated
								// for transparent notify lines
	vec4_t	color;

	int		viswidth;
	int		vispage;

	qboolean carriageReturn;
	qboolean newline;

	char prefix[9];			// fixed width prefix, hh:mm:ss
	char date[11];			// fixed width date, YYYY-MM-DD
} console_t;

extern  qboolean    chat_team;
extern  int         chat_playerNum;

console_t	con;

cvar_t		*con_timestamp;
cvar_t		*con_conspeed;
cvar_t		*con_autoclear;
cvar_t		*con_notifytime;
cvar_t		*con_notifylines;
cvar_t		*con_notifyx;
cvar_t		*con_notifyy;
cvar_t		*con_notifykeep;
cvar_t		*con_scale;
cvar_t		*con_height;
cvar_t		*con_heightShift;
cvar_t		*con_heightCtrl;
cvar_t		*con_heightAlt;
cvar_t		*con_timedisplay;
static qboolean con_timedisplay_show = qfalse;

int			g_console_field_width;

#ifdef USE_PCRE
#define PCRE_STATIC 1
#include <pcre.h>
#define		MAX_CON_FILTERS 10
static	cvar_t* con_filters[MAX_CON_FILTERS];
static	cvar_t* con_filter;
static	pcre* con_filters_compiled[MAX_CON_FILTERS];

/*
================
Con_ShowPCREVersion_f
================
*/
static void Con_ShowPCREVersion_f( void ) {
	Com_Printf( "%s\n", pcre_version() );
}


/*
===============
Con_UpdateFilters

Prepares regular expressions in con_filter* cvars for matching.
Executed on init and on every console print.
Doesn't do anything if filter cvars have not been changed since last run.  
===============
*/
static void Con_UpdateFilters( void ) {
	const char* errptr;
	int erroffset;
	cvar_t** cvar;
	pcre** regex;

	for (cvar = con_filters; cvar - con_filters < MAX_CON_FILTERS; cvar++) {
		if (!*cvar || !(*cvar)->modified) {
			continue;
		}
		regex = con_filters_compiled + (cvar - con_filters);
		(*cvar)->modified = qfalse;
		if (!strlen((*cvar)->string)) {
			*regex = NULL;
			continue;
		}
		*regex = pcre_compile((*cvar)->string, 0, &errptr, &erroffset, NULL);
		if (!*regex) {
			Com_Printf( S_COLOR_YELLOW "Failed to compile %s at character %i: %s\n", (*cvar)->name, erroffset, errptr);
			Cvar_Set((*cvar)->name, "");
			(*cvar)->modified = qfalse;
		}

	}
}


/*
===============
Con_CheckFilters

Check if a string is a match for any of the regular expressions in con_filter* cvars.
===============
*/
static qboolean Con_CheckFilters( const char* txt ) {
	char txt_nocolor[MAXPRINTMSG];
	int ovector[30];
	pcre** regex;

	Con_UpdateFilters();

	Q_strncpyz( txt_nocolor, txt, sizeof( txt_nocolor ) );
	Q_DecolorStr( txt_nocolor );

	for ( regex = con_filters_compiled; regex - con_filters_compiled < MAX_CON_FILTERS; regex++ ) {
		if ( *regex && pcre_exec( *regex, NULL, txt_nocolor, strlen( txt_nocolor ), 0, 0, ovector, 30 ) > 0 ) {
			return qtrue;
		}
	}

	return qfalse;
}


/*
===============
Con_InitFilters
===============
*/
static void Con_InitFilters( void ) {
	int		i;
	con_filter = Cvar_Get( "con_filter", "0", CVAR_ARCHIVE_ND );
	for ( i = 0; i < MAX_CON_FILTERS; i++ ) {
		con_filters[i] = Cvar_Get(va("con_filter%i", i), "", CVAR_ARCHIVE_ND);
	}
	Con_UpdateFilters();
}
#endif /* USE_PCRE */

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f( void ) {
	// Can't toggle the console when it's the only thing available
	if ( cls.state == CA_DISCONNECTED && Key_GetCatcher() == KEYCATCH_CONSOLE ) {
		return;
	}

	if ( con_autoclear->integer ) {
		Field_Clear( &g_consoleField );
	}

	Con_ResetFieldWidth();

	if (!con_notifykeep->integer) {
		Con_ClearNotify();
	}

	if ( keys[K_SHIFT].down )	{
		con.userFrac = con_heightShift->value;
	}
	else if ( keys[K_CTRL].down )	{
		con.userFrac = con_heightCtrl->value;
	}
	else if ( keys[K_ALT].down )	{
		con.userFrac = con_heightAlt->value;
	}
	else {
		con.userFrac = con_height->value;
	}

	Key_SetCatcher( Key_GetCatcher() ^ KEYCATCH_CONSOLE );
}


/*
================
Con_MessageMode_f
================
*/
static void Con_MessageMode_f( void ) {
	chat_playerNum = -1;
	chat_team = qfalse;
	Field_Clear( &chatField );
	chatField.widthInChars = 30;

	Key_SetCatcher( Key_GetCatcher() ^ KEYCATCH_MESSAGE );
}


/*
================
Con_MessageMode2_f
================
*/
static void Con_MessageMode2_f( void ) {
	chat_playerNum = -1;
	chat_team = qtrue;
	Field_Clear( &chatField );
	chatField.widthInChars = 25;
	Key_SetCatcher( Key_GetCatcher() ^ KEYCATCH_MESSAGE );
}


/*
================
Con_MessageMode3_f
================
*/
static void Con_MessageMode3_f( void ) {
	chat_playerNum = cgvm ? VM_Call( cgvm, 0, CG_CROSSHAIR_PLAYER ) : -1;
	if ( chat_playerNum < 0 || chat_playerNum >= MAX_CLIENTS ) {
		chat_playerNum = -1;
		return;
	}
	chat_team = qfalse;
	Field_Clear( &chatField );
	chatField.widthInChars = 30;
	Key_SetCatcher( Key_GetCatcher() ^ KEYCATCH_MESSAGE );
}


/*
================
Con_MessageMode4_f
================
*/
static void Con_MessageMode4_f( void ) {
	chat_playerNum = cgvm ? VM_Call( cgvm, 0, CG_LAST_ATTACKER ) : -1;
	if ( chat_playerNum < 0 || chat_playerNum >= MAX_CLIENTS ) {
		chat_playerNum = -1;
		return;
	}
	chat_team = qfalse;
	Field_Clear( &chatField );
	chatField.widthInChars = 30;
	Key_SetCatcher( Key_GetCatcher() ^ KEYCATCH_MESSAGE );
}


/*
================
Con_Clear_f
================
*/
static void Con_Clear_f( void ) {
	int		i;

	for ( i = 0 ; i < con.linewidth ; i++ ) {
		con.text[i] = ( ColorIndex( COLOR_WHITE ) << 8 ) | ' ';
	}

	con.x = 0;
	con.current = 0;
	con.carriageReturn = qfalse;
	con.newline = qtrue;

	Con_Bottom();		// go to end
}


/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
static void Con_Dump_f( void )
{
	int		l, x, i, n;
	short	*line;
	fileHandle_t	f;
	int		bufferlen;
	char	*buffer;
	char	filename[ MAX_OSPATH ];
	const char *ext;

	if ( Cmd_Argc() != 2 )
	{
		Com_Printf( "usage: condump <filename>\n" );
		return;
	}

	Q_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".txt" );

	if ( !FS_AllowedExtension( filename, qfalse, &ext ) ) {
		Com_Printf( "%s: Invalid filename extension '%s'.\n", __func__, ext );
		return;
	}

	f = FS_FOpenFileWrite( filename );
	if ( f == FS_INVALID_HANDLE )
	{
		Com_Printf( "ERROR: couldn't open %s.\n", filename );
		return;
	}

	Com_Printf( "Dumped console text to %s.\n", filename );

	if ( con.current >= con.totallines ) {
		n = con.totallines;
		l = con.current + 1;
	} else {
		n = con.current + 1;
		l = 0;
	}

	bufferlen = con.linewidth + ARRAY_LEN( Q_NEWLINE ) * sizeof( char );
	buffer = Hunk_AllocateTempMemory( bufferlen );

	// write the remaining lines
	buffer[ bufferlen - 1 ] = '\0';

	for ( i = 0; i < n ; i++, l++ )
	{
		line = con.text + (l % con.totallines) * con.linewidth;
		// store line
		for( x = 0; x < con.linewidth; x++ )
			buffer[ x ] = line[ x ] & 0xff;
		buffer[ con.linewidth ] = '\0';
		// terminate on ending space characters
		for ( x = con.linewidth - 1 ; x >= 0 ; x-- ) {
			if ( buffer[ x ] == ' ' )
				buffer[ x ] = '\0';
			else
				break;
		}
		Q_strcat( buffer, bufferlen, Q_NEWLINE );
		FS_Write( buffer, strlen( buffer ), f );
	}

	Hunk_FreeTempMemory( buffer );
	FS_FCloseFile( f );
}


/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify( void ) {
	int		i;

	for ( i = 0 ; i < NUM_CON_TIMES ; i++ ) {
		con.times[i] = 0;
	}
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize( void )
{
	int		i, j, width, oldwidth, oldtotallines, oldcurrent, numlines, numchars;
	short	tbuf[CON_TEXTSIZE], *src, *dst;
	static int old_width, old_vispage;
	int		vispage;
	float	scale;

	if ( con.viswidth == cls.glconfig.vidWidth && !con_scale->modified ) {
		return;
	}

	scale = con_scale->value;

	con.viswidth = cls.glconfig.vidWidth;

	smallchar_width = SMALLCHAR_WIDTH * scale * cls.con_factor;
	smallchar_height = SMALLCHAR_HEIGHT * scale * cls.con_factor;
	bigchar_width = BIGCHAR_WIDTH * scale * cls.con_factor;
	bigchar_height = BIGCHAR_HEIGHT * scale * cls.con_factor;

	if ( cls.glconfig.vidWidth == 0 ) // video hasn't been initialized yet
	{
		g_console_field_width = DEFAULT_CONSOLE_WIDTH;
		width = DEFAULT_CONSOLE_WIDTH * scale;
		con.linewidth = width;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		con.vispage = 4;

		Con_Clear_f();
	}
	else
	{
		width = ((cls.glconfig.vidWidth / smallchar_width) - 2);

		g_console_field_width = width;
		Con_ResetFieldWidth();

		if ( width > MAX_CONSOLE_WIDTH )
			width = MAX_CONSOLE_WIDTH;

		vispage = cls.glconfig.vidHeight / ( smallchar_height * 2 ) - 1;

		if ( old_vispage == vispage && old_width == width )
			return;

		oldwidth = con.linewidth;
		oldtotallines = con.totallines;
		oldcurrent = con.current;

		con.linewidth = width;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		con.vispage = vispage;

		old_vispage = vispage;
		old_width = width;

		numchars = oldwidth;
		if ( numchars > con.linewidth )
			numchars = con.linewidth;

		if ( oldcurrent > oldtotallines )
			numlines = oldtotallines;
		else
			numlines = oldcurrent + 1;

		if ( numlines > con.totallines )
			numlines = con.totallines;

		Com_Memcpy( tbuf, con.text, CON_TEXTSIZE * sizeof( short ) );

		for ( i = 0; i < CON_TEXTSIZE; i++ )
			con.text[i] = (ColorIndex(COLOR_WHITE)<<8) | ' ';

		for ( i = 0; i < numlines; i++ )
		{
			src = &tbuf[ ((oldcurrent - i + oldtotallines) % oldtotallines) * oldwidth ];
			dst = &con.text[ (numlines - 1 - i) * con.linewidth ];
			for ( j = 0; j < numchars; j++ )
				*dst++ = *src++;
		}

		Con_ClearNotify();

		con.current = numlines - 1;
	}

	con.display = con.current;

	con_scale->modified = qfalse;
}


/*
==================
Con_ResetFieldWidth
==================
*/
void Con_ResetFieldWidth( void )
{
	g_consoleField.widthInChars = g_console_field_width -
		(con_timedisplay && con_timedisplay->integer & 1 ? sizeof(con.prefix) : 0) -
		(con_timedisplay && con_timedisplay->integer & 2 ? sizeof(con.prefix) + sizeof(con.date) : 0);
	if ( g_consoleField.widthInChars <= 10 ) {
		g_consoleField.widthInChars = g_console_field_width;
		con_timedisplay_show = qfalse;
	} else {
		con_timedisplay_show = qtrue;
	}
}


/*
==================
Cmd_CompleteTxtName
==================
*/
static void Cmd_CompleteTxtName( char *args, int argNum ) {
	if ( argNum == 2 ) {
		Field_CompleteFilename( "", "txt", qfalse, FS_MATCH_EXTERN | FS_MATCH_STICK );
	}
}


/*
===============
Con_UpdateDateTime
===============
*/
static void Con_UpdateDateTime( void )
{
	qtime_t	now;

	Com_RealTime( &now );
	assert( sizeof(con.prefix) == 9 );
	Com_sprintf( con.prefix, sizeof(con.prefix), "%02d:%02d:%02d", now.tm_hour, now.tm_min, now.tm_sec );
	assert( sizeof(con.date) == 11 );
	Com_sprintf( con.date, sizeof(con.date), "%04d-%02d-%02d", 1900 + now.tm_year, 1 + now.tm_mon, now.tm_mday );
}


/*
================
Con_Init
================
*/
void Con_Init( void )
{
	con_timestamp = Cvar_Get( "con_timestamp", "1", CVAR_ARCHIVE );
	con_notifytime = Cvar_Get( "con_notifyTime", "3", CVAR_ARCHIVE );
	con_notifylines = Cvar_Get( "con_notifyLines", "3", CVAR_ARCHIVE );
	Cvar_CheckRange( con_notifylines, "0", XSTRING( NUM_CON_TIMES ), CV_INTEGER );
	con_notifyx = Cvar_Get( "con_notifyX", "0", CVAR_ARCHIVE );
	con_notifyy = Cvar_Get( "con_notifyY", "0", CVAR_ARCHIVE );
	con_notifykeep = Cvar_Get( "con_notifyKeep", "0", CVAR_ARCHIVE );
	con_conspeed = Cvar_Get( "con_togglespeed", "3", 0 );
	con_height = Cvar_Get( "con_height", "0.5", CVAR_ARCHIVE_ND );
	con_heightShift = Cvar_Get( "con_heightShift", "0.5", CVAR_ARCHIVE_ND ); // same default because Shift+Esc opens the console too 
	con_heightCtrl = Cvar_Get( "con_heightCtrl", "0.25", CVAR_ARCHIVE_ND );
	con_heightAlt = Cvar_Get( "con_heightAlt", "1.0", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( con_height, "0.1", "1.0", CV_FLOAT ); // 0.1 to prevent the console from being completely hidden
	Cvar_CheckRange( con_heightShift, "0.1", "1.0", CV_FLOAT );
	Cvar_CheckRange( con_heightCtrl, "0.1", "1.0", CV_FLOAT );
	Cvar_CheckRange( con_heightAlt, "0.1", "1.0", CV_FLOAT );
	con_timedisplay = Cvar_Get( "con_timedisplay", "3", CVAR_ARCHIVE_ND );

	Con_UpdateDateTime();

	con_autoclear = Cvar_Get("con_autoclear", "1", CVAR_ARCHIVE_ND);
	con_scale = Cvar_Get( "con_scale", "1", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( con_scale, "0.5", "8", CV_FLOAT );

	Field_Clear( &g_consoleField );
	Con_ResetFieldWidth();

	Cmd_AddCommand( "clear", Con_Clear_f );
	Cmd_AddCommand( "clearnotify", Con_ClearNotify );
	Cmd_AddCommand( "condump", Con_Dump_f );
	Cmd_SetCommandCompletionFunc( "condump", Cmd_CompleteTxtName );
	Cmd_AddCommand( "toggleconsole", Con_ToggleConsole_f );
	Cmd_AddCommand( "messagemode", Con_MessageMode_f );
	Cmd_AddCommand( "messagemode2", Con_MessageMode2_f );
	Cmd_AddCommand( "messagemode3", Con_MessageMode3_f );
	Cmd_AddCommand( "messagemode4", Con_MessageMode4_f );
#ifdef USE_PCRE
	Cmd_AddCommand( "pcre_version", Con_ShowPCREVersion_f );
	Con_InitFilters();
#endif /* USE_PCRE */
}


/*
================
Con_Shutdown
================
*/
void Con_Shutdown( void )
{
	Cmd_RemoveCommand( "clear" );
	Cmd_RemoveCommand( "clearnotify" );
	Cmd_RemoveCommand( "condump" );
	Cmd_RemoveCommand( "toggleconsole" );
	Cmd_RemoveCommand( "messagemode" );
	Cmd_RemoveCommand( "messagemode2" );
	Cmd_RemoveCommand( "messagemode3" );
	Cmd_RemoveCommand( "messagemode4" );
#ifdef USE_PCRE
	Cmd_RemoveCommand( "pcre_version" );
#endif /* USE_PCRE */
}


/*
===============
Con_Fixup
===============
*/
static void Con_Fixup( void ) 
{
	int filled;

	if ( con.current >= con.totallines ) {
		filled = con.totallines;
	} else {
		filled = con.current + 1;
	}

	if ( filled <= con.vispage ) {
		con.display = con.current;
	} else if ( con.current - con.display > filled - con.vispage ) {
		con.display = con.current - filled + con.vispage;
	} else if ( con.display > con.current ) {
		con.display = con.current;
	}
}


/*
===============
Con_Prefix
===============
*/
static void Con_Prefix( void )
{
	assert( con.x == 0 );

	if ( con_timestamp && con_timestamp->integer ) {
		Con_UpdateDateTime();
		assert( strlen(con.prefix) == sizeof(con.prefix) - 1 );
		con.x = sizeof(con.prefix); // prefix + ' '
		short *s = &con.text[ ( con.current % con.totallines ) * con.linewidth ];
		for ( int i = 0; i < sizeof(con.prefix) - 1; ++i )
			*s++ = (9<<8) | con.prefix[i];
		*s++ = (ColorIndex(COLOR_WHITE)<<8) | ' ';
	}
}


/*
===============
Con_NewLine

Move to newline only when we _really_ need this
===============
*/
static void Con_NewLine( void )
{
	short *s;
	int i;

	assert( con.newline == qtrue );
	assert( con.x == 0 );

	// follow last line
	if ( con.display == con.current )
		con.display++;
	con.current++;

	Con_Prefix();

	s = &con.text[ ( con.current % con.totallines ) * con.linewidth + con.x];
	for ( i = con.x; i < con.linewidth ; i++ )
		*s++ = (ColorIndex(COLOR_WHITE)<<8) | ' ';
}


/*
===============
Con_CarriageReturn
===============
*/
static void Con_CarriageReturn( void )
{
	con.carriageReturn = qtrue;
	con.x = 0;
}


/*
===============
Con_Linefeed
===============
*/
static void Con_Linefeed( qboolean skipnotify )
{
	// mark time for transparent overlay
	if ( con.current >= 0 )	{
		if ( skipnotify )
			con.times[ con.current % NUM_CON_TIMES ] = 0;
		else
			con.times[ con.current % NUM_CON_TIMES ] = cls.realtime;
	}

	if ( con.newline ) {
		Con_NewLine();
	} else {
		con.newline = qtrue;
	}
	con.x = 0;

	Con_Fixup();
}


static int Con_WordLength( const char *txt )
{
	assert( *txt );
	int l = 0;
	while ( *txt > ' ' ) {
		if ( Q_IsColorString( txt ) ) {
			txt += 2;
		} else {
			++l;
			++txt;
		}
	}
	return l;
}


/*
================
CL_ConsolePrint

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/
void CL_ConsolePrint( const char *txt ) {
	int		y;
	int		c, l;
	int		colorIndex;
	qboolean skipnotify = qfalse;		// NERVE - SMF
	int prev;							// NERVE - SMF

	// for some demos we don't want to ever show anything on the console
	if ( cl_noprint && cl_noprint->integer ) {
		return;
	}

#ifdef USE_PCRE
	if ( con_filter && con_filter->integer && Con_CheckFilters( txt ) ) {
		return;
	}
#endif /* USE_PCRE */

	// TTimo - prefix for text that shows up in console but not in notify
	// backported from RTCW
	if ( !Q_strncmp( txt, "[skipnotify]", 12 ) ) {
		skipnotify = qtrue;
		txt += 12;
	}

	if ( !con.initialized ) {
		static cvar_t null_cvar = { 0 };
		con.color[0] =
		con.color[1] =
		con.color[2] =
		con.color[3] = 1.0f;
		con.viswidth = -9999;
		cls.con_factor = 1.0f;
		con_scale = &null_cvar;
		con_scale->value = 1.0f;
		con_scale->modified = qtrue;
		Con_CheckResize();
		con.initialized = qtrue;
	}

	l = 0;
	colorIndex = ColorIndex( COLOR_WHITE );

	while ( (c = *txt) != 0 ) {
		if ( Q_IsColorString( txt ) ) {
			colorIndex = ColorIndexFromChar( *(txt+1) );
			txt += 2;
			continue;
		}

		switch( c )
		{
		case '\n':
			Con_Linefeed( skipnotify );
			break;
		case '\r':
			Con_CarriageReturn();
			break;
		default:
			if ( con.x == con.linewidth ) {
				Con_Linefeed( skipnotify );
			}

			if ( con.newline ) {
				Con_NewLine();
				Con_Fixup();
				con.carriageReturn = qfalse;
				con.newline = qfalse;
			} else if ( con.carriageReturn ) {
				Con_Prefix();
				con.carriageReturn = qfalse;
				assert( con.newline == qfalse );
			}

			if ( l == 0 ) {
				l = Con_WordLength(txt);
				l = l ? l : 1; // if no word, just advance by 1

				// word wrap
				if ( con.x + l > con.linewidth && (con_timestamp && con_timestamp->integer ? sizeof(con.prefix) : 0) + l < con.linewidth ) {
					Con_Linefeed( skipnotify );
					continue;
				}
			}
			assert( l > 0 );

			// display character and advance
			assert( con.x < con.linewidth );
			y = con.current % con.totallines;
			con.text[y * con.linewidth + con.x ] = (colorIndex << 8) | (c & 255);
			con.x++;
			--l;
			break;
		}

		txt++;
	}

	// mark time for transparent overlay
	if ( con.current >= 0 ) {
		if ( skipnotify ) {
			prev = con.current % NUM_CON_TIMES - 1;
			if ( prev < 0 )
				prev = NUM_CON_TIMES - 1;
			con.times[ prev ] = 0;
		} else {
			con.times[ con.current % NUM_CON_TIMES ] = cls.realtime;
		}
	}
}


/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

Draw the editline after a ] prompt
================
*/
static void Con_DrawInput( void ) {
	int		y;
	int		offset = 0;

	if ( cls.state != CA_DISCONNECTED && !(Key_GetCatcher( ) & KEYCATCH_CONSOLE ) ) {
		return;
	}

	y = con.vislines - ( smallchar_height * 2 );

	if ( con_timedisplay->integer & 1 && con_timedisplay_show ) {
		offset = sizeof(con.prefix);
		re.SetColor( g_color_table[ColorIndexFromChar('8')] );
		SCR_DrawSmallString( con.xadjust + smallchar_width, y, con.prefix, sizeof(con.prefix) - 1 );
	}
	re.SetColor( con.color );

	SCR_DrawSmallChar( con.xadjust + (offset + 1) * smallchar_width, y, ']' );

	Field_Draw( &g_consoleField, con.xadjust + (offset + 2) * smallchar_width, y,
		SCREEN_WIDTH - 3 * smallchar_width, qtrue, qtrue );
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
static void Con_DrawNotify( void )
{
	int		x, v;
	short	*text;
	int		i;
	int		time;
	int		skip;
	int		currentColorIndex;
	int		colorIndex;
	
	float notifyx = con_notifyx->value;
	float notifyy = con_notifyy->value;
	SCR_AdjustFrom640(&notifyx, &notifyy, NULL, NULL);

	currentColorIndex = ColorIndex( COLOR_WHITE );
	re.SetColor( g_color_table[ currentColorIndex ] );

	v = 0;
	for (i = con.current - con_notifylines->integer + 1; i <= con.current; i++)
	{
		if (i < 0)
			continue;
		time = con.times[i % NUM_CON_TIMES];
		if (time == 0)
			continue;
		time = cls.realtime - time;
		if (con_notifytime->value != -1 && time >= con_notifytime->value*1000 )
			continue;
		text = con.text + (i % con.totallines)*con.linewidth;

		if (cl.snap.ps.pm_type != PM_INTERMISSION && Key_GetCatcher( ) & (KEYCATCH_UI | KEYCATCH_CGAME) ) {
			continue;
		}

		for (x = con_timestamp->integer ? sizeof(con.prefix) : 0; x < con.linewidth ; x++) {
			if ( ( text[x] & 0xff ) == ' ' ) {
				continue;
			}
			colorIndex = ( text[x] >> 8 ) & 63;
			if ( currentColorIndex != colorIndex ) {
				currentColorIndex = colorIndex;
				re.SetColor( g_color_table[ colorIndex ] );
			}
			SCR_DrawSmallChar( (int)notifyx + con.xadjust + (x + 1 - (con_timestamp->integer ? sizeof(con.prefix) : 0)) * smallchar_width, v + (int)notifyy, text[x] & 0xff );
		}

		v += smallchar_height;
	}

	re.SetColor( NULL );

	if ( Key_GetCatcher() & (KEYCATCH_UI | KEYCATCH_CGAME) ) {
		return;
	}

	// draw the chat line
	if ( Key_GetCatcher( ) & KEYCATCH_MESSAGE )
	{
		v += (int)notifyy;
		// rescale to virtual 640x480 space
		v /= cls.glconfig.vidHeight / 480.0;

		if (chat_team)
		{
			SCR_DrawBigString( smallchar_width, v, "say_team:", 1.0f, qfalse );
			skip = 10;
		}
		else
		{
			SCR_DrawBigString( smallchar_width, v, "say:", 1.0f, qfalse );
			skip = 5;
		}

		Field_BigDraw( &chatField, skip * BIGCHAR_WIDTH, v,
			SCREEN_WIDTH - ( skip + 1 ) * BIGCHAR_WIDTH, qtrue, qtrue );
	}
}


/*
================
Con_DrawSolidConsole

Draws the console with the solid background
================
*/
static void Con_DrawSolidConsole( float frac ) {

	static float conColorValue[4] = { 0.0, 0.0, 0.0, 0.0 };
	// for cvar value change tracking
	static char  conColorString[ MAX_CVAR_VALUE_STRING ] = { '\0' };

	int				i, x, y;
	int				rows;
	short			*text;
	int				row;
	int				lines;
	int				currentColorIndex;
	int				colorIndex;
	float			yf, wf;
	char			buf[ MAX_CVAR_VALUE_STRING ], *v[4];

	lines = cls.glconfig.vidHeight * frac;
	if ( lines <= 0 )
		return;

	if ( re.FinishBloom )
		re.FinishBloom();

	if ( lines > cls.glconfig.vidHeight )
		lines = cls.glconfig.vidHeight;

	wf = SCREEN_WIDTH;

	// draw the background
	yf = frac * SCREEN_HEIGHT;

	// on wide screens, we will center the text
	con.xadjust = 0;
	SCR_AdjustFrom640( &con.xadjust, &yf, &wf, NULL );

	if ( yf < 1.0 ) {
		yf = 0;
	} else {
		// custom console background color
		if ( cl_conColor->string[0] ) {
			// track changes
			if ( strcmp( cl_conColor->string, conColorString ) )
			{
				Q_strncpyz( conColorString, cl_conColor->string, sizeof( conColorString ) );
				Q_strncpyz( buf, cl_conColor->string, sizeof( buf ) );
				Com_Split( buf, v, 4, ' ' );
				for ( i = 0; i < 4 ; i++ ) {
					conColorValue[ i ] = Q_atof( v[ i ] ) / 255.0f;
					if ( conColorValue[ i ] > 1.0f ) {
						conColorValue[ i ] = 1.0f;
					} else if ( conColorValue[ i ] < 0.0f ) {
						conColorValue[ i ] = 0.0f;
					}
				}
			}
			re.SetColor( conColorValue );
			re.DrawStretchPic( 0, 0, wf, yf, 0, 0, 1, 1, cls.whiteShader );
		} else {
			re.SetColor( g_color_table[ ColorIndex( COLOR_WHITE ) ] );
			re.DrawStretchPic( 0, 0, wf, yf, 0, 0, 1, 1, cls.consoleShader );
		}

	}

	re.SetColor( g_color_table[ ColorIndex( COLOR_CYAN ) ] );
	re.DrawStretchPic( 0, yf, wf, 2, 0, 0, 1, 1, cls.whiteShader );

	//y = yf;

	// draw the version number
	SCR_DrawSmallString( cls.glconfig.vidWidth - ( ARRAY_LEN( Q3_VERSION ) ) * smallchar_width,
		lines - smallchar_height, Q3_VERSION, ARRAY_LEN( Q3_VERSION ) - 1 );

	if ( con_timedisplay->integer && con_timedisplay_show ) {
		Con_UpdateDateTime();
	}

	// draw date and time
	if ( con_timedisplay->integer & 2 && con_timedisplay_show ) {
		SCR_DrawSmallString( cls.glconfig.vidWidth - (sizeof(con.prefix) + sizeof(con.date)) * smallchar_width,
			lines - smallchar_height * 2, con.prefix, sizeof(con.prefix) - 1 );
		SCR_DrawSmallString( cls.glconfig.vidWidth - sizeof(con.date) * smallchar_width,
			lines - smallchar_height * 2, con.date, sizeof(con.date) - 1 );
	}

	// draw the text
	con.vislines = lines;
	rows = lines / smallchar_width - 1;	// rows of text to draw

	y = lines - (smallchar_height * 3);

	row = con.display;

	// draw from the bottom up
	if ( con.display != con.current )
	{
		// draw arrows to show the buffer is backscrolled
		re.SetColor( g_color_table[ ColorIndex( COLOR_CYAN ) ] );
		for ( x = 0 ; x < con.linewidth ; x += 4 )
			SCR_DrawSmallChar( con.xadjust + (x+1)*smallchar_width, y, '^' );
		y -= smallchar_height;
		row--;
	}

#ifdef USE_CURL
	if ( download.progress[ 0 ] )
	{
		currentColorIndex = ColorIndex( COLOR_CYAN );
		re.SetColor( g_color_table[ currentColorIndex ] );

		i = strlen( download.progress );
		for ( x = 0 ; x < i ; x++ )
		{
			SCR_DrawSmallChar( ( x + 1 ) * smallchar_width,
				lines - smallchar_height, download.progress[x] );
		}
	}
#endif

	currentColorIndex = ColorIndex( COLOR_WHITE );
	re.SetColor( g_color_table[ currentColorIndex ] );

	for ( i = 0 ; i < rows ; i++, y -= smallchar_height, row-- )
	{
		if ( row < 0 )
			break;

		if ( con.current - row >= con.totallines ) {
			// past scrollback wrap point
			continue;
		}

		text = con.text + (row % con.totallines) * con.linewidth;

		for ( x = 0 ; x < con.linewidth ; x++ ) {
			// skip rendering whitespace
			if ( ( text[x] & 0xff ) == ' ' ) {
				continue;
			}
			// track color changes
			colorIndex = ( text[ x ] >> 8 ) & 63;
			if ( currentColorIndex != colorIndex ) {
				currentColorIndex = colorIndex;
				re.SetColor( g_color_table[ colorIndex ] );
			}
			SCR_DrawSmallChar( con.xadjust + (x + 1) * smallchar_width, y, text[x] & 0xff );
		}
	}

	// draw the input prompt, user text, and cursor if desired
	Con_DrawInput();

	re.SetColor( NULL );
}


/*
==================
Con_DrawConsole
==================
*/
void Con_DrawConsole( void ) {

	// check for console width changes from a vid mode change
	Con_CheckResize();

	// if disconnected, render console full screen
	if ( cls.state == CA_DISCONNECTED ) {
		if ( !( Key_GetCatcher( ) & (KEYCATCH_UI | KEYCATCH_CGAME)) ) {
			Con_DrawSolidConsole( 1.0 );
			return;
		}
	}

	if ( con.displayFrac ) {
		Con_DrawSolidConsole( con.displayFrac );
	} else {
		// draw notify lines
		if ( cls.state == CA_ACTIVE ) {
			Con_DrawNotify();
		}
	}
}

//================================================================

/*
==================
Con_RunConsole

Scroll it up or down
==================
*/
void Con_RunConsole( void )
{
	// decide on the destination height of the console
	if ( Key_GetCatcher( ) & KEYCATCH_CONSOLE )
		con.finalFrac = con.userFrac;	// based on con_height* cvars, different with modifier keys
	else
		con.finalFrac = 0.0;	// none visible

	// scroll towards the destination height
	if ( con.finalFrac < con.displayFrac )
	{
		con.displayFrac -= con_conspeed->value * cls.realFrametime * 0.001;
		if ( con.finalFrac > con.displayFrac )
			con.displayFrac = con.finalFrac;

	}
	else if ( con.finalFrac > con.displayFrac )
	{
		con.displayFrac += con_conspeed->value * cls.realFrametime * 0.001;
		if ( con.finalFrac < con.displayFrac )
			con.displayFrac = con.finalFrac;
	}
}


void Con_PageUp( int lines )
{
	if ( lines == 0 )
		lines = con.vispage - 2;

	con.display -= lines;

	Con_Fixup();
}


void Con_PageDown( int lines )
{
	if ( lines == 0 )
		lines = con.vispage - 2;

	con.display += lines;

	Con_Fixup();
}


void Con_Top( void )
{
	// this is generally incorrect but will be adjusted in Con_Fixup()
	con.display = con.current - con.totallines;

	Con_Fixup();
}


void Con_Bottom( void )
{
	con.display = con.current;

	Con_Fixup();
}


void Con_Close( void )
{
	if ( !com_cl_running->integer )
		return;

	Field_Clear( &g_consoleField );
	Con_ClearNotify();
	Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_CONSOLE );
	con.finalFrac = 0.0;			// none visible
	con.displayFrac = 0.0;
}
