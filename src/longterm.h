#pragma once
#include "packets.pb-c.h"
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <optional>
#include <queue>
#include <sqlite3.h>
#include <string>
#include <unordered_map>
extern "C" {
#include <guthrie.h>
}

#define MAX_QUEUE_SIZE 50

class LongtermDatabase {
public:
  LongtermDatabase() {
    connect_guthrie();
    if (connection == CONNECTION_CONNECTED) {
      if (guthrie_send_version(state) != 0)
        connection = CONNECTION_DISCONECTED;
    }
    if (connection == CONNECTION_CONNECTED) {
      if (guthrie_send_auth(
              state,
              "bc48c1b3e2454adf0bd8b9c23f2bc125742b7a57377c3f8a36fb36"
              "cb9753c870",
              "6b3a55e0261b0304143f805a24924d0c1c44524821305f31d92778"
              "43b8a10f4e") != 0) // hardcoded rn take input in the future
        connection = CONNECTION_DISCONECTED;
    }

    sqlite3_open("data.db", &db);
    char *accounts_sql = "CREATE TABLE IF NOT EXISTS accounts ("
                         "user_identifier TEXT NOT NULL,"
                         "user_password TEXT NOT NULL,"
                         "PRIMARY KEY (user_identifier)"
                         ")";
    char *error_msg;
    if (sqlite3_exec(db, accounts_sql, NULL, 0, &error_msg) != SQLITE_OK) {
      printf("%s\n", error_msg);
      sqlite3_close(db);
      exit(1);
    }

    char *contacts_sql = "CREATE TABLE IF NOT EXISTS contacts ("
                         "user_identifier TEXT NOT NULL,"
                         "contact_name TEXT NOT NULL,"
                         "PRIMARY KEY (user_identifier)"
                         ")";
    if (sqlite3_exec(db, contacts_sql, NULL, 0, &error_msg) != SQLITE_OK) {
      printf("%s\n", error_msg);
      sqlite3_close(db);
      exit(1);
    }

    char *messages_sql = "CREATE TABLE IF NOT EXISTS messages ("
                         "from_user_identifier TEXT NOT NULL,"
                         "channel_id TEXT,"
                         "message_content TEXT NOT NULL,"
                         "date_created INTEGER NOT NULL,"
                         "message_id INTEGER PRIMARY KEY AUTOINCREMENT"
                         ")";
    if (sqlite3_exec(db, messages_sql, NULL, 0, &error_msg) != SQLITE_OK) {
      printf("%s\n", error_msg);
      sqlite3_close(db);
      exit(1);
    }

    load_contacts();
  }
  ~LongtermDatabase() {
    if (state != NULL) {
      guthrie_exit(state);
    }
    sqlite3_close(db);

    for (auto pair : allocation_map) {
      free(pair.second);
    }
    allocation_map.clear();
  }

  void step();
  std::optional<std::string> get_error() { return error_str; }
  struct Message {
    std::string sender;
    std::string message;
    time_t time;
    sqlite3_int64 local_id;
  };
  struct Thread {
    enum {
      THREAD_DIRECT,
      THREAD_GROUP,
    } type;
    std::string id;

    bool operator==(Thread &b) { return id == b.id && type == b.type; }
  };
  struct Contact {
    std::string user_identifier;
    std::string contact_name;
  };

  std::deque<Message> get_messages_queue() { return message_queue; }
  std::map<std::string, Contact> get_contacts() { return contacts; }

  void close_thread() {
    thread_id = {};
    thread_range = {};

    for (auto pair : allocation_map) {
      free(pair.second);
    }
    allocation_map.clear();
  }

  char *get_allocated(std::string str) {
    if (allocation_map.contains(str))
      return allocation_map[str];
    allocation_map[str] = (char *)malloc(str.size() + 1);
    memcpy(allocation_map[str], str.c_str(), str.size());
    allocation_map[str][str.size()] = 0;
    return allocation_map[str];
  }

