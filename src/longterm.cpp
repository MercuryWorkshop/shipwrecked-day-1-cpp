#include "longterm.h"
void LongtermDatabase::step() {
  if (connection != CONNECTION_CONNECTED) {
    return;
  }
  Status status = guthrie_async_read(state);
  if (status == Status::STATUS_EXIT)
    connection = CONNECTION_DISCONECTED;
  if (status != Status::STATUS_PACKET_AVAILABLE)
    return;

  error_str = {};
  affirmation = {};
  UniversalPacket *packet = guthrie_parse_packet(state);
  switch (packet->payload_case) {
  case UNIVERSAL_PACKET__PAYLOAD_ERROR: {
    printf("Error: %s\n", packet->error->error);
    error_str = std::string(packet->error->error);
    break;
  }
  case UNIVERSAL_PACKET__PAYLOAD_MSG: {
    parse_inbound_message(packet);
    break;
  }
  case UNIVERSAL_PACKET__PAYLOAD_AFFIRM: {
    affirmation = packet->affirm->type;
    break;
  }
  default: {
    // client doesnt handle these messages do nothing...
  }
  }
}
