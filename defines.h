#ifndef DEFINES_H
#define DEFINES_H

#define PORT 5678
#define BUFFER_SIZE 256
#define INVALID_CHARACTERS "!§$%&/()=?`^°;:_,.-#+*~´`?\"’"
#define KEY_NAME_LENGTH BUFFER_SIZE / 4
#define KEY_VALUE_LENGTH BUFFER_SIZE - KEY_NAME_LENGTH
#define MAX_CAPACITY 10000

constexpr char commandList[] = "HELP - usage: HELP - Sends a list of possible commands - HELP\n"
                               "QUIT - usage: QUIT - Disconnects user from server\n"
                           "PUT - usage: PUT <key> <value> - Puts a key and its value into storage\n"
                           "GET - usage: GET <key> - Get the value of a key\n"
                           "DEL - usage: DEL <key> - Deletes key and its value\n";

#endif //DEFINES_H