  void load_contacts() {
    char *contacts_sql = "SELECT user_identifier, contact_name "
                         "FROM contacts";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, contacts_sql, -1, &stmt, NULL) != SQLITE_OK) {
      printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(1);
    }

    bool read_all_rows = false;
    while (!read_all_rows) {
      switch (sqlite3_step(stmt)) {
      case SQLITE_ROW: {
        const unsigned char *user_identifier = sqlite3_column_text(stmt, 0);
        const unsigned char *contact_name = sqlite3_column_text(stmt, 1);

        std::string user_identifier_str((char *)user_identifier);

        contacts[user_identifier_str] = ((Contact){
            .user_identifier = user_identifier_str,
            .contact_name = std::string((char *)contact_name),
        });
        break;
      }
      default:
        read_all_rows = true;
        break;
      }
    }

    sqlite3_finalize(stmt);
  }

  void open_thread(Thread thread) {
    thread_id = thread;

    const char *selected_sql;
    if (thread.type == Thread::THREAD_DIRECT) {
      selected_sql =
          "SELECT from_user_identifier, channel_id, message_content, "
          "date_created, message_id FROM messages WHERE channel_id IS NULL AND "
          "from_user_identifier = ? ORDER BY message_id DESC LIMIT ?";
    } else {
      selected_sql =
          "SELECT from_user_identifier, channel_id, message_content, "
          "date_created, message_id FROM messages WHERE channel_id IS ? ORDER "
          "BY message_id DESC LIMIT ?";
    }

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, selected_sql, -1, &stmt, NULL) != SQLITE_OK) {
      printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(1);
    }

    sqlite3_bind_text(stmt, 1, thread.id.c_str(), thread.id.size(),
                      SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, MAX_QUEUE_SIZE);

    bool read_all_rows = false;
    while (!read_all_rows) {
      switch (sqlite3_step(stmt)) {
      case SQLITE_ROW: {
        const unsigned char *from_user_identifier =
            sqlite3_column_text(stmt, 0);
        const unsigned char *message_content = sqlite3_column_text(stmt, 2);
        sqlite3_int64 date_created = sqlite3_column_int64(stmt, 3);
        sqlite3_int64 message_id = sqlite3_column_int64(stmt, 4);

        message_queue.push_front((Message){
            .sender = std::string((char *)from_user_identifier),
            .message = std::string((char *)message_content),
            .time = date_created,
            .local_id = message_id,
        });
        if (!thread_range.has_value()) {
          thread_range = IdRange();
          thread_range->newest_id = message_id;
          thread_range->oldest_id = message_id;
        } else {
          if (message_id < thread_range->oldest_id)
            thread_range->oldest_id = message_id;
          if (message_id > thread_range->newest_id)
            thread_range->newest_id = message_id;
        }
        break;
      }
      default:
        read_all_rows = true;
        break;
      }
    }

    sqlite3_finalize(stmt);
  }

private:
  void parse_inbound_message(UniversalPacket *packet) {
    Thread message_thread_id{
        .type = packet->msg->channel_id != NULL
                    ? LongtermDatabase::Thread::THREAD_GROUP
                    : LongtermDatabase::Thread::THREAD_DIRECT,
        .id = packet->msg->channel_id != NULL ? packet->msg->channel_id
                                              : packet->msg->sender_identifier};

    std::string sender(packet->msg->sender_identifier);

    if (!contacts.contains(sender)) {
      std::string contact_name = "Account ";
      contact_name.insert(contact_name.end(), sender.begin(),
                          sender.begin() + 8);
      new_contact({.user_identifier = sender, .contact_name = contact_name});
    }

    Message message = {
        .sender = packet->msg->sender_identifier,
        .message = packet->msg->message,
        .time = time(NULL),
    };
    store_message(message, message_thread_id);
  }

  void store_message(Message message, Thread thread) {
    const char *insert_sql = "INSERT INTO messages (from_user_identifier, "
                             "channel_id, message_content, date_created) "
                             "VALUES (?, ?, ?, strftime('%s', 'now'))";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
      printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(1);
    }

    sqlite3_bind_text(stmt, 1, message.sender.c_str(), -1, SQLITE_TRANSIENT);
    if (thread.type == Thread::THREAD_GROUP) {
      sqlite3_bind_text(stmt, 2, thread_id->id.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_bind_text(stmt, 3, message.message.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      printf("Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_int64 local_id = sqlite3_last_insert_rowid(db);

    message.local_id = local_id;
    sqlite3_finalize(stmt);

    if (thread_id.has_value() && thread == thread_id.value())
      push_message(message);
  }

  void push_message(Message message) {
    message_queue.push_back(message);
    while (message_queue.size() > MAX_QUEUE_SIZE)
      message_queue.pop_front();
    if (thread_range.has_value())
      thread_range->newest_id = message.local_id;
  }

  void connect_guthrie() {
    if (state != NULL) {
      guthrie_exit(state);
      state = NULL;
    }

    auto op = guthrie_init();
    if (op.type == OptionalGuthrieState::TYPE_ERROR) {
      connection = CONNECTION_DISCONECTED;
    } else {
      state = op.data.state;
      connection = CONNECTION_CONNECTED;
    }
  }

  std::unordered_map<std::string, char *> allocation_map;

  struct IdRange {
    size_t oldest_id;
    size_t newest_id;
  };
  std::optional<IdRange> thread_range;
  std::optional<Thread> thread_id;
  std::deque<Message> message_queue; // back is newest front is oldest

  void new_contact(Contact contact) {
    const char *insert_sql =
        "INSERT INTO contacts (user_identifier, contact_name) VALUES (?, ?)";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
      printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(1);
    }

    sqlite3_bind_text(stmt, 1, contact.user_identifier.c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, contact.contact_name.c_str(), -1,
                      SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      printf("Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    contacts[contact.user_identifier] = contact;
  }

  std::map<std::string, Contact> contacts;

  sqlite3 *db;
  GuthrieState *state = NULL;
  std::optional<std::string> error_str;
  std::optional<AffirmationType> affirmation;
  enum DistantConnection {
    CONNECTION_DISCONECTED,
    CONNECTION_CONNECTED
  } connection = CONNECTION_DISCONECTED;
};
