#import cython

cdef extern from "../include/logger.h":
    ctypedef struct GPU_monitor:
        pass

    GPU_monitor* monitor_new()
    void init_monitor(GPU_monitor *mon, int ndevices, int *devices)
    void monitor_start(GPU_monitor* mon)
    void monitor_stop(GPU_monitor* mon)
    void monitor_destroy (GPU_monitor* mon)
    void monitor_set_updatetime(GPU_monitor* mon, unsigned milliseconds)
