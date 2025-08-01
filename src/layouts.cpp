#include "app.h"
#include "main.h"
#include "sdl_clay.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stddef.h>
#include <string>

App::ButtonState App::clay_input(Clay_ElementDeclaration base) {
  Clay__OpenElement();
  bool hovered = Clay_Hovered();
  input_element_id[button_indexer] = CLAY_STRING("UserTextBox");

  enum ButtonState {
    BUTTON_NONE,
    BUTTON_HOVERED,
    BUTTON_PRESSED,
  } button_state = BUTTON_NONE;

  if (hovered && mouse_state) {
    button_state = BUTTON_PRESSED;
    selected = button_indexer;
  } else if (hovered)
    button_state = BUTTON_HOVERED;

  if (button_state == BUTTON_PRESSED &&
      (!selected.has_value() || selected.value() != button_indexer)) {
    // first clicked
  }

  bool just_pressed =
      previous_mouse_state && !mouse_state &&
      (selected.has_value() && selected.value() == button_indexer);

  if (mouse_state && selected.has_value() &&
      selected.value() == button_indexer && !hovered) {
    selected = {};
  }

  Clay_Color background_color = {0, 0, 0, 0};
  Clay_Color border_color = (Clay_Color){ACCENT_COLOR, 255};
  Clay_Color text_color = (Clay_Color){TEXT_COLOR, 255};

  if (button_state != BUTTON_PRESSED && selected.has_value() &&
      selected.value() == button_indexer) {
    border_color = (Clay_Color){TEXT_COLOR, 255};
  }

  base.id = CLAY_IDI("UserTextBox", button_indexer);
  base.backgroundColor = background_color;
  base.cornerRadius = {10, 10, 10, 10};
  base.border = {
      .color = border_color,
      .width = {2, 2, 2, 2},
  };
  base.layout.layoutDirection = CLAY_TOP_TO_BOTTOM;

  Clay__ConfigureOpenElement(
      CLAY__CONFIG_WRAPPER(Clay_ElementDeclaration, base));

  if ((!selected.has_value() || selected.value() != button_indexer) &&
      !input_split_map.contains(button_indexer)) {
    CLAY_TEXT(CLAY_STRING("Select me to start typing."),
              CLAY_TEXT_CONFIG({
                  .textColor = {TEXT_COLOR, 50},
                  .fontId = 0,
                  .fontSize = small_font_size,
              }));
  } else {
    if (input_map[button_indexer].size() == 0) {
      CLAY({
          .id = CLAY_IDI("TextBuffer", button_indexer),
          .layout =
              {
                  .sizing = {CLAY_SIZING_GROW(0),
                             CLAY_SIZING_FIXED((float)small_font_size + 8)},
              },
      }) {}
    } else {
      CLAY({
          .id = CLAY_IDI("TextBuffer", button_indexer),
          .layout =
              {
                  .sizing = {CLAY_SIZING_GROW(0),
                             CLAY_SIZING_FIXED(
                                 (float)small_font_size *
                                     (input_split_map[button_indexer].size() -
                                      1) +
                                 8)},
              },
      }) {}
    }
  }

  button_indexer++;
  return {text_color, just_pressed};
}

App::ButtonState App::clay_button(Clay_ElementDeclaration base) {
  Clay__OpenElement();
  bool hovered = Clay_Hovered();

  enum ButtonState {
    BUTTON_NONE,
    BUTTON_HOVERED,
    BUTTON_PRESSED,
  } button_state = BUTTON_NONE;

  if (hovered && mouse_state)
    button_state = BUTTON_PRESSED;
  else if (hovered)
    button_state = BUTTON_HOVERED;

  bool just_pressed =
      previous_mouse_state && !mouse_state &&
      (selected.has_value() && selected.value() == button_indexer);

  if (mouse_state && selected.has_value() &&
      selected.value() == button_indexer && !hovered)
    selected = {};

  Clay_Color background_color;
  Clay_Color border_color;
  Clay_Color text_color;
  switch (button_state) {
  case BUTTON_NONE:
    text_color = (Clay_Color){TEXT_COLOR, 255};
    background_color = (Clay_Color){0, 0, 0, 0};
    border_color = (Clay_Color){0, 0, 0, 0};
    break;
  case BUTTON_HOVERED:
    text_color = (Clay_Color){TEXT_COLOR, 255};
    background_color = (Clay_Color){SECONDARY_COLOR, 255};
    border_color = (Clay_Color){0, 0, 0, 0};
    break;
  case BUTTON_PRESSED:
    selected = button_indexer;
    text_color = (Clay_Color){BACKGROUND_COLOR, 255};
    background_color = (Clay_Color){PRIMARY_COLOR, 255};
    border_color = (Clay_Color){0, 0, 0, 0};
    break;
  }

  if (button_state != BUTTON_PRESSED && selected.has_value() &&
      selected.value() == button_indexer) {
    border_color = (Clay_Color){TEXT_COLOR, 255};
  }

  base.id = CLAY_IDI("Button", button_indexer);
  base.backgroundColor = background_color;
  base.cornerRadius = {10, 10, 10, 10};
  base.border = {
      .color = border_color,
      .width = {2, 2, 2, 2},

  };

  Clay__ConfigureOpenElement(
      CLAY__CONFIG_WRAPPER(Clay_ElementDeclaration, base));

  button_indexer++;
  return {text_color, just_pressed};
}

