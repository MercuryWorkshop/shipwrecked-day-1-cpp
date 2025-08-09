#pragma once
#include "SDL3/SDL_render.h"
#include "main.h"
#include <cmath>
#include <cstdio>
#include <vector>

struct Particle {
  float x;
  float y;

  float vel_x;
  float vel_y;
};

#define START_DIST 40
#define MAX_DIST 20

class App {
public:
  App(float width, float height, ProgState *state)
      : width(width), height(height), state(state) {
    for (float x = 0; x < width; x += START_DIST) {
      for (float y = 0; y < height; y += START_DIST) {
        particles.push_back({x, y, 0, 0});
      }
    }
  }
  ~App() {}

  std::vector<Particle> particles;

  void draw() {
    for (auto part : particles) {
      SDL_FRect rect{.x = part.x - 5, .y = part.y - 5, .w = 10, .h = 10};
      SDL_SetRenderDrawColor(state->renderer, PRIMARY_COLOR, SDL_ALPHA_OPAQUE);
      SDL_RenderRect(state->renderer, &rect);
    }
  }
  void step() {

    for (auto &part : particles) {
      part.vel_y += 0.05;

      part.y = std::fmax(0 - part.y, part.y);
      if (part.y > height) {
        part.y += height - part.y;
      }

      part.x += part.vel_x;
      part.y += part.vel_y;
      part.vel_x *= 0.8;
      part.vel_y *= 0.8;
    }
  }

private:
  float width, height;
  ProgState *state;
};
