#pragma once
#include "SDL3/SDL_render.h"
#include "sdl_clay.h"
#include <cstddef>
extern "C" {
#include <guthrie.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#ifndef EMSCRIPTEN
#define PUBLIC_FOLDER "../public/"
#else
#define PUBLIC_FOLDER "./"
#endif

enum GuthrieAuth {
	UNAUTHED,
	AUTHED
};

struct ProgState {
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  TTF_Font *lato_regular = NULL;
  TTF_Font *lato_regular_tiny = NULL;
  TTF_Font *lato_regular_big = NULL;
  Clay_SDL3RendererData render_data;

  std::vector<SDL_Texture *> texture_array;

  GuthrieState *client = NULL;
  GuthrieAuth auth = GuthrieAuth::UNAUTHED;

  int width;
  int height;
};

#define BACKGROUND_COLOR 251, 251, 254
#define TEXT_COLOR 4, 3, 22
#define PRIMARY_COLOR 110, 98, 172
#define SECONDARY_COLOR 168, 166, 213
#define ACCENT_COLOR 162, 135, 183
