/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#ifndef _SDL_os4video_h
#define _SDL_os4video_h

#include <exec/types.h>
#include <intuition/intuition.h>
#include <proto/intuition.h> // TODO: cleanup these
#include <proto/graphics.h>
#include <proto/keymap.h>
#include <proto/layers.h>
#include <proto/icon.h>
#include <proto/textclip.h>
#include <proto/input.h>
#include <proto/dos.h>

#include "../SDL_sysvideo.h"

/* Private display data */

typedef struct
{
    STRPTR                  appName;
    uint32                  appId;

    struct Screen          *publicScreen;

    struct MsgPort         *userPort;
    struct MsgPort         *appMsgPort;

    struct MsgPort         *inputPort;
    struct IOStdReq        *inputReq;

    APTR                    pool;

    struct InputIFace       *iInput;

    BOOL                    vsyncEnabled;
} SDL_VideoData;

#define IInput ((SDL_VideoData *) _this->driverdata)->iInput

extern void * OS4_SaveAllocPooled(_THIS, uint32 size);
extern void * OS4_SaveAllocVecPooled(_THIS, uint32 size);
extern void OS4_SaveFreePooled(_THIS, void *mem, uint32 size);
extern void OS4_SaveFreeVecPooled(_THIS, void *mem);

extern DECLSPEC struct MsgPort * OS4_GetSharedMessagePort();

#endif /* _SDL_os4video_h */

/* vi: set ts=4 sw=4 expandtab: */
