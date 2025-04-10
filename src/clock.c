#include "../include/constants.h"
#include "../include/helper.h"

#include <stdint.h>      // Fixed-width integer types
#include <stdio.h>       // Standard I/O
#include <stdlib.h>      // General utilities
#include <string.h>
#include <time.h>        // It's in your clocks

int get_current_time(char *time_str,size_t buff_size)
{
  static time_t now;
  now = time(NULL);

  /* Format time */
  strftime(time_str, buff_size, " %a %d %b %H:%M:%S", localtime(&now));

  return EXIT_SUCCESS;
}

int generate_time_date(char *time_str, char *prev_color)
{
  static char *time_color = SOLAR_BASE1;
  static char time_date[100];

  /* Generate the current time code */
  if (get_current_time(&*time_date, sizeof(time_date)) == EXIT_FAILURE) {
    goto err_fail;
  };

  /* Put the formatted time code into the json */
  if (gen_status_json(prev_color,
                  time_color,
                  "timedate",
                  "#000000",
                  time_date,
                  &*time_str) == EXIT_SUCCESS) {
    strcpy(prev_color, time_color);
  } else {
    goto err_fail;
  };

  return EXIT_SUCCESS;
err_fail:
  return EXIT_FAILURE;
}
