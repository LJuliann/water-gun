#ifndef GUN_UTILS_H
#define GUN_UTILS_H

#include <stdbool.h>

typedef enum { CMD_AIM, CMD_SHOOT } CommandType;

// Aiming limits in tenths of a degree (drone frame: pitch+ up, yaw+ left).
// Upper pitch limit: 75 deg, set to aim higher; the servo itself stops at 77.
#define PITCH_MIN_TENTHS (-900)
#define PITCH_MAX_TENTHS 750
#define YAW_MIN_TENTHS (-900)
#define YAW_MAX_TENTHS 900

typedef struct {
  CommandType type;
  int pitch_tenths; // Tenths of a degree: 134 = 13.4 deg
  int yaw_tenths;   // Tenths of a degree: -76 = -7.6 deg
  int duration_ms;
} GunCommand;

/**
 * @brief Decodes a water gun command received from the Jetson.
 *
 * Supported formats:
 * - `<pitch13.4;yaw-7.6>` (angles in degrees, at most one decimal)
 * - `<pitch10;yaw-20>` (whole degrees, still accepted)
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

/**
 * @brief Converts a decimal angle into tenths of a degree.
 *
 * Accepts an optional `-`, 1 to 4 integer digits and at most one decimal
 * digit: `13` -> 130, `13.4` -> 134, `-0.5` -> -5.
 *
 * @param[in] text Null-terminated decimal string.
 * @param[out] tenths Parsed value in tenths, updated only on success.
 *
 * @return true on success, false on invalid input.
 */
bool parse_tenths(const char *text, int *tenths);

/**
 * @brief Parses a decimal angle (see parse_tenths) until a delimiter.
 *
 * @param[in,out] cursor Current position, advanced past the delimiter.
 * @param[in] delimiter Expected delimiter.
 * @param[out] tenths Parsed value in tenths of a degree.
 *
 * @return true on success, false otherwise (cursor and value unchanged).
 */
bool parse_tenths_until(const char **cursor, char delimiter, int *tenths);

#endif
