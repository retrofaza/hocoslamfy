/*
 * Hocoslamfy, main program file
 * Copyright (C) 2014 Nebuleon Fumika <nebuleon@gcw-zero.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdbool.h>
#include <stddef.h>

#include <SDL/SDL.h>

#include "main.h"
#include "init.h"
#include "platform.h"
#include "text.h"
#include <SDL/SDL_image.h>

#ifdef __AROS__
#include <workbench/startup.h>
#include <proto/icon.h>
#include <proto/dos.h>
#endif

int __nostdiowin = 1;

static bool         Continue                             = true;
static bool         Error                                = false;

       SDL_Surface* Screen                               = NULL;
       SDL_Surface* TitleScreenFrames[TITLE_FRAME_COUNT] = { NULL };
       SDL_Surface* BackgroundImages[BG_LAYER_COUNT]     = { NULL };
       SDL_Surface* CharacterFrames                      = NULL;
       SDL_Surface* ColumnImage                          = NULL;
       SDL_Surface* CollisionImage                       = NULL;
       SDL_Surface* GameOverFrame                        = NULL;

       TGatherInput GatherInput;
       TDoLogic     DoLogic;
       TOutputFrame OutputFrame;

static uint32_t FPS_Count = 0;
static uint32_t FPS_LastTime = 0;
static uint32_t FPS_Current = 0;
static char FPS_Text[20] = "FPS: 0";

#ifdef __AROS__
void CheckAROSTooltypes(int argc, char* argv[])
{
    struct DiskObject* diskObj = NULL;
    char* programName = NULL;

    if (argc == 0) {
        // Uruchomienie z ikony (Workbench)
        struct WBStartup* wbs = (struct WBStartup*)argv;
        if (wbs->sm_NumArgs > 0) {
            BPTR oldDir = CurrentDir(wbs->sm_ArgList[0].wa_Lock);
            diskObj = GetDiskObject(wbs->sm_ArgList[0].wa_Name);
            CurrentDir(oldDir);
        }
    } else {
        // Uruchomienie z CLI - sprawdzamy ikonę pliku binarnego
        diskObj = GetDiskObject(argv[0]);
    }

    if (diskObj) {
        // Szukamy "FPS-NO-LIMIT"
        char* val = FindToolType(diskObj->do_ToolTypes, "FPS-NO-LIMIT");
        if (val) {
            // Jeśli Tooltype istnieje i jest ustawiony na "true" lub "yes"
            if (MatchToolValue(val, "true") || MatchToolValue(val, "yes")) {
                DisableFPSLimit = true;
            }
        }
        FreeDiskObject(diskObj);
    }
}
#endif

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-limit") == 0) {
            DisableFPSLimit = true;
        }
    }

#ifdef __AROS__
    CheckAROSTooltypes(argc, argv);
#endif

    Initialize(&Continue, &Error);
    Uint32 Duration = 16;
    while (Continue)
    {
        GatherInput(&Continue);
        if (!Continue) break;
        
        DoLogic(&Continue, &Error, Duration);
        if (!Continue) break;

	// 1. Rysowanie gry
        OutputFrame(); 

        // 2. OBLICZANIE FPS (raz na sekundę)
        FPS_Count++;
        uint32_t currentTime = SDL_GetTicks();
        if (currentTime - FPS_LastTime >= 1000) {
            FPS_Current = FPS_Count;
            sprintf(FPS_Text, "FPS: %u", FPS_Current);
            FPS_Count = 0;
            FPS_LastTime = currentTime;
        }

	if (ShowFPS) {
            if (SDL_MUSTLOCK(Screen)) SDL_LockSurface(Screen);
            PrintStringOutline32(FPS_Text,
                SDL_MapRGB(Screen->format, 255, 255, 0),
                SDL_MapRGB(Screen->format, 0, 0, 0),
                Screen->pixels, Screen->pitch,
                5, 5, 150, 20, LEFT, TOP);
            if (SDL_MUSTLOCK(Screen)) SDL_UnlockSurface(Screen);
        }

	// 4. Skalowanie i wyświetlanie na fizycznym ekranie
        if (ActualScreen) {
            SDL_SoftStretch(Screen, NULL, ActualScreen, NULL);
            SDL_Flip(ActualScreen);
        }

        Duration = ToNextFrame();
    }
    Finalize();
    return Error ? 1 : 0;
}