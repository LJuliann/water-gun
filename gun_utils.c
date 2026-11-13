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

// Copies the token before `delimiter` into `buffer` and returns a pointer
// past the delimiter, or NULL if the delimiter is missing, the token is empty
// or does not fit.
static const char *extract_until(const char *current, char delimiter,
                                 char *buffer, size_t size) {
  size_t index = 0;

  // Read characters until delimiter
  while (*current != delimiter && *current != '\0') {
    if (index >= size - 1) {
      return NULL;
    }

    buffer[index++] = *current;
    current++;
  }

  // Check for missing delimiter or empty token
  if (*current != delimiter || index == 0) {
    return NULL;
  }

  // Terminate string
  buffer[index] = '\0';
  return current + 1;
}

bool parse_until(const char **cursor, char delimiter, int *value) {
  if (cursor == NULL || *cursor == NULL || value == NULL) {
    return false;
  }

  char buffer[16];
  const char *next = extract_until(*cursor, delimiter, buffer, sizeof(buffer));
  if (next == NULL) {
    return false;
  }

  // Convert using our existing parser
  int result;
  if (!parse_integer(buffer, &result)) {
    return false;
  }

  // Save result and advance cursor
  *value = result;
  *cursor = next;

  return true;
}

bool parse_tenths(const char *text, int *tenths) {
  if (text == NULL || tenths == NULL) {
    return false;
  }

  int sign = 1;
  if (*text == '-') {
    sign = -1;
    text++;
  }

  // Integer part: 1 to 4 digits (no overflow possible)
  int value = 0;
  int digits = 0;
  while (*text >= '0' && *text <= '9') {
    if (++digits > 4) {
      return false;
    }
    value = value * 10 + (*text - '0');
    text++;
  }
  if (digits == 0) {
    return false;
  }
  value *= 10;

  // Optional single decimal digit
  if (*text == '.') {
    text++;
    if (*text < '0' || *text > '9') {
      return false;
    }
    value += *text - '0';
    text++;
  }

  if (*text != '\0') {
    return false;
  }

  *tenths = sign * value;
  return true;
}

bool parse_tenths_until(const char **cursor, char delimiter, int *tenths) {
  if (cursor == NULL || *cursor == NULL || tenths == NULL) {
    return false;
  }

  char buffer[16];
  const char *next = extract_until(*cursor, delimiter, buffer, sizeof(buffer));
  if (next == NULL) {
    return false;
  }

  int result;
  if (!parse_tenths(buffer, &result)) {
    return false;
  }

  *tenths = result;
  *cursor = next;

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

    // Extract pitch (degrees, at most one decimal)
    if (!parse_tenths_until(&cursor, ';', &temp.pitch_tenths)) {
      return false;
    }

    // Validate yaw prefix
    if (strncmp(cursor, "yaw", 3) != 0) {
      return false;
    }
    cursor += 3;

    // Extract yaw (degrees, at most one decimal)
    if (!parse_tenths_until(&cursor, '>', &temp.yaw_tenths)) {
      return false;
    }

    // Validate angle limits
    if (temp.pitch_tenths < PITCH_MIN_TENTHS ||
        temp.pitch_tenths > PITCH_MAX_TENTHS ||
        temp.yaw_tenths < YAW_MIN_TENTHS || temp.yaw_tenths > YAW_MAX_TENTHS) {
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
