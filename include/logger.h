#ifndef _logger_H
#define _logger_H
#include <nvml.h>

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#define LOG_MAX_FILE_NAME 1024
#define GPU_LOG_FILE_PATH "."
const char* nvml_error_code_string(nvmlReturn_t ret);

// Simple wrapper function to remove boiler plate code of checking
// NVML API return codes.
//
// Returns non-zero on error, 0 otherwise
static inline int nvml_try(nvmlReturn_t ret, const char* fn)
{
  // We ignore the TIMEOUT error, as it simply indicates that
  // no events (errors) were triggered in the given interval.
  if(ret != NVML_SUCCESS && ret != NVML_ERROR_TIMEOUT) {
    fprintf(stderr, "%s: %s: %s\n", fn, nvml_error_code_string(ret),
            nvmlErrorString(ret));
    return 1;
  }

  return 0;
}

#define NVML_TRY(code) nvml_try(code, #code)

void shutdown_nvml(void);

enum feature {
  GPU_TEMPERATURE      = 1 << 0,
  GPU_COMPUTE_MODE     = 1 << 1,
  GPU_POWER_USAGE      = 1 << 2,
  GPU_MEMORY_INFO      = 1 << 3,
  GPU_UTILIZATION_INFO = 1 << 6
};

struct GPU_device {
  unsigned index;
  FILE *log_file;

  nvmlDevice_t handle;


  nvmlMemory_t memory;
  // Current device resource utilization rates (as percentages)
  nvmlUtilization_t util;
  // In Celsius
  unsigned temperature;

  // In milliwatts
  unsigned power_usage;

  // Maximum clock speeds, in MHz


  // Fan speed, percentage

  char name[NVML_DEVICE_NAME_BUFFER_SIZE];
  char serial[NVML_DEVICE_SERIAL_BUFFER_SIZE];
  char uuid[NVML_DEVICE_UUID_BUFFER_SIZE];
  char file_name[LOG_MAX_FILE_NAME];
  // Bitmask of enum feature
  unsigned feature_support;
};

 typedef struct GPU_monitor {
  // How long should we wait before polling the devices again? In milliseconds.
  unsigned update_interval;
  // Whether or not the monitor should continue running
  int active;

  char driver_version[NVML_SYSTEM_DRIVER_VERSION_BUFFER_SIZE];
  char nvml_version[NVML_SYSTEM_NVML_VERSION_BUFFER_SIZE];
  char hostname[64];

  // When we last updated this data
  clock_t last_update;

  unsigned dev_count;
  struct GPU_device* devices;
  pthread_t log_thread;
}GPU_monitor;

GPU_monitor* monitor_new( );
int init_monitor(GPU_monitor *mon, int ndevices, int *devices);
void monitor_start(GPU_monitor* mon);
void monitor_destroy (GPU_monitor* mon);
void monitor_stop(GPU_monitor* mon);
void monitor_set_updatetime(GPU_monitor* mon, unsigned milliseconds);
#endif /* _logger_h*/
