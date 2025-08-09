#pragma once
#include "SDL3/SDL_init.h"
#include "app.hpp"
#include "guthrie.h"
#include "main.h"
#include "packets.pb-c.h"
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

#define STATE_PREFIX "WILLIAM_DANIEL_IS_NEXT"
bool prefix(const char *pre, const char *str) {
  return strncmp(pre, str, strlen(pre)) == 0;
}

bool deserializeVectorOfParticles(const char *serialized_data,
                                  std::vector<Particle> *particles) {
  if (serialized_data == nullptr) {
    std::cerr << "Error: Input serialized_data is nullptr." << std::endl;
    return false;
  }

  std::string s(serialized_data);
  std::stringstream ss(s);
  std::string segment;

  // Read the count first
  if (!std::getline(ss, segment, '|')) {
    std::cerr << "Error: Could not read particle count from serialized data."
              << std::endl;
    return false;
  }

  int count;
  try {
    count = std::stoi(segment);
  } catch (const std::exception &e) {
    std::cerr << "Error: Invalid particle count format: " << e.what()
              << std::endl;
    return false;
  }

  if (count < 0) {
    std::cerr << "Error: Negative particle count encountered." << std::endl;
    return false;
  }

  // If count is 0, we're done.
  if (count == 0) {
    return true;
  }

  // Read remaining particle data
  for (int i = 0; i < count; ++i) {
    if (!std::getline(ss, segment, '|')) {
      std::cerr << "Error: Incomplete serialized data. Expected " << count
                << " particles, but only found " << i << "." << std::endl;
      particles->clear(); // Clean up partially deserialized data
      return false;
    }

    float x, y, decay;
    int id;
    // Use sscanf on the segment to parse x and y
    int items_assigned =
        sscanf(segment.c_str(), "%f,%f,%f,%i", &x, &y, &decay, &id);

    if (items_assigned != 4) {
      std::cerr << "Error: Failed to parse x,y,decay,id for particle " << i + 1
                << ". Segment: \"" << segment << "\"" << std::endl;
      particles->clear(); // Clean up partially deserialized data
      return false;
    }

    particles->push_back(
        {x, y, 0, 0, decay}); // vel_x and vel_y are not restored
  }

  return true; // Deserialization successful
}

enum GuthrieAuth { UNAUTHED, AUTHED };
class Guthrie {
  GuthrieState *client = NULL;
  GuthrieAuth auth = GuthrieAuth::UNAUTHED;
  std::vector<Particle> *particles = NULL;

  std::string sender = "";

public:
  SDL_AppResult init(char *host, int port, std::string sender,
                     std::vector<Particle> *particles) {
    this->sender = sender;
    this->particles = particles;

    OptionalGuthrieState op = guthrie_init(host, port);
    if (op.type == OptionalGuthrieState::Type::TYPE_ERROR) {
      printf("%s\n", op.data.error_str);
      return SDL_APP_FAILURE;
    }
    this->client = op.data.state;
    guthrie_send_version(this->client);
    if (guthrie_send_auth(
            this->client, (char *)this->sender.c_str(),
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
      if (packet->payload_case ==
              UniversalPacket__PayloadCase::UNIVERSAL_PACKET__PAYLOAD_MSG &&
          prefix(STATE_PREFIX, packet->msg->message)) {
        char *message = packet->msg->message + strlen(STATE_PREFIX);
        if (!deserializeVectorOfParticles(message, this->particles)) {
          printf("fail\n");
          return SDL_APP_FAILURE;
        }
      }
      printf("packet\n");
    } else if (status == Status::STATUS_EXIT) {
      return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
  }

  SDL_AppResult send_state() {
    std::stringstream ss;
    ss << STATE_PREFIX;

    size_t size = 0;
    for (auto &p : *this->particles) {
      if (p.sent)
        continue;
      size++;
    }

    ss << size;
    ss << std::fixed << std::setprecision(2);
    for (auto &p : *this->particles) {
      if (p.sent) {
        continue;
      }
      ss << "|" << p.x << "," << p.y << "," << p.decay << "," << p.id;
      p.sent = true;
    }

    std::string s = ss.str();

    char **recips = new char *[] { (char *)this->sender.c_str() };
    guthrie_send_message(this->client, (char *)this->sender.c_str(), NULL,
                         recips, 1, (char *)s.c_str());

    return SDL_APP_CONTINUE;
  }
};
