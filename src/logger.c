

#include "nvml.h"
#include <pthread.h>


#include "logger.h"

// Build the set of device features

static void get_device_features(struct GPU_device* dev)
{
  if(nvmlDeviceGetTemperature(dev->handle, NVML_TEMPERATURE_GPU,
                              &dev->temperature) == NVML_SUCCESS) {
    dev->feature_support |= GPU_TEMPERATURE;
  }

  if(nvmlDeviceGetMemoryInfo(dev->handle, &dev->memory) == NVML_SUCCESS) {
    dev->feature_support |= GPU_MEMORY_INFO;
  }

  if(nvmlDeviceGetPowerUsage(dev->handle, &dev->power_usage) == NVML_SUCCESS) {
    dev->feature_support |= GPU_POWER_USAGE;
  }


  if(nvmlDeviceGetUtilizationRates(dev->handle, &dev->util) == NVML_SUCCESS) {
    dev->feature_support |= GPU_UTILIZATION_INFO;
  }
}

static void init_device_info( GPU_monitor* mon, int ndevices, int *devices)
{
  gethostname(mon->hostname, 64);
  nvmlInit();
  char *log_path = getenv("GPU_LOG_FILE_PATH");

  NVML_TRY(nvmlSystemGetDriverVersion(mon->driver_version,
                                      sizeof(mon->driver_version)));
  NVML_TRY(nvmlSystemGetNVMLVersion(mon->nvml_version,
                                    sizeof(mon->nvml_version)));
  if(ndevices<=0)
      NVML_TRY(nvmlDeviceGetCount(&mon->dev_count));
  else mon->dev_count=ndevices;
  mon->devices = malloc(mon->dev_count * sizeof(struct GPU_device));
  for(unsigned i = 0; i < mon->dev_count; ++i) {
    struct GPU_device dev;
    memset(&dev, 0, sizeof(struct GPU_device));
    if(ndevices<=0)
      dev.index = i;
    else
      dev.index = devices[i];
    if(log_path == NULL)
      sprintf(dev.file_name, "%s/%d_GPU_%d.log",  GPU_LOG_FILE_PATH, getpid(), dev.index);
    else
      sprintf(dev.file_name, "%s/%d_GPU_%d.log", log_path,  getpid(), dev.index);
    printf("File name %s\n", dev.file_name);
    dev.old_utilization= -1;
    NVML_TRY(nvmlDeviceGetHandleByIndex(dev.index, &dev.handle));

    NVML_TRY(nvmlDeviceGetName(dev.handle, dev.name, sizeof(dev.name)));
    dev.log_file = fopen(dev.file_name, "w");
    printf("GPU name %s\n", dev.name);
    //NVML_TRY(nvmlDeviceGetSerial(dev.handle, dev.serial, sizeof(dev.serial)));
    NVML_TRY(nvmlDeviceGetUUID(dev.handle, dev.uuid, sizeof(dev.uuid)));
    fprintf(dev.log_file,"#%s dev num %d\n", dev.name, dev.index);

    get_device_features(&dev);
    fprintf(dev.log_file, "Timestep, Temperature, Memory Usage, Device Utiliziation, Memory Utiliztion, Power\n");
    mon->devices[i] = dev;
  }

  mon->last_update = getMicrotime();
}

static void update_device_info(GPU_monitor* mon)
{
  // TODO: NVML is thread safe, and the order we grab GPU information
  // here doesn't particularly matter, so might as well take advantage
  // of parallelism here.

  unsigned i;

  for(i = 0; i < mon->dev_count; ++i) {
    struct GPU_device* dev = &mon->devices[i];

    if(dev->feature_support & GPU_MEMORY_INFO) {
      NVML_TRY(nvmlDeviceGetMemoryInfo(dev->handle, &dev->memory));
    }

    if(dev->feature_support & GPU_TEMPERATURE) {
      NVML_TRY(nvmlDeviceGetTemperature(dev->handle, NVML_TEMPERATURE_GPU,
                                        &dev->temperature));
    }

    if(dev->feature_support & GPU_POWER_USAGE) {
      NVML_TRY(nvmlDeviceGetPowerUsage(dev->handle, &dev->power_usage));
    }

  if(nvmlDeviceGetUtilizationRates(dev->handle, &dev->util) == NVML_SUCCESS) {
    dev->feature_support |= GPU_UTILIZATION_INFO;
  }


  }

  mon->last_update = getMicrotime();
}

GPU_monitor* monitor_new()
{
  struct GPU_monitor* mon = malloc(sizeof(struct GPU_monitor));

  // Half a second is a decent default


  return mon;
}

