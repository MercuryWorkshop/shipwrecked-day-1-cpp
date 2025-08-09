#pragma once
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "main.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>

struct Particle {
  float x;
  float y;

  float vel_x;
  float vel_y;

  float decay;

  bool sent;
  int id;
};

#define START_DIST 40
#define MAX_DIST 20

class App {
public:
  App(float width, float height, ProgState *state)
      : width(width), height(height), state(state) {}
  ~App() {}

  std::vector<Particle> particles;
  float last_x = 0;
  float last_y = 0;

  void draw() {

    for (auto part : particles) {
      SDL_FRect rect = {
          .x = part.x,
          .y = part.y,
          .w = std::lerp((float)MAX_DIST * 2, (float)0, part.decay),
          .h = std::lerp((float)MAX_DIST * 2, (float)0, part.decay),
      };
      SDL_RenderTexture(state->renderer, state->texture_array[0], NULL, &rect);
    }
  }

  void event(SDL_Event *event) {
    float dist = std::sqrt(pow(last_x - event->motion.x, 2) +
                           pow(last_y - event->motion.y, 2));

    if (event->motion.state == SDL_BUTTON_LEFT && dist > 50) {
      last_x = event->motion.x;
      last_y = event->motion.y;
      particles.push_back(
          {event->motion.x, event->motion.y, 0, 0, 0, false, rand()});
    }
  }

  void step() {
    for (auto &part : particles) {
      part.vel_x += float(rand() % 11 - 5) / 5;
      part.vel_y += float(rand() % 11 - 5) / 5;

      part.vel_x *= 0.3;
      part.vel_y *= 0.3;
      part.decay += (float)state->delta / 10000000000;

      std::vector<std::vector<Particle>::iterator> iters;
      for (auto iter = particles.begin(); iter != particles.end(); iter++) {
        if (iter->decay > 1) {
          iters.push_back(iter);
        }
      }
      for (auto iter : iters) {
        particles.erase(iter);
      }

      part.x += part.vel_x;
      part.y += part.vel_y;
    }
  }

private:
  float width, height;
  ProgState *state;
};
