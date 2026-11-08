#ifndef GUN_UTILS_H
#define GUN_UTILS_H

#include <stdbool.h>

typedef enum { CMD_AIM, CMD_SHOOT } CommandType;

typedef struct {
  CommandType type;
  int pitch;
  int yaw;
  int duration_ms;
} GunCommand;

/**
 * @brief Decodes a water gun command received from the Jetson.
 *
 * Supported formats:
 * - `<pitch10;yaw-20>`
 * - `<shoot750>`
 *
 * Commands may end with an optional LF or CRLF line ending.
 *
 * @param[in] input Null-terminated command string.
 * @param[out] cmd Decoded command, updated only on success.
 *
 * @return true if the command is valid, false otherwise.
 */
bool decode_command(const char *input, GunCommand *cmd);

/**
 * @brief Converts a numeric string into an integer.
 *
 * @param[in] text Null-terminated numeric string.
 * @param[out] value Parsed integer.
 *
 * @return true on success, false on invalid input or overflow.
 */
bool parse_integer(const char *text, int *value);

/**
 * @brief Parses an integer until a delimiter.
 *
 * @param[in,out] cursor Current position in the string.
 * @param[in] delimiter Expected delimiter.
 * @param[out] value Parsed integer.
 *
 * @return true on success, false otherwise.
 */
bool parse_until(const char **cursor, char delimiter, int *value);

#endif