int init_monitor(GPU_monitor *mon, int ndevices, int *devices)
{
  mon->update_interval = 500;

  mon->active = 10;

  // Fetch the basic info that should hold true throughout execution
  init_device_info(mon, ndevices, devices );

   return 0;
}

static inline void write_to_file(GPU_monitor *mon)
{
  unsigned  i;
  for(i = 0; i < mon->dev_count; ++i) {
    struct GPU_device* dev = &mon->devices[i];
    double memory = (double)dev->memory.used/(double)(dev->memory.total)*100.0;
    if(dev->old_utilization != dev->util.gpu )
      fprintf(dev->log_file,"%ld, %d, %f, %d, %d, %d\n",mon->last_update, dev->temperature, memory,\
    dev->util.gpu, dev->util.memory, dev->power_usage);
    dev->old_utilization = dev->util.gpu;
    }
}
static inline void monitor_start_thread(void *arg)
{
GPU_monitor* mon = (GPU_monitor*) arg;
  while(mon->active) {
    update_device_info(mon);
    write_to_file(mon);
    usleep(mon->update_interval * 1000);
  }
  pthread_exit(NULL);

}

void monitor_start(GPU_monitor* mon) {

 mon->active = 10;
pthread_create( &mon->log_thread,
                    NULL,
                    &monitor_start_thread,
                    (void*) mon);

}
void monitor_set_updatetime(GPU_monitor* mon, unsigned milliseconds){

  mon->update_interval= milliseconds;

}


void monitor_stop( GPU_monitor* mon) {

  mon->active= 0;
  pthread_join(mon->log_thread, NULL);

}
void monitor_destroy( GPU_monitor* mon)
{
  for(unsigned i = 0; i < mon->dev_count; ++i) {
    struct GPU_device* dev = &mon->devices[i];
     fclose(dev->log_file);

  }

  free(mon->devices);
  free(mon);
}


const char* nvml_error_code_string(nvmlReturn_t ret)
{
  switch(ret) {
  case NVML_SUCCESS:
    return "The operation was successful";
  case NVML_ERROR_UNINITIALIZED:
    return "was not first initialized with nvmlInit()";
  case NVML_ERROR_INVALID_ARGUMENT:
    return "A supplied argument is invalid";
  case NVML_ERROR_NOT_SUPPORTED:
    return "The requested operation is not available on target device";
  case NVML_ERROR_NO_PERMISSION:
    return "The current user does not have permission for operation";
  case NVML_ERROR_ALREADY_INITIALIZED:
    return"Deprecated: Multiple initializations are now allowed through ref counting";
  case NVML_ERROR_NOT_FOUND:
    return "A query to find an object was unsuccessful";
  case NVML_ERROR_INSUFFICIENT_SIZE:
    return "An input argument is not large enough";
  case NVML_ERROR_INSUFFICIENT_POWER:
    return "A device’s external power cables are not properly attached";
  case NVML_ERROR_DRIVER_NOT_LOADED:
    return "NVIDIA driver is not loaded";
  case NVML_ERROR_TIMEOUT:
    return "User provided timeout passed";
  case NVML_ERROR_IRQ_ISSUE:
    return "NVIDIA Kernel detected an interrupt issue with a GPU";
  case NVML_ERROR_LIBRARY_NOT_FOUND:
    return "NVML Shared Library couldn’t be found or loaded";
  case NVML_ERROR_FUNCTION_NOT_FOUND:
    return"Local version of NVML doesn’t implement this function";
  case NVML_ERROR_CORRUPTED_INFOROM:
    return "infoROM is corrupted";
  case NVML_ERROR_GPU_IS_LOST:
    return "The GPU has fallen off the bus or has otherwise become inaccessible.";
  case NVML_ERROR_RESET_REQUIRED:
    return "The GPU requires a reset before it can be used again.";
  case NVML_ERROR_OPERATING_SYSTEM:
    return "The GPU control device has been blocked by the operating system/cgroups.";
  case NVML_ERROR_LIB_RM_VERSION_MISMATCH:
    return "RM detects a driver/library version mismatch.";
  case NVML_ERROR_IN_USE:
    return "An operation cannot be performed because the GPU is currently in use.";
  case NVML_ERROR_MEMORY:
      return "Insufficient memory.";
  case NVML_ERROR_VGPU_ECC_NOT_SUPPORTED:
      return "The requested vgpu operation is not available on target device, becasue ECC is enabled.";
  case NVML_ERROR_NO_DATA:
    return "No Data";
  case NVML_ERROR_UNKNOWN:
    return "An internal driver error occurred";
  };
  return "Unknown Error!";

}
