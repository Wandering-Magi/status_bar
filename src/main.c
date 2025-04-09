#include "../include/network.h"
#include "../include/clock.h"
#include "../include/constants.h"

#include <stdio.h>       // Standard I/O
#include <stdlib.h>      // General utilities
#include <unistd.h>     // POSIX API
#include <string.h>

void build_a_bar()
{
  static char time_json[LINE_BUFFER_SIZE];
  static char network_json[LINE_BUFFER_SIZE];
  static char previous_color[8];
  static char master_json[2048];
   
  strcpy(time_json, "This is how we do it.\n");

  printf(
    "{\"version\":1 \"click_events\":true}\n" /* Specify version and config options */
    "[\n"                                     /* Begin the infinite array */
    "[]\n");                                  /* Prepend a blank array */

  /* Continually stream status updates */
  while (1) {
    /* The initial bar will come off of the default background */
    strcpy(previous_color, I3_BAR_COLOR);

    printf("[");
    get_network_traffic(&*network_json, &*previous_color);
    printf("%s", network_json);
    /*
    generate_time_date(&*time_json);
    printf("%s", time_json);
    */
    printf("]\n");
    sleep(1);
  }
}

int main() {
  
  build_a_bar();
  //network_traffic();
  return EXIT_SUCCESS;
}
