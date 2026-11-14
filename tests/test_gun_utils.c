#include "gun_utils.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
  do {                                                                          \
    if (!(condition)) {                                                          \
      fprintf(stderr, "%s:%d: FAIL: %s\n", __FILE__, __LINE__, #condition);        \
      exit(EXIT_FAILURE);                                                       \
    }                                                                           \
  } while (0)

static void test_integer(void) {
  const struct {
    const char *input;
    int expected;
  } valid[] = {{"0", 0}, {"-0", 0}, {"123", 123}, {"-123", -123}, {"007", 7}};

  for (size_t i = 0; i < sizeof(valid) / sizeof(valid[0]); ++i) {
    int value = 777;
    CHECK(parse_integer(valid[i].input, &value));
    CHECK(value == valid[i].expected);
  }

  char limit[32];
  int value = 0;
  snprintf(limit, sizeof(limit), "%d", INT_MAX);
  CHECK(parse_integer(limit, &value) && value == INT_MAX);
  snprintf(limit, sizeof(limit), "%d", INT_MIN);
  CHECK(parse_integer(limit, &value) && value == INT_MIN);

  const char *invalid[] = {NULL, "", "-", "+1", " 1", "1 ", "1a", "1\n",
                           "999999999999999999999999999999",
                           "-999999999999999999999999999999"};
  for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
    value = 777;
    CHECK(!parse_integer(invalid[i], &value));
    CHECK(value == 777);
  }
  CHECK(!parse_integer("1", NULL));
}

static void test_delimiter(void) {
  const char *cursor = "-20;remaining";
  int value = 777;
  CHECK(parse_until(&cursor, ';', &value));
  CHECK(value == -20 && strcmp(cursor, "remaining") == 0);

  const char *invalid[] = {"", ";tail", "12", "12x;tail", "9999999999999999;"};
  for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
    cursor = invalid[i];
    value = 777;
    CHECK(!parse_until(&cursor, ';', &value));
    CHECK(cursor == invalid[i] && value == 777);
  }
  CHECK(!parse_until(NULL, ';', &value));
  cursor = NULL;
  CHECK(!parse_until(&cursor, ';', &value));
  cursor = "12;";
  CHECK(!parse_until(&cursor, ';', NULL));
}

static void test_tenths(void) {
  const struct {
    const char *input;
    int expected;
  } valid[] = {{"0", 0},       {"-0", 0},     {"13", 130},  {"13.4", 134},
               {"-7.6", -76},  {"-0.5", -5},  {"0.0", 0},   {"75", 750},
               {"9999.9", 99999}};

  for (size_t i = 0; i < sizeof(valid) / sizeof(valid[0]); ++i) {
    int value = 777;
    CHECK(parse_tenths(valid[i].input, &value));
    CHECK(value == valid[i].expected);
  }

  const char *invalid[] = {NULL,    "",      "-",      ".5",    "-.5",
                           "13.",   "13.45", "1.2.3",  "+1",    " 1",
                           "1 ",    "1a",    "--1",    "10000", "1,5",
                           "1\n"};
  for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
    int value = 777;
    CHECK(!parse_tenths(invalid[i], &value));
    CHECK(value == 777);
  }
  CHECK(!parse_tenths("1", NULL));

  const char *cursor = "-7.6;remaining";
  int value = 777;
  CHECK(parse_tenths_until(&cursor, ';', &value));
  CHECK(value == -76 && strcmp(cursor, "remaining") == 0);

  const char *invalid_until[] = {"", ";tail", "1.5", "1.55;tail",
                                 "12345678901234567;"};
  for (size_t i = 0; i < sizeof(invalid_until) / sizeof(invalid_until[0]);
       ++i) {
    cursor = invalid_until[i];
    value = 777;
    CHECK(!parse_tenths_until(&cursor, ';', &value));
    CHECK(cursor == invalid_until[i] && value == 777);
  }
  CHECK(!parse_tenths_until(NULL, ';', &value));
}