App::ButtonState App::side_bar_icon(size_t texture_index, float row_size) {
  App::ButtonState button_state = clay_button(
      {.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(row_size)},
                  .padding = CLAY_PADDING_ALL(10),
                  .childGap = 10,
                  .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                     .y = CLAY_ALIGN_Y_CENTER},
                  .layoutDirection = CLAY_LEFT_TO_RIGHT}});

  CLAY({
      .id = CLAY_IDI("Icon", button_indexer),
      .layout =
          {
              .sizing = {CLAY_SIZING_PERCENT(1), CLAY_SIZING_GROW(0)},
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
          },
      .aspectRatio = {1},
      .image =
          {
              .imageData = state->texture_array[texture_index],
          },

  }) {}

  Clay__CloseElement();
  return button_state;
}

void App::layout_contacts() {
  float row_size = (float)state->height / 6;
  Clay_BeginLayout();
  CLAY({.id = CLAY_ID("OuterContainer"),
        .layout =
            {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        .backgroundColor = {BACKGROUND_COLOR, 255}}) {

    CLAY({.id = CLAY_ID("Title"),
          .layout =
              {
                  .sizing = {CLAY_SIZING_GROW(0), CLAY__SIZING_TYPE_FIT},
                  .padding = CLAY_PADDING_ALL(5),
                  .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
              },
          .border = {
              .color = {ACCENT_COLOR, 255},
              .width = {.bottom = 1},
          }}) {

      CLAY_TEXT(CLAY_STRING("Messages"),
                CLAY_TEXT_CONFIG({
                    .textColor = {TEXT_COLOR, 255},
                    .fontId = 1,
                    .fontSize = large_font_size,
                    .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                }));
    }
    CLAY({
        .id = CLAY_ID("Content"),
        .layout =
            {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
    }) {

      CLAY({
          .id = CLAY_ID("SideBar"),
          .layout =
              {
                  .sizing = {CLAY_SIZING_PERCENT(0.2), CLAY_SIZING_GROW(0)},
                  .padding = CLAY_PADDING_ALL(16),
                  .childGap = 16,
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
              },
      }) {
        side_bar_icon(1, row_size);
        side_bar_icon(2, row_size);
        side_bar_icon(5, row_size);
        side_bar_icon(4, row_size);
      }

      CLAY({
          .id = CLAY_ID("Body"),
          .layout =
              {
                  .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_PERCENT(1)},
                  .padding = {0, 0, 16, 0},
                  .childGap = 16,
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
              },
          .clip = {.vertical = true},
      }) {

        auto contacts = longterm_db.get_contacts();
        auto contact = contacts.begin();

        for (int i = 0; i < 4; i++) {
          if (contact == contacts.end()) {
            CLAY({.id = CLAY_IDI("BlankRow", i),
                  .layout = {.sizing = {CLAY_SIZING_GROW(0),
                                        CLAY_SIZING_FIXED(row_size)},
                             .padding = CLAY_PADDING_ALL(10),
                             .childGap = 10,
                             .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                .y = CLAY_ALIGN_Y_CENTER},
                             .layoutDirection = CLAY_LEFT_TO_RIGHT}}) {}

            continue;
          }

          App::ButtonState button_state = clay_button(
              {.layout = {
                   .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(row_size)},
                   .padding = CLAY_PADDING_ALL(10),
                   .childGap = 10,
                   .childAlignment = {.x = CLAY_ALIGN_X_LEFT,
                                      .y = CLAY_ALIGN_Y_CENTER},
                   .layoutDirection = CLAY_LEFT_TO_RIGHT}});

          if (button_state.pressed) {
            switch_layout(LAYOUT_MESSAGE);
          }

          CLAY({
              .id = CLAY_IDI("Profile", i),
              .layout =
                  {
                      .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                  },
              .aspectRatio = {1},
              .image =
                  {
                      .imageData = state->texture_array[0],
                  },

          }) {}
          CLAY({
              .id = CLAY_IDI("TextContent", i),
              .layout =
                  {
                      .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                  },
          }) {
            Clay_String username_str = {
                .isStaticallyAllocated = false,
                .length = (int32_t)contact->second.contact_name.size(),
                .chars =
                    longterm_db.get_allocated(contact->second.contact_name),
            };

            CLAY_TEXT(username_str, CLAY_TEXT_CONFIG({
                                        .textColor = button_state.text_color,
                                        .fontId = 0,
                                        .fontSize = small_font_size,
                                        .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                                    }));
            CLAY_TEXT(CLAY_STRING("Direct Message"),
                      CLAY_TEXT_CONFIG({
                          .textColor = {button_state.text_color.r,
                                        button_state.text_color.g,
                                        button_state.text_color.b, 100},
                          .fontId = 0,
                          .fontSize = small_font_size,
                          .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                      }));
          }
          Clay__CloseElement();

          contact++;
        }
        for (int i = 4; i < 4 + 3; i++) {
          if (contact == contacts.end()) {
            CLAY({.id = CLAY_IDI("BlankRow", i),
                  .layout = {.sizing = {CLAY_SIZING_GROW(0),
                                        CLAY_SIZING_FIXED(row_size)},
                             .padding = CLAY_PADDING_ALL(10),
                             .childGap = 10,
                             .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                                .y = CLAY_ALIGN_Y_CENTER},
                             .layoutDirection = CLAY_LEFT_TO_RIGHT}}) {}

            continue;
          }

          CLAY({.id = CLAY_IDI("ProfileContainer", i),
                .layout = {.sizing = {CLAY_SIZING_GROW(0),
                                      CLAY_SIZING_FIXED(row_size)},
                           .padding = CLAY_PADDING_ALL(10),
                           .childGap = 10,
                           .childAlignment = {.x = CLAY_ALIGN_X_LEFT,
                                              .y = CLAY_ALIGN_Y_CENTER},

                           .layoutDirection = CLAY_LEFT_TO_RIGHT}}) {

            CLAY({
                .id = CLAY_IDI("Profile", i),
                .layout =
                    {
                        .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                    },
                .aspectRatio = {1},
                .image =
                    {
                        .imageData = state->texture_array[0],
                    },

            }) {

              CLAY(
                  {.id = CLAY_IDI("ProfileCover", i),
                   .layout =
                       {
                           .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                           .layoutDirection = CLAY_TOP_TO_BOTTOM,
                       },
                   .backgroundColor = {255, 255, 255, 200}}) {}
            }
            CLAY({
                .id = CLAY_IDI("TextContent", i),
                .layout =
                    {
                        .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
            }) {

              Clay_String username_str = {
                  .isStaticallyAllocated = false,
                  .length = (int32_t)contact->second.contact_name.size(),
                  .chars =
                      longterm_db.get_allocated(contact->second.contact_name),
              };

              CLAY_TEXT(username_str,
                        CLAY_TEXT_CONFIG({
                            .textColor = {TEXT_COLOR, 50},
                            .fontId = 0,
                            .fontSize = small_font_size,
                            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                        }));
              CLAY_TEXT(CLAY_STRING("Direct Message"),
                        CLAY_TEXT_CONFIG({
                            .textColor = {TEXT_COLOR, 25},
                            .fontId = 0,
                            .fontSize = small_font_size,
                            .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                        }));
            }
          }
        }
      }
      CLAY({
          .id = CLAY_ID("DeleteBar"),
          .layout =
              {
                  .sizing = {CLAY_SIZING_PERCENT(0.2), CLAY_SIZING_GROW(0)},
                  .padding = CLAY_PADDING_ALL(16),
                  .childGap = 16,
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
              },
      }) {
        for (size_t i = 0; i < 4; i++) {
          side_bar_icon(7, row_size);
        }
      }
    }
  }

  Clay_RenderCommandArray render_commands = Clay_EndLayout();
  SDL_Clay_RenderClayCommandsProxy(&state->render_data, &render_commands);
}
void App::layout_message() {
  float row_size = (float)state->height / 6;
  Clay_BeginLayout();
  CLAY({.id = CLAY_ID("OuterContainer"),
        .layout =
            {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
            },
        .backgroundColor = {BACKGROUND_COLOR, 255}}) {

    CLAY({.id = CLAY_ID("Title"),
          .layout =
              {
                  .sizing = {CLAY_SIZING_GROW(0), CLAY__SIZING_TYPE_FIT},
                  .padding = CLAY_PADDING_ALL(5),
                  .childAlignment = {.x = CLAY_ALIGN_X_CENTER},
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
              },
          .border = {
              .color = {ACCENT_COLOR, 255},
              .width = {.bottom = 1},
          }}) {

      CLAY_TEXT(CLAY_STRING("FoxMoss"),
                CLAY_TEXT_CONFIG({
                    .textColor = {TEXT_COLOR, 255},
                    .fontId = 1,
                    .fontSize = large_font_size,
                    .textAlignment = CLAY_TEXT_ALIGN_CENTER,
                }));
    }
    CLAY({
        .id = CLAY_ID("Content"),
        .layout =
            {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
            },
    }) {

      CLAY({
          .id = CLAY_ID("SideBar"),
          .layout =
              {
                  .sizing = {CLAY_SIZING_PERCENT(0.2), CLAY_SIZING_GROW(0)},
                  .padding = CLAY_PADDING_ALL(16),
                  .childGap = 16,
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
              },
      }) {
        App::ButtonState home_state = side_bar_icon(8, row_size);
        if (home_state.pressed) {
          switch_layout(LAYOUT_CONTACTS);
          selected = {};
        }
        CLAY({.id = CLAY_ID("BufferIcon"),
              .layout = {
                  .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(row_size)},
                  .padding = CLAY_PADDING_ALL(10),
              }}) {}
        side_bar_icon(3, row_size);
        side_bar_icon(6, row_size);
      }

      CLAY({
          .id = CLAY_ID("Body"),
          .layout =
              {
                  .sizing = {CLAY_SIZING_PERCENT(0.8), CLAY_SIZING_GROW(0)},
                  .layoutDirection = CLAY_TOP_TO_BOTTOM,
              },
          .clip =
              {
                  .horizontal = true,
              },

      }) {
        CLAY({
            .id = CLAY_ID("MessagesContainer"),
            .layout =
                {
                    .sizing = {CLAY_SIZING_PERCENT(1), CLAY_SIZING_GROW(0)},
                    .padding = {16, 16, 16, 16},
                    .childGap = 16,
                    .childAlignment =
                        {
                            .y = CLAY_ALIGN_Y_BOTTOM,
                        },
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,

                },
            .clip =
                {
                    .vertical = true,
                },
        }) {

          CLAY({
              .id = CLAY_ID("MessageInvisible"),
              .layout =
                  {
                      .sizing = {CLAY_SIZING_PERCENT(1), CLAY_SIZING_GROW(0)},
                  },
          }) {}

          for (auto message : longterm_db.get_messages_queue()) {
            CLAY({
                .id = CLAY_IDI("Message", message.local_id),
                .layout =
                    {
                        .sizing = {CLAY_SIZING_PERCENT(1), CLAY_SIZING_FIT(0)},
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
            }) {
              auto contacts = longterm_db.get_contacts();
              std::string username = "Unknown Sender";
              if (contacts.contains(message.sender)) {
                username = contacts[message.sender].contact_name;
              }

              Clay_String username_str = {
                  .isStaticallyAllocated = false,
                  .length = (int32_t)username.size(),
                  .chars = longterm_db.get_allocated(username),
              };
              Clay_String message_str = {
                  .isStaticallyAllocated = false,
                  .length = (int32_t)message.message.size(),
                  .chars = longterm_db.get_allocated(message.message),
              };

              CLAY_TEXT(username_str, CLAY_TEXT_CONFIG({
                                          .textColor = {TEXT_COLOR, 255},
                                          .fontId = 0,
                                          .fontSize = small_font_size,
                                      }));
              CLAY_TEXT(message_str, CLAY_TEXT_CONFIG({
                                         .textColor = {TEXT_COLOR, 100},
                                         .fontId = 0,
                                         .fontSize = small_font_size,
                                     }));
            }
          }
        }
        CLAY({
            .id = CLAY_ID("TextBox"),
            .layout =
                {
                    .sizing = {CLAY_SIZING_PERCENT(1), CLAY_SIZING_FIT(0)},
                    .padding = {16, 16, 16, 16},
                    .childGap = 16,

                },
        }) {
          clay_input({
              .layout{
                  .sizing = {CLAY_SIZING_GROW(0),
                             CLAY_SIZING_GROW((float)small_font_size)},
                  .padding = CLAY_PADDING_ALL(16),
              },
          });
          Clay__CloseElement();
        }
      }
    }
  }

  Clay_RenderCommandArray render_commands = Clay_EndLayout();
  SDL_Clay_RenderClayCommandsProxy(&state->render_data, &render_commands);
}
