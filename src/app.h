#pragma once

#include "sdl_clay.h"
#include <cstddef>
#include <optional>
struct ProgState;

class App {
public:
  App(ProgState *state) : state(state) {}
  ~App() {}
  void draw() {
    button_indexer = 0;
    switch (layout) {
    case LAYOUT_CONTACTS: {
      layout_contacts();
      break;
    }
    case LAYOUT_MESSAGE: {
      layout_message();
      break;
    }
    }
  }

  void update_mouse(bool state) { mouse_state = state; }

  struct ButtonState {
    Clay_Color text_color;
    bool pressed;
  };

private:
  Clay_Color clay_button(Clay_ElementDeclaration base);
  void side_bar_icon(size_t texture_index, float row_size);
  void layout_contacts();
  void layout_message();

  enum State { LAYOUT_CONTACTS, LAYOUT_MESSAGE } layout = LAYOUT_CONTACTS;
  ProgState *state;
  bool mouse_state = false;

  size_t button_indexer = 0;
  std::optional<size_t> selected = {};
};
