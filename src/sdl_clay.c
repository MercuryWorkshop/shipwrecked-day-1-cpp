#define SDL_main_h_
#include "renderers/SDL3/clay_renderer_SDL3.c"

void SDL_Clay_RenderClayCommandsProxy(Clay_SDL3RendererData *rendererData,
                                      Clay_RenderCommandArray *rcommands) {
  SDL_Clay_RenderClayCommands(rendererData, rcommands);
}
