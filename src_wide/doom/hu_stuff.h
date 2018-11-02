//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:  Head up display
//

// Russian Doom (C) 2016-2018 Julian Nechaevsky


#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "d_event.h"


//
// Globally visible constants.
//
#define HU_FONTSTART    '!' // the first font characters
#define HU_FONTEND      '_' // the last font characters

// [JN] Definitions of small gray font (note: this is not full array)
#define HU_FONTSTART_GRAY    '!' // the first font characters
#define HU_FONTEND_GRAY      ':' // the last font characters
#define HU_FONTSIZE_GRAY     (HU_FONTEND_GRAY - HU_FONTSTART_GRAY + 1)	

// Calculate # of glyphs in font.
#define HU_FONTSIZE     (HU_FONTEND - HU_FONTSTART + 1)	
#define HU_BROADCAST    5
#define HU_MSGX         0
#define HU_MSGY         0
#define HU_MSGWIDTH     64  // in characters
#define HU_MSGHEIGHT    1   // in lines
#define HU_MSGTIMEOUT   (4*TICRATE)


//
// HEADS UP TEXT
//

void HU_Init(void);
void HU_Start(void);

boolean HU_Responder(event_t* ev);

void HU_Ticker(void);
void HU_Drawer(void);
char HU_dequeueChatChar(void);
void HU_Erase(void);

extern char *chat_macros[10];


#endif
