#pragma once
#include "SDL3/SDL_init.h"
#include "main.h"

enum GuthrieAuth { UNAUTHED, AUTHED };
class Guthrie {
  GuthrieState *client = NULL;
  GuthrieAuth auth = GuthrieAuth::UNAUTHED;

public:
  SDL_AppResult init(char *host, int port) {
    OptionalGuthrieState op = guthrie_init(host, port);
    if (op.type == OptionalGuthrieState::Type::TYPE_ERROR) {
      printf("%s\n", op.data.error_str);
      return SDL_APP_FAILURE;
    }
    this->client = op.data.state;
    guthrie_send_version(this->client);
    if (guthrie_send_auth(
            this->client,
            ":3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3",
            ":3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:3:"
            "3") == -1) {
      printf("a\n");
    }

    return SDL_APP_CONTINUE;
  }

  SDL_AppResult loop() {
    enum Status status = guthrie_async_read(this->client);
    if (status == Status::STATUS_PACKET_AVAILABLE) {
      UniversalPacket *packet = guthrie_parse_packet(this->client);
      if (packet->payload_case ==
              UniversalPacket__PayloadCase::UNIVERSAL_PACKET__PAYLOAD_AFFIRM &&
          packet->affirm->type ==
              AffirmationType::AFFIRMATION_TYPE__AFFIRM_LOGIN) {
        this->auth = GuthrieAuth::AUTHED;
        printf("logged in\n");
      }
      if (packet->payload_case ==
          UniversalPacket__PayloadCase::UNIVERSAL_PACKET__PAYLOAD_ERROR)
        printf("error: %s\n", packet->error->error);
      printf("packet\n");
    } else if (status == Status::STATUS_EXIT) {
      return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
  }
};
