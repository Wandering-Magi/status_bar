#include "../include/network.h"
#include "../include/clock.h"
#include "../include/constants.h"

#include <stdarg.h>     // Vardiac functions
#include <stdio.h>       // Standard I/O
#include <stdlib.h>      // General utilities
#include <unistd.h>     // POSIX API
#include <string.h>
#include <unistd.h>     // Turn off buffering

int cat_long_json(char *source_buffer, size_t max_length, int section_count, ...)
{
  static char buffer[LINE_BUFFER_SIZE];
  va_list args;
  va_start (args, section_count);
  
  /* Add the first argument to the source */
  strcat(source_buffer, va_arg(args, const char*));
  for (int i = 0; i < section_count - 1; i++) {
    /* Load the next argument into the local buffer */
    strcpy(buffer, va_arg(args, const char*));
    /* Check to make sure there is enough room in the source for the buffer */
    if (strlen(buffer) + strlen(source_buffer) + 1 < max_length) {
      /* Append a comma first, to prevent trailing commas */
      strcat(source_buffer, ",");
      /* Now append the buffer */
      strcat(source_buffer, buffer);
    } else {
      goto err_fail;
    }
  }
  va_end(args);
  
  return EXIT_SUCCESS;
err_fail:
  va_end(args);
  return EXIT_FAILURE;
}

void build_a_bar()
{
  static char time_json[LINE_BUFFER_SIZE];
  static char network_json[LINE_BUFFER_SIZE];
  static char previous_color[8];
  static char master_json[1024];

  printf(
    "{\"version\":1,\"click_events\":true}\n" /* Specify version and config options */
    "[\n"                                     /* Begin the infinite array */
    "[]\n");                                  /* Prepend a blank array for easy formatting later */

  /* Continually stream status updates */
  while (1) {
    /* The initial bar will come off of the default background */
    strcpy(previous_color, I3_BAR_COLOR);
    /* Start the array with a prepended comma */
    sprintf(master_json, ",[");
    
    get_network_traffic(&*network_json, &*previous_color);
    generate_time_date(&*time_json, &*previous_color);
    
    /* Put everything onto one long json buffer */
    cat_long_json(&*master_json, sizeof(master_json), 
                  2, 
                  network_json, time_json);
    /* End the array */
    strcat(master_json, "]\n");
    /* Send it to the status bar */
    printf("%s", master_json);
    sleep(1);
  }
}

int main() {
  setvbuf(stdout, NULL, _IONBF, 0); // Disable stdout buffering
  
  build_a_bar();
  //network_traffic();
  return EXIT_SUCCESS;
}
