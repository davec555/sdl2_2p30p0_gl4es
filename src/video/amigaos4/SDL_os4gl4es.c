/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_VIDEO_DRIVER_AMIGAOS4

#include "SDL_os4video.h"
#include "SDL_os4window.h"
#include "SDL_os4library.h"
#include "SDL_os4opengl.h"
#include "GL/gl.h"

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include "agl.h"

#define DEBUG
#include "../../main/amigaos4/SDL_os4debug.h"


int OS4_GL_LoadLibrary(_THIS, const char * path)
{
    dprintf("Called %d\n", _this->gl_config.driver_loaded);
    dprintf("Opening of libraries done inside of gl4es\n");	

    return 0;
}

void *OS4_GL_GetProcAddress(_THIS, const char * proc)
{
    void *func = NULL;

    dprintf("Called for '%s'\n", proc);

    func = (void *)aglGetProcAddress(proc);

    if (func == NULL) {
        dprintf("Failed to load '%s'\n", proc);
        SDL_SetError("Failed to load function");
    }

    return func;

}



void OS4_GL_UnloadLibrary(_THIS)
{
    dprintf("Called %d\n", _this->gl_config.driver_loaded);
    dprintf("Closing of ogles2 libraries done inside of gl4es\n");
}


SDL_GLContext OS4_GL_CreateContext(_THIS, SDL_Window * window)
{

        //int width, height;
        ULONG errCode = 0;

        SDL_WindowData *data = window->driverdata;

        //OS4_GetWindowActiveSize(window, &width, &height);


        if (data->glContext) {
            dprintf("Old context %p found, deleting\n", data->glContext);

            aglDestroyContext(data->glContext);

            data->glContext = NULL;
        }

        dprintf("Depth buffer size %d, stencil buffer size %d\n",
            _this->gl_config.depth_size, _this->gl_config.stencil_size);


        struct TagItem create_context_taglist[] =
        {
            {OGLES2_CCT_WINDOW, (ULONG)data->syswin},
            {OGLES2_CCT_DEPTH, _this->gl_config.depth_size},
            {OGLES2_CCT_STENCIL, _this->gl_config.stencil_size},
            {OGLES2_CCT_VSYNC, 0},
            {OGLES2_CCT_SINGLE_GET_ERROR_MODE,1},
            {OGLES2_CCT_RESIZE_VIEWPORT,TRUE},
            {TAG_DONE, 0}
        };

        data->glContext = aglCreateContext2(&errCode,create_context_taglist);

        if (data->glContext) {

            dprintf("OpenGL ES 2 context %p created for window '%s'\n",
                data->glContext, window->title);


            aglMakeCurrent(data->glContext);

            // Some games (like q3) doesn't clear the z-buffer prior to use. Since we're using a floating-point depth buffer in warp3dnova,
            // that means it may contain illegal floating-point values, which causes some pixels to fail the depth-test when they shouldn't,
            // so we clear the depth buffer to a constant value when it's first created.
            // Pandora may well use an integer depth-buffer, in which case this can't happen.
            // On MiniGL it didn't happens as there is workaround inside of old warp3d (and probabaly inside of MiniGL itself too).
            // in SDL1 with gl4es (so warp3dnova/ogles2, where no such workaround) it didn't happens probabaly because SDL1 doing something like that (but not glClear).

            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            //glViewport(0, 0, width, height);
            glViewport(0,0,window->w,window->h);
            return data->glContext;

        } else {
            dprintf("Failed to create OpenGL ES 2 context for window '%s' (error code %d)\n",
                window->title, errCode);

            SDL_SetError("Failed to create OpenGL ES 2 context");
            return NULL;
        }

    return NULL;

}

int OS4_GL_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context)
{

    int result = -1;

    if (!window || !context) {
        dprintf("Called window=%p context=%p\n", window, context);
    }

    if (window) {
            SDL_WindowData *data = window->driverdata;

            if (context != data->glContext) {
                dprintf("Context pointer mismatch: %p<>%p\n", context, data->glContext);
                SDL_SetError("Context pointer mismatch");
            } else {
                aglMakeCurrent(context);
            }
    }

    result = 0;

    return result;
}


int OS4_GL_SwapWindow(_THIS, SDL_Window * window)
{
        SDL_WindowData *data = window->driverdata;

        if (data->glContext) {
            SDL_VideoData *videodata = _this->driverdata;


            glFinish();

            if (videodata->vsyncEnabled) {
                IGraphics->WaitTOF();
            }

            // Swap the Buffers!
            aglSwapBuffers();
            return 0;

        } else {
            dprintf("No OpenGL ES 2 context\n");
        }
    return -1;
}



void OS4_GL_DeleteContext(_THIS, SDL_GLContext context)
{
        dprintf("Called with context=%p\n", context);

        if (context) {

            SDL_Window *sdlwin;
            Uint32 deletions = 0;

            for (sdlwin = _this->windows; sdlwin; sdlwin = sdlwin->next) {

                SDL_WindowData *data = sdlwin->driverdata;

                    if (data->glContext == context) { 

                        dprintf("Found ogles2 context, destroy it and clearing window binding\n");

                        aglDestroyContext(context);
                        data->glContext = NULL;
                        deletions++;
                    }
            }

            if (deletions == 0) {
                dprintf("OpenGL ES 2 context doesn't seem to have window binding\n");
                dprintf("So, just delete context without clearing of window binding\n");
                aglDestroyContext(context);
            }
        } else {
            dprintf("No context to delete\n");
        }

}

SDL_bool OS4_GL_ResizeContext(_THIS, SDL_Window * window)
{
    return SDL_TRUE;
}


void OS4_GL_UpdateWindowPointer(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = window->driverdata;

    dprintf("Updating GLES2 window pointer %p\n", data->syswin);

    struct TagItem setparams[] =
    {
        {OGLES2_CCT_WINDOW, (ULONG)data->syswin},
        {TAG_DONE, 0}
    };

    aglSetParams2(setparams);
}


SDL_bool
OS4_GL_AllocateBuffers(_THIS, int width, int height, int depth, SDL_WindowData * data)
{

    return SDL_TRUE;
}

void
OS4_GL_FreeBuffers(_THIS, SDL_WindowData * data)
{
   
} 







void OS4_GL_GetDrawableSize(_THIS, SDL_Window * window, int * w, int * h)
{
    OS4_WaitForResize(_this, window, w, h);
}

int
OS4_GL_SetSwapInterval(_THIS, int interval)
{
    SDL_VideoData *data = _this->driverdata;

    switch (interval) {
        case 0:
        case 1:
            data->vsyncEnabled = interval ? TRUE : FALSE;
            dprintf("VSYNC %d\n", interval);
            return 0;
        default:
            dprintf("Unsupported interval %d\n", interval);
            return -1;
    }
}

int
OS4_GL_GetSwapInterval(_THIS)
{

    SDL_VideoData *data = _this->driverdata;

    return data->vsyncEnabled ? 1 : 0;
}





#endif /* SDL_VIDEO_DRIVER_AMIGAOS4 */

