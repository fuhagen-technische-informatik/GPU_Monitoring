#include <logger.h>

int main(int *argc, char **argv)
{
    int device =0;
   struct GPU_monitor *monitor = monitor_new();
   init_monitor(monitor, 1, &device);

   monitor_start(monitor);
   sleep(1);
   monitor_stop(monitor);
   sleep(1);
   monitor_start(monitor);
   sleep(1);
   monitor_stop(monitor);
   monitor_destroy(monitor);
  return 0;
}
