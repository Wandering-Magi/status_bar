#define _GNU_SOURCE
#include <dirent.h>      // POSIX directory handling
#include <errno.h>       // Error numbers
#include <linux/limits.h> // Path-related constants
#include <stdint.h>      // Fixed-width integer types
#include <stdio.h>       // Standard I/O
#include <stdlib.h>      // General utilities
#include <string.h>      // String manipulation
#include <sys/stat.h>   // File status
#include <unistd.h>     // POSIX API

#define I3_BAR_COLOR "#000000"
/* Solarized Color Scheme */
#define SOLAR_BASE03 "#002B36" 
#define SOLAR_BASE02 "#073642" 
#define SOLAR_BASE01 "#586E75" 
#define SOLAR_BASE00 "#657B83" 
#define SOLAR_BASE0 "#839496" 
#define SOLAR_BASE1 "#93A1A1"
#define SOLAR_BASE2 "#EEE8D5"
#define SOLAR_BASE3 "#FDF6E3"
#define SOLAR_YELLOW "#B58900"
#define SOLAR_ORANGE "#CB4B16"
#define SOLAR_RED "#DC322F"
#define SOLAR_MAGENTA "#D33682"
#define SOLAR_VIOLET "#6C71C4"
#define SOLAR_BLUE "#268BD2"
#define SOLAR_CYAN "#2AA198"
#define SOLAR_GREEN "#859900"

#define LINE_BUFFER_SIZE 256
#define BYTE_BUFFER_SIZE 20

/*
* Checks in /proc/net/route for a default network device
* Default devices should route through 00000000
* If there is no network device, it will return nothing.
*/
int find_default_network_device(char *device_name)
{
  #define ROUTE "/proc/net/route"
  char line[LINE_BUFFER_SIZE];
  int i = 0;
  
  FILE *route = fopen(ROUTE, "r");
  if (!route) {
    perror("Failed to open /proc/net/route");
    goto err_fail;
  }

  fgets(line, sizeof(line), route); /*  Skip the headers */

  while (fgets(line, sizeof(line), route) != NULL && i < 100) {
    char iface[16];
    char dest_hex[9];
    
    /* Parse each line for interface and destinaton */
    if (sscanf(line, "%15s %8s", iface, dest_hex) >=2 
      && strcmp(dest_hex, "00000000") == 0 ) {

      strncpy(device_name, iface, strlen(iface) + 1);
      fclose(route);
      return EXIT_SUCCESS;
    }
    i ++;
  }

/* There is no default device */
fclose(route);
err_fail:
  return EXIT_FAILURE;
}

/* Helper function to read the byte value from a file */
uint64_t read_bytes_value(const char *device_name, const char *suffix)
{
  #define DIR "/sys/class/net/"

  /* Calculate the total path size */
  char path[LINE_BUFFER_SIZE];
  /* Construct the full path */
  snprintf(path, LINE_BUFFER_SIZE, "%s%s%s", DIR, device_name, suffix);

  /* Open the file */
  FILE *file = fopen(path, "r");
  if (!file) {
    fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
    goto file_err;
  }

  /* Read the value from the file */
  char buffer[BYTE_BUFFER_SIZE];
  if(!fgets(buffer, sizeof(buffer), file)) {
    perror("Failed to read from file");
    goto file_err;
  }
  fclose(file);

  /* Convert the string value to a long integer */
  return strtoull(buffer, NULL, 10);

file_err:
  fclose(file);
  return -1;
}

/* Function to check the RX and TX values of a given network device */
int get_up_down_speed(char *device_name, uint64_t *rx_byte, uint64_t *tx_byte)
{
  #define RX "/statistics/rx_bytes"
  #define TX "/statistics/tx_bytes"
  
  uint64_t rx_byte_stat = read_bytes_value(device_name, RX);
  uint64_t tx_byte_stat = read_bytes_value(device_name, TX);

  if (rx_byte_stat == -1 || tx_byte_stat == -1) {
    /* fprintf(stderr, "Failed to retrieve network data for device %s\n", device_name); */
    goto err_exit;
  }

  *rx_byte = rx_byte_stat;
  *tx_byte = tx_byte_stat;
  return EXIT_SUCCESS;

err_exit:
  return EXIT_FAILURE;
}

//char *network_usage()
//{
//  
//  char *device_name = "wlan0";
//  //char *device_name = find_default_network_device();
//  if (!device_name) {
//    /* printf("No default network device found, or error occurred.\n"); */
//    goto err_exit;
//  }
//  /*printf("The default network device is %s\n", device_name);*/
//
//  printf("{");
//
//  printf("\"name\": \"network\",");
//  printf("\"background\": \"%s\",", SOLAR_BLUE);
//  printf("\"full_text\":\" %s\",", get_up_down_speed(device_name));
//
//  /*printf("\"color\": #000000,");*/
//  printf("\"color\": #ffffff,");
//
//  printf("}");
//
//  free(device_name);
//  return NULL;
//
//err_exit:
//  free(device_name);
//  return NULL;
//}

int network_math()
{
  return EXIT_FAILURE;
}

int network_traffic()
{
  static char device[16];
  static uint64_t rx_current, rx_previous, rx_change, tx_current, tx_previous, tx_change;

  /* Figrue out the current default network device. */
  if (find_default_network_device(device) == EXIT_FAILURE) {
    /* No default network device routing could be found, bail. */
    goto err_fail;
  }
  /* A network device has bees found, get the current rx and tx values */
  if (get_up_down_speed(device, &rx_current, &tx_current) == EXIT_FAILURE) {
    goto err_fail;
  };

  /* Subtract the previous values from the current values */
  rx_change = rx_current - rx_previous;
  rx_previous = rx_current;
  tx_change = tx_current - tx_previous;
  tx_previous = tx_current;
  printf("RX :%ld TX: %ld\n", rx_change, tx_change);

  return EXIT_SUCCESS;

err_fail:
  return EXIT_FAILURE;
}


void build_a_bar()
{
  printf(
    /* Specify version and config options */
    "{\"version\":1 \"click_events\":true}\n"
    /* Begin the infinite array */
    "[\n"
    /* Prepend a blank array */
    "[]\n");

  /* Continually stream status updates */
  while (1) {
    printf("[");
    //network_usage();
    network_traffic();
    printf("]\n");
    sleep(1);
  }
}

int main() {
  struct timespec time_start = get_time_start();
  
  build_a_bar();
  //network_traffic();

  print_elapsed_time(time_start);
  return EXIT_SUCCESS;
}
