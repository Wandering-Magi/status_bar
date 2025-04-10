#define _GNU_SOURCE

#include "../include/constants.h"
#include "../include/helper.h"

#include <dirent.h>      // POSIX directory handling
#include <errno.h>       // Error numbers
#include <inttypes.h>   // For string interpolation of uint64_t integers
#include <linux/limits.h> // Path-related constants
#include <stdint.h>      // Fixed-width integer types
#include <stdio.h>       // Standard I/O
#include <stdlib.h>      // General utilities
#include <string.h>      // String manipulation
#include <sys/stat.h>   // File status
#include <unistd.h>     // POSIX API

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

/* A circle buffer for tracking average speeds over the last interval in seconds */
int insert_into_circle_buffer(uint64_t *buffer, size_t buffer_size, int *pointer, uint64_t *new_number)
{
  if(!buffer || !pointer || !new_number || buffer_size <= 0) goto err_fail;

  buffer[*pointer] = *new_number;
  /*
  for(int i = 0; i < buffer_size; i++){
    printf("%d, %ld | ", i, buffer[i]);
  }
  printf("\n");
  */
  (*pointer)++;
  if(*pointer == buffer_size) *pointer = 0;

  return EXIT_SUCCESS;

err_fail:
  return EXIT_FAILURE;
}

int calc_average_speed(uint64_t *buffer, float buffer_size, float *average)
{
  if(!buffer || !average || buffer_size <= 0) goto err_fail;
  
  uint64_t sum = 0;
  for(int i = 0; i < buffer_size; i++){
    sum += buffer[i] * 8;
  }
  *average = (float)sum / buffer_size;
  return EXIT_SUCCESS;

err_fail:
  return EXIT_FAILURE;
}

/* Convert the float value speeds into K/M/Gbps strings*/
char build_speed_string(float *bytes, char *out_str, size_t max_len)
{
  if(!bytes || !out_str) goto err_fail;
  snprintf(out_str, max_len, "%.0f bps", *bytes);
  
  if(*bytes > 1024.0f) {
    *bytes = *bytes / 1024.0f;
    snprintf(out_str, max_len, "%.2f Kbps", *bytes);
  }

  if(*bytes > 1024.0f) {
    *bytes = *bytes / 1024.0f;
    snprintf(out_str, max_len, "%.2f Mbps", *bytes);
  }

  if(*bytes > 1024.0f) {
    *bytes = *bytes / 1024.0f;
    snprintf(out_str, max_len, "%.2f Gbps", *bytes);
  }

  return EXIT_SUCCESS;
err_fail:
  return EXIT_FAILURE;
}

/* The main networking function that calls everything else */
int get_network_traffic(char *network_json, char *previous_color)
{
  #define BUFFER_SIZE 5 /* In seconds */
  #define RT_MAX_LEN 12
  static char device[16];
  static uint64_t rx_current, rx_previous, rx_change, rx_buffer[BUFFER_SIZE];
  static uint64_t tx_current, tx_previous, tx_change, tx_buffer[BUFFER_SIZE];
  static int rx_point = 0, tx_point = 0;
  static float rx_average, tx_average;
  static char rx_string[RT_MAX_LEN], tx_string[RT_MAX_LEN];
  static char network_string[LINE_BUFFER_SIZE];
  static char *network_color = SOLAR_BLUE;

  /* Figrue out the current default network device. */
  if (find_default_network_device(device) == EXIT_FAILURE) goto err_fail;

  /* A network device has been found, get the current rx and tx values */
  if (get_up_down_speed(device, &rx_current, &tx_current) == EXIT_FAILURE) goto err_fail;

  /* Subtract the previous values from the current values */
  rx_change = rx_current - rx_previous;
  rx_previous = rx_current;
  tx_change = tx_current - tx_previous;
  tx_previous = tx_current;
  
  /* Load change values into a circle buffer for future average calculation */
  if (insert_into_circle_buffer(&*rx_buffer, BUFFER_SIZE, &rx_point, &rx_change) == EXIT_FAILURE) goto err_fail;
  if (insert_into_circle_buffer(&*tx_buffer, BUFFER_SIZE, &tx_point, &tx_change) == EXIT_FAILURE) goto err_fail;

  /* Calculate the average speed over BUFFER_SIZE seconds */
  if (calc_average_speed(rx_buffer, BUFFER_SIZE, &rx_average) == EXIT_FAILURE) goto err_fail;
  if (calc_average_speed(tx_buffer, BUFFER_SIZE, &tx_average) == EXIT_FAILURE) goto err_fail;

  /* Generate a string in the format of ###.## Xbps and write it into their respective buffers*/
  if (build_speed_string(&rx_average, rx_string, RT_MAX_LEN) == EXIT_FAILURE) goto err_fail;
  if (build_speed_string(&tx_average, tx_string, RT_MAX_LEN) == EXIT_FAILURE) goto err_fail;

  /* Build the full text for the network string */
  snprintf(network_string, sizeof(network_string), "   %s  %s", rx_string, tx_string);

  /* Build the json and write it into the network_json buffer */
  if (gen_status_json(previous_color,
                  network_color,
                  "network_data",
                  "#ffffff",
                  network_string,
                  &*network_json) == EXIT_SUCCESS) {
    strcpy(previous_color, network_color);
  } else {
    goto err_fail;
  };
  
  previous_color = network_color;
  return EXIT_SUCCESS;

err_fail:
  return EXIT_FAILURE;
}