static void test_command(void) {
  const struct {
    const char *input;
    GunCommand expected;
  } valid[] = {
      {"<pitch10;yaw-20>", {CMD_AIM, 100, -200, 0}},
      {"<pitch-90;yaw-90>", {CMD_AIM, -900, -900, 0}},
      {"<pitch25;yaw90>", {CMD_AIM, 250, 900, 0}},
      {"<pitch75;yaw90>", {CMD_AIM, 750, 900, 0}},
      {"<pitch0;yaw0>", {CMD_AIM, 0, 0, 0}},
      {"<pitch13.4;yaw-7.6>", {CMD_AIM, 134, -76, 0}},
      {"<pitch-0.5;yaw0.0>", {CMD_AIM, -5, 0, 0}},
      {"<pitch-90.0;yaw90.0>", {CMD_AIM, -900, 900, 0}},
      {"<shoot500>", {CMD_SHOOT, 0, 0, 500}},
      {"<shoot750>", {CMD_SHOOT, 0, 0, 750}},
      {"<shoot2000>", {CMD_SHOOT, 0, 0, 2000}},
  };
  const char *endings[] = {"", "\n", "\r\n"};
  for (size_t i = 0; i < sizeof(valid) / sizeof(valid[0]); ++i) {
    for (size_t j = 0; j < sizeof(endings) / sizeof(endings[0]); ++j) {
      char input[64];
      snprintf(input, sizeof(input), "%s%s", valid[i].input, endings[j]);
      GunCommand command = {CMD_SHOOT, 11, 22, 777};
      CHECK(decode_command(input, &command));
      CHECK(command.type == valid[i].expected.type);
      CHECK(command.pitch_tenths == valid[i].expected.pitch_tenths);
      CHECK(command.yaw_tenths == valid[i].expected.yaw_tenths);
      CHECK(command.duration_ms == valid[i].expected.duration_ms);
    }
  }

  const char *invalid[] = {
      NULL, "", "\n", "<shoot499>", "<shoot2001>\n", "<shoot3000>",
      "<pitch-91;yaw0>", "<pitch75.1;yaw0>\r\n", "<pitch0;yaw-91>",
      "<pitch0;yaw91>", "<pitch-90.1;yaw0>", "<pitch0;yaw90.1>",
      "<pitch13.45;yaw0>", "<pitch13.;yaw0>", "<pitch1,5;yaw0>", "<shoot>", "<pitch10;yaw>", "<pitch10;roll20>",
      "<shoot750", "<shoot750 >", "<shoot750\n>", "<shoot750>extra",
      "<shoot750>\nextra", "<shoot750>\n\n", "<shoot750>\r",
      "<shoot750>\r\nextra", "<shoot750>\r\n\n", "<shoot750>\\n",
      "<shoot999999999999999999999>", "<pitch10;yaw999999999999999999999>",
      "<shoot750>\n<shoot1000>\n",
  };
  for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
    GunCommand command = {CMD_SHOOT, 11, 22, 777};
    CHECK(!decode_command(invalid[i], &command));
    CHECK(command.type == CMD_SHOOT && command.pitch_tenths == 11 &&
          command.yaw_tenths == 22 && command.duration_ms == 777);
  }
  CHECK(!decode_command("<shoot750>", NULL));
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s integer|delimiter|tenths|command\n", argv[0]);
    return EXIT_FAILURE;
  }
  if (strcmp(argv[1], "integer") == 0) {
    test_integer();
  } else if (strcmp(argv[1], "delimiter") == 0) {
    test_delimiter();
  } else if (strcmp(argv[1], "tenths") == 0) {
    test_tenths();
  } else if (strcmp(argv[1], "command") == 0) {
    test_command();
  } else {
    fprintf(stderr, "Unknown suite: %s\n", argv[1]);
    return EXIT_FAILURE;
  }
  printf("PASS: %s\n", argv[1]);
  return EXIT_SUCCESS;
}
