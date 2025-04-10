#include "../include/constants.h"

#include <stdio.h>
#include <stdlib.h>      // General utilities

int gen_seperator(char *previous_color, char *current_color, char *buffer)
{
  snprintf(buffer, LINE_BUFFER_SIZE,
           "{"
           "\"full_text\":\"\","
           "\"color\":\"%s\","
           "\"background\":\"%s\""
           "%s"
           "},"
           , current_color, previous_color, COMMON_JSON);
  return EXIT_SUCCESS;
}

int gen_status_json(char *prev_color, 
                    char *current_color, 
                    char *name, 
                    char *font_color, 
                    char *text, 
                    char *buffer)
{
  snprintf(buffer, LINE_BUFFER_SIZE,
           /* Build the seperator icon */
           "{"
           "\"full_text\":\"\","
           "\"background\":\"%s\","
           "\"color\":\"%s\","
           "%s"
           "},"
           /* Build the main bar portion */
           "{"
           "\"name\":\"%s\","
           "\"background\":\"%s\","
           "\"color\":\"%s\","
           "\"full_text\":\"%s\","
           "%s" 
           "}"
           , prev_color, current_color, COMMON_JSON
           , name, current_color, font_color, text, COMMON_JSON);

  return EXIT_SUCCESS;
}
