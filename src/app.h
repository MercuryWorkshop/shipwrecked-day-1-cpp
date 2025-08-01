#pragma once

#include "longterm.h"
#include "sdl_clay.h"
#include <cstddef>
#include <cstdio>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
struct ProgState;

class App {
public:
  App(ProgState *state) : state(state), longterm_db() {
    longterm_db.open_thread({
        .type = LongtermDatabase::Thread::THREAD_DIRECT,
        .id =
            "50b37fb1f57045ce5245642494446daa7b9ec2c06acda810db957aa7f05e0230",
    });
  }
  ~App() {}
  void draw();

  void update_mouse(bool state) {
    previous_mouse_state = mouse_state;
    mouse_state = state;
  }
  void input_event(SDL_Event *event);

  struct ButtonState {
    Clay_Color text_color;
    bool pressed;
  };

private:
  ButtonState clay_button(Clay_ElementDeclaration base);
  ButtonState clay_input(Clay_ElementDeclaration base);
  ButtonState side_bar_icon(size_t texture_index, float row_size);
  void layout_contacts();
  void layout_message();
  uint16_t small_font_size;
  uint16_t large_font_size;

  enum Layout { LAYOUT_CONTACTS, LAYOUT_MESSAGE };
  Layout layout = LAYOUT_CONTACTS;
  void switch_layout(Layout new_layout) {
    layout = new_layout;
    input_map.clear();
    input_element_id.clear();
    selected = {};
  }

  ProgState *state;
  bool mouse_state = false;
  bool previous_mouse_state = false;

  size_t button_indexer = 0;
  std::optional<size_t> selected = {};
  std::unordered_map<size_t, std::string> input_map;
  std::unordered_map<size_t, std::vector<size_t>> input_split_map;
  std::unordered_map<size_t, size_t> input_cursor_index_map;
  std::unordered_map<size_t, Clay_Vector2> input_cursor_location_map;
  std::unordered_map<size_t, Clay_String> input_element_id;

  LongtermDatabase longterm_db;
};
