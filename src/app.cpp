#include "app.h"
#include "SDL3/SDL_clipboard.h"
#include "main.h"
#include <cstddef>
#include <cstdio>
#include <iterator>

void App::draw() {
  button_indexer = 0;
  small_font_size = state->height / 27;
  large_font_size = state->height / 20;

  switch (layout) {
  case LAYOUT_CONTACTS: {
    layout_contacts();
    break;
  }
  case LAYOUT_MESSAGE: {
    layout_message();
    for (auto pair : input_element_id) {
      Clay_ElementData element = Clay_GetElementData(
          Clay_GetElementIdWithIndex(pair.second, pair.first));

      if (!input_map.contains(pair.first))
        continue;
      if (!input_split_map.contains(pair.first) ||
          input_split_map[pair.first].size() == 0)
        continue;
      if (input_map[pair.first] == "")
        continue;

      std::string target_str = input_map[pair.first];

      TTF_Font *font = state->lato_regular;

      auto line_splits = input_split_map[pair.first];

      size_t line = 0;
      for (auto iter = line_splits.begin() + 1; iter != line_splits.end();
           iter += 1) {

        auto last_iter = iter - 1;

        TTF_Text *text = TTF_CreateText(state->render_data.textEngine, font,
                                        target_str.c_str() + *last_iter.base(),
                                        *iter.base() - *last_iter.base());
        TTF_SetTextColor(text, TEXT_COLOR, 255);
        TTF_DrawRendererText(text, element.boundingBox.x + 16,
                             element.boundingBox.y + 16 +
                                 line * small_font_size);
        TTF_DestroyText(text);
        if (input_cursor_location_map.contains(pair.first)) {
          auto location = input_cursor_location_map[pair.first];
          SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
          SDL_FRect box = {
              .x = (location.x + element.boundingBox.x),
              .y = (location.y + element.boundingBox.y),
              .w = 30,
              .h = (float)small_font_size,
          };
          SDL_RenderRect(state->renderer, &box);
        }
        line++;
      }
    }
    break;
  }
  }
}
void App::input_event(SDL_Event *event) {
  if (!selected.has_value())
    return;
  if (!input_map.contains(selected.value()))
    return;
  if (!input_element_id.contains(selected.value()))
    return;

  if (!input_cursor_location_map.contains(selected.value()))
    input_cursor_index_map[selected.value()] = 0;

  Clay_ElementData element = Clay_GetElementData(Clay_GetElementIdWithIndex(
      input_element_id[selected.value()], selected.value()));

  switch (event->type) {
  case SDL_EVENT_KEY_DOWN:
    switch (event->key.key) {
    case SDLK_BACKSPACE:
      if (input_map[selected.value()].empty())
        break;
      input_map[selected.value()].pop_back();
      break;
    case SDLK_C: {
      if ((event->key.mod & SDL_KMOD_CTRL) == 0)
        break;
      SDL_SetClipboardText(input_map[selected.value()].c_str());
    }
    case SDLK_V: {
      if ((event->key.mod & SDL_KMOD_CTRL) == 0)
        break;
      char *clip = SDL_GetClipboardText();
      input_map[selected.value()] = std::string(clip);
    }
    }
    break;

  case SDL_EVENT_TEXT_INPUT: {
    input_map[selected.value()] += std::string(event->text.text);

    printf("%s\n", input_map[selected.value()].c_str());
    break;
  }
  case SDL_EVENT_TEXT_EDITING:
    printf("edit %s\n", input_map[selected.value()].c_str());
    break;
  }

  size_t real_size = input_map[selected.value()].size();
  for (; real_size > 0; real_size--) {
    if (input_map[selected.value()][real_size] != ' ' &&
        input_map[selected.value()][real_size] != 0) {
      break;
    }
  }

  TTF_Font *font = state->lato_regular;

  size_t cursor = 0;
  std::vector<size_t> line_splits = {0};

  TTF_SetFontSize(font, small_font_size);

  if (real_size != 0) {
    size_t last_cursor = 0;
    while (cursor < real_size + 1) {
      cursor++;
      for (; cursor < real_size + 1; cursor++) {
        int width = 0, height = 0;
        if (!TTF_GetStringSize(
                font, input_map[selected.value()].c_str() + last_cursor,
                cursor - last_cursor, &width, &height))
          continue;

        if (width > element.boundingBox.width - 32 - 16) {
          last_cursor = cursor;
          line_splits.push_back(cursor);
          break;
        }
      }
    }
  }
  line_splits.push_back(real_size + 1);
  input_split_map[selected.value()] = line_splits;
}
