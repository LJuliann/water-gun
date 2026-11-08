#include "gun_utils.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define COMMAND_SHOOT "<shoot"
#define COMMAND_AIM "<pitch"

bool parse_integer(const char *text, int *value) {
  if (text == NULL || value == NULL) {
    return false;
  }

  int sign = 1;
  int64_t result = 0;

  // Detect negative sign
  if (*text == '-') {
    sign = -1;
    text++;
  }

  // Reject empty strings
  if (*text == '\0') {
    return false;
  }

  // Convert characters to integer
  while (*text != '\0') {
    if (*text < '0' || *text > '9') {
      return false;
    }

    int digit = *text - '0';

    // Detect integer overflow before multiplication
    int64_t limit = (sign == -1) ? -(int64_t)INT_MIN : INT_MAX;

    if (result > (limit - digit) / 10) {
      return false;
    }

    result = result * 10 + digit;
    text++;
  }

  // Save converted value
  *value = (int)(result * sign);

  return true;
}

bool parse_until(const char **cursor, char delimiter, int *value) {
  if (cursor == NULL || *cursor == NULL || value == NULL) {
    return false;
  }

  char buffer[16];
  size_t index = 0;
  const char *current = *cursor;

  // Read characters until delimiter
  while (*current != delimiter && *current != '\0') {
    if (index >= sizeof(buffer) - 1) {
      return false;
    }

    buffer[index++] = *current;
    current++;
  }

  // Check for missing delimiter or empty number
  if (*current != delimiter || index == 0) {
    return false;
  }

  // Terminate string
  buffer[index] = '\0';

  // Convert using our existing parser
  int result;
  if (!parse_integer(buffer, &result)) {
    return false;
  }

  // Save result and advance cursor
  *value = result;
  *cursor = current + 1;

  return true;
}

bool decode_command(const char *input, GunCommand *cmd) {
  if (input == NULL || cmd == NULL) {
    return false;
  }

  GunCommand temp = {0};
  const char *cursor = NULL;

  // Identify command type and skip its prefix
  if (strncmp(input, COMMAND_SHOOT, strlen(COMMAND_SHOOT)) == 0) {
    temp.type = CMD_SHOOT;
    cursor = input + strlen(COMMAND_SHOOT);

  } else if (strncmp(input, COMMAND_AIM, strlen(COMMAND_AIM)) == 0) {
    temp.type = CMD_AIM;
    cursor = input + strlen(COMMAND_AIM);

  } else {
    return false;
  }

  // Decode command parameters
  if (temp.type == CMD_AIM) {

    // Extract pitch
    if (!parse_until(&cursor, ';', &temp.pitch)) {
      return false;
    }

    // Validate yaw prefix
    if (strncmp(cursor, "yaw", 3) != 0) {
      return false;
    }
    cursor += 3;

    // Extract yaw
    if (!parse_until(&cursor, '>', &temp.yaw)) {
      return false;
    }

    // Validate angle limits
    if (temp.pitch < -90 || temp.pitch > 25 || temp.yaw < -90 ||
        temp.yaw > 90) {
      return false;
    }

  } else if (temp.type == CMD_SHOOT) {

    // Extract shooting duration
    if (!parse_until(&cursor, '>', &temp.duration_ms)) {
      return false;
    }

    // Validate duration (milliseconds)
    if (temp.duration_ms < 500 || temp.duration_ms > 2000) {
      return false;
    }
  }

  // Accept optional LF or CRLF line ending
  if (*cursor == '\r' && cursor[1] == '\n') {
    cursor += 2;
  } else if (*cursor == '\n') {
    cursor++;
  }

  // Reject unexpected trailing characters
  if (*cursor != '\0') {
    return false;
  }

  // Save only fully validated commands
  *cmd = temp;
  return true;
}
