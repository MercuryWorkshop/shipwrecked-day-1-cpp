#include "SDL3_image/SDL_image.h"
#include <SDL3/SDL_events.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "main.h"
#include "sdl_clay.h"
#include <guthrie.h>
#include <stdio.h>
#include <stdlib.h>

static inline Clay_Dimensions SDL_MeasureText(Clay_StringSlice text,
                                              Clay_TextElementConfig *config,
                                              void *userData) {
  TTF_Font **fonts = (TTF_Font **)userData;
  TTF_Font *font = fonts[config->fontId];
  int width, height;

  TTF_SetFontSize(font, config->fontSize);
  if (!TTF_GetStringSize(font, text.chars, text.length, &width, &height)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to measure text: %s",
                 SDL_GetError());
  }

  return (Clay_Dimensions){(float)width, (float)height};
}

void HandleClayErrors(Clay_ErrorData errorData) {
  printf("%s", errorData.errorText.chars);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("uMessage", "1.0", "com.foxmoss.umessage");

  ProgState *state = new ProgState;
  *appstate = state;
  state->width = 480;
  state->height = 480;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("uMessage", state->width, state->height,
                                   SDL_WINDOW_RESIZABLE, &state->window,
                                   &state->renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_StartTextInput(state->window);

  if (!TTF_Init()) {
    SDL_Log("Couldn't initialize TTF: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  state->lato_regular = TTF_OpenFont(PUBLIC_FOLDER "Lato-Bold.ttf", 14);
  if (state->lato_regular == NULL) {
    SDL_Log("Couldn't initialize font: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  TTF_SetFontWrapAlignment(state->lato_regular, TTF_HORIZONTAL_ALIGN_CENTER);

  state->lato_regular_big = TTF_OpenFont(PUBLIC_FOLDER "Lato-Bold.ttf", 14);
  if (state->lato_regular_big == NULL) {
    SDL_Log("Couldn't initialize font: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  TTF_SetFontWrapAlignment(state->lato_regular_big,
                           TTF_HORIZONTAL_ALIGN_CENTER);

  SDL_GetWindowSize(state->window, &state->width, &state->height);

  state->render_data.renderer = state->renderer;
  state->render_data.fonts = (TTF_Font **)SDL_malloc(2 * sizeof(TTF_Font *));
  state->render_data.textEngine = TTF_CreateRendererTextEngine(state->renderer);
  state->render_data.fonts[0] = state->lato_regular;
  state->render_data.fonts[1] = state->lato_regular_big;
  state->app = new App(state);

  state->texture_array.push_back(
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "foxmoss-pfp.png"));
  state->texture_array.push_back(
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "icons/plus.png"));
  state->texture_array.push_back(
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "icons/profile.png"));
  state->texture_array.push_back(
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "icons/up.png"));
  state->texture_array.push_back(
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "icons/down.png"));
  state->texture_array.push_back(
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "icons/up_50.png"));
  state->texture_array.push_back(
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "icons/down_50.png"));
  state->texture_array.push_back(
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "icons/x.png"));
  state->texture_array.push_back(
      IMG_LoadTexture(state->renderer, PUBLIC_FOLDER "icons/home.png"));

  size_t totalMemorySize = Clay_MinMemorySize();
  Clay_Arena clayMemory = (Clay_Arena){
      .capacity = totalMemorySize,
      .memory = (char *)SDL_malloc(totalMemorySize),
  };
  Clay_Initialize(clayMemory,
                  (Clay_Dimensions){(float)state->width, (float)state->height},
                  (Clay_ErrorHandler){HandleClayErrors});
  Clay_SetMeasureTextFunction(SDL_MeasureText, state->render_data.fonts);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  ProgState *state = (ProgState *)appstate;

  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_WINDOW_RESIZED:
    SDL_GetWindowSize(state->window, &state->width, &state->height);
    SDL_UpdateWindowSurface(state->window);
    Clay_SetLayoutDimensions((Clay_Dimensions){(float)event->window.data1,
                                               (float)event->window.data2});
    break;
  case SDL_EVENT_MOUSE_MOTION:
    Clay_SetPointerState((Clay_Vector2){event->motion.x, event->motion.y},
                         event->motion.state & SDL_BUTTON_LMASK);
    break;
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    Clay_SetPointerState((Clay_Vector2){event->button.x, event->button.y},
                         event->button.button == SDL_BUTTON_LEFT);
    if (event->button.button == SDL_BUTTON_LEFT) {
      state->app->update_mouse(true);
    }
    break;
  case SDL_EVENT_MOUSE_BUTTON_UP:
    if (event->button.button == SDL_BUTTON_LEFT) {
      state->app->update_mouse(false);
    }
    break;
  case SDL_EVENT_MOUSE_WHEEL:
    Clay_UpdateScrollContainers(
        true, (Clay_Vector2){event->wheel.x, event->wheel.y}, 0.01f);
    break;
  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_TEXT_INPUT:
  case SDL_EVENT_TEXT_EDITING:
    state->app->input_event(event);
    break;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  ProgState *state = (ProgState *)appstate;

  SDL_SetRenderDrawColor(state->renderer, BACKGROUND_COLOR, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(state->renderer);

  state->app->draw();

  static Uint64 accu = 0;
  static Uint64 last = 0;
  static Uint64 past = 0;
  Uint64 now = SDL_GetTicksNS();
  Uint64 dt_ns = now - past;
  static char debug_string[32];

  if (now - last > 999999999) {
    last = now;
    SDL_snprintf(debug_string, sizeof(debug_string), "%" SDL_PRIu64 " fps",
                 accu);
    accu = 0;
  }
  past = now;
  accu += 1;
  Uint64 elapsed = SDL_GetTicksNS() - now;
  if (elapsed < 999999) {
    SDL_DelayNS(999999 - elapsed);
  }

  SDL_RenderDebugText(state->renderer, 0, 0, debug_string);

  SDL_RenderPresent(state->renderer);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  ProgState *state = (ProgState *)appstate;

  if (state != NULL) {
    delete state->app;

    if (state->render_data.renderer)
      SDL_DestroyRenderer(state->render_data.renderer);

    if (state->window)
      SDL_DestroyWindow(state->window);

    if (state->render_data.fonts) {
      TTF_CloseFont(state->lato_regular);
      TTF_CloseFont(state->lato_regular_big);

      SDL_free(state->render_data.fonts);
    }

    for (auto texture : state->texture_array) {
      SDL_DestroyTexture(texture);
    }
    state->texture_array.clear();

    if (state->render_data.textEngine)
      TTF_DestroyRendererTextEngine(state->render_data.textEngine);

    delete state;
  }

  TTF_Quit();
}
