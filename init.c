/*
 * Hocoslamfy, initialisation code file
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
#include <SDL/SDL_image.h>

#include "main.h"
#include "init.h"
#include "audio.h"
#include "platform.h"
#include "title.h"

static const char* BackgroundImageNames[BG_LAYER_COUNT] = {
	"Sky.png",
	"Mountains.png",
	"Clouds3.png",
	"Clouds2.png",
	"Clouds1.png",
	"Grass3.png",
	"Grass2.png",
	"Grass1.png"
};

static const char* TitleScreenFrameNames[TITLE_FRAME_COUNT] = {
	"TitleHeader1.png",
	"TitleHeader2.png",
	"TitleHeader3.png",
	"TitleHeader4.png",
	"TitleHeader5.png",
	"TitleHeader6.png",
	"TitleHeader7.png",
	"TitleHeader8.png"
};

static SDL_Surface* LoadImage(const char* Path)
{
	char path[256];
	snprintf(path, 256, DATA_PATH "%s", Path);
	return IMG_Load(path);
}

static bool CheckImage(bool* Continue, bool* Error, const SDL_Surface* Image, const char* Name)
{
	if (Image == NULL)
	{
		*Continue = false;  *Error = true;
		printf("%s: LoadImage failed: %s\n", Name, IMG_GetError());
		return false;
	}
	else
	{
		printf("Successfully loaded %s\n", Name);
		return true;
	}
}

static SDL_Surface* ConvertSurface(bool* Continue, bool* Error, SDL_Surface* Source, const char* Name)
{
	SDL_Surface* Dest;
	if (Source->format->Amask != 0)
		Dest = SDL_DisplayFormatAlpha(Source);
	else
		Dest = SDL_DisplayFormat(Source);
	if (Dest == NULL)
	{
		*Continue = false;  *Error = true;
		printf("%s: SDL_ConvertSurface failed: %s\n", Name, SDL_GetError());
		SDL_ClearError();
		return NULL;
	}
	else
	{
		printf("Successfully converted %s to the screen's pixel format\n", Name);
		SDL_FreeSurface(Source);
		return Dest;
	}
}

SDL_Surface* ActualScreen = NULL;
static SDL_Joystick* Stick = NULL;

void Initialize(bool* Continue, bool* Error)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
	{
		*Continue = false;  *Error = true;
		printf("SDL initialisation failed: %s\n", SDL_GetError());
		SDL_ClearError();
		return;
	} else printf("SDL initialisation succeeded\n");

	if (SDL_NumJoysticks() > 0) {
        	Stick = SDL_JoystickOpen(0);
        	if (Stick) {
            		printf("Joystick 0 successfully opened\n");
        	}
    	}

	SDL_Surface* WindowIcon = LoadImage("hocoslamfy.png");
	if (!CheckImage(Continue, Error, WindowIcon, "hocoslamfy.png"))
		return;
	SDL_WM_SetIcon(WindowIcon, NULL);
	SDL_WM_SetCaption("hocoslamfy", "hocoslamfy");


	ActualScreen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    
	if (ActualScreen == NULL) {
		*Continue = false;  *Error = true;
		printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
		SDL_ClearError();
		return;
	}

	Screen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	Screen = SDL_DisplayFormat(Screen);

	SDL_ShowCursor(0);

	uint32_t i;
	for (i = 0; i < BG_LAYER_COUNT; i++)
	{
		BackgroundImages[i] = LoadImage(BackgroundImageNames[i]);
		if (!CheckImage(Continue, Error, BackgroundImages[i], BackgroundImageNames[i]))
			return;
		if ((BackgroundImages[i] = ConvertSurface(Continue, Error, BackgroundImages[i], BackgroundImageNames[i])) == NULL)
			return;
	}

	for (i = 0; i < TITLE_FRAME_COUNT; i++)
	{
		TitleScreenFrames[i] = LoadImage(TitleScreenFrameNames[i]);
		if (!CheckImage(Continue, Error, TitleScreenFrames[i], TitleScreenFrameNames[i]))
			return;
		if ((TitleScreenFrames[i] = ConvertSurface(Continue, Error, TitleScreenFrames[i], TitleScreenFrameNames[i])) == NULL)
			return;
	}

	CharacterFrames = LoadImage("Bee.png");
	if (!CheckImage(Continue, Error, CharacterFrames, "Bee.png"))
		return;
	if ((CharacterFrames = ConvertSurface(Continue, Error, CharacterFrames, "Bee.png")) == NULL)
		return;
	CollisionImage = LoadImage("Crash.png");
	if (!CheckImage(Continue, Error, CollisionImage, "Crash.png"))
		return;
	if ((CollisionImage = ConvertSurface(Continue, Error, CollisionImage, "Crash.png")) == NULL)
		return;
	ColumnImage = LoadImage("Bamboo.png");
	if (!CheckImage(Continue, Error, ColumnImage, "Bamboo.png"))
		return;
	if ((ColumnImage = ConvertSurface(Continue, Error, ColumnImage, "Bamboo.png")) == NULL)
		return;
	GameOverFrame = LoadImage("GameOverHeader.png");
	if (!CheckImage(Continue, Error, GameOverFrame, "GameOverHeader.png"))
		return;
	if ((GameOverFrame = ConvertSurface(Continue, Error, GameOverFrame, "GameOverHeader.png")) == NULL)
		return;

	InitializePlatform();
	if (!InitializeAudio())
	{
		*Continue = false;  *Error = true;
		return;
	}
	else
		StartBGM();

	// Title screen. (-> title.c)
	ToTitleScreen();
}

void Finalize()
{
	uint32_t i;
	StopBGM();
	FinalizeAudio();
	for (i = 0; i < BG_LAYER_COUNT; i++)
	{
		SDL_FreeSurface(BackgroundImages[i]);
		BackgroundImages[i] = NULL;
	}
	for (i = 0; i < TITLE_FRAME_COUNT; i++)
	{
		SDL_FreeSurface(TitleScreenFrames[i]);
		TitleScreenFrames[i] = NULL;
	}
	SDL_FreeSurface(CharacterFrames);
	CharacterFrames = NULL;
	SDL_FreeSurface(ColumnImage);
	ColumnImage = NULL;
	SDL_FreeSurface(GameOverFrame);
	GameOverFrame = NULL;
	if (Stick != NULL) {
        	SDL_JoystickClose(Stick);
        	Stick = NULL;
    	}
	SDL_Quit();
}

void ToggleFullscreen(void)
{
    static bool isFullscreen = false;
    isFullscreen = !isFullscreen;

    Uint32 flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
    int width = SCREEN_WIDTH;   // 320
    int height = SCREEN_HEIGHT; // 240

    if (isFullscreen) {
        flags |= SDL_FULLSCREEN;
        width = 640;
        height = 480;
    }

    ActualScreen = SDL_SetVideoMode(width, height, 32, flags);
    
    if (ActualScreen == NULL) {
        printf("Mode change error: %s\n", SDL_GetError());
    }
}