#include "../include/constants.h"

#include <stdint.h>      // Fixed-width integer types
#include <stdio.h>       // Standard I/O
#include <stdlib.h>      // General utilities
#include <time.h>        // It's in your clocks

int get_current_time(char *time_str,size_t buff_size)
{
  static time_t now;
  now = time(NULL);

  /* Format time */
  strftime(time_str, buff_size, "%a %d %b %H:%M:%S", localtime(&now));

  return EXIT_SUCCESS;
}

int build_json(char *time_str, char *json)
{
  snprintf(json, LINE_BUFFER_SIZE, "{"
         "\"name\":\"clock\","
         "\"background\":\"%s\","
         "\"color\":\"#000000\","
         "\"full_text\":\" %s\""
         "%s"
         "}"
         , SOLAR_BASE1, time_str, COMMON_JSON);

  return EXIT_SUCCESS;
}

int generate_time_date(char *time_str)
{
  static char time_date[100];

  /* Generate the current time code */
  get_current_time(&*time_date, sizeof(time_date));

  /* Put the formatted time code into the json */
  build_json(time_date, &*time_str);

  return EXIT_SUCCESS;
}
