# distutils: sources = ../src/logger.c
# distutils: include_dirs = ../include/ /usr/local/cuda/include/

from libc.stdlib cimport malloc, free
import cython
cimport GPUMon

cdef class PyGPUMon:
    cdef GPUMon.GPU_monitor *_c_monitor

    def __cinit__(self):
        self._c_monitor = monitor_new()

    cdef init_gpulist(self, gpu_list):
        cdef int *my_ints
        my_ints = <int *>malloc(len(gpu_list)*cython.sizeof(int))
        if my_ints is NULL:
            raise MemoryError()

        for i in xrange(len(gpu_list)):
            my_ints[i] = gpu_list[i]
        GPUMon.init_monitor(self._c_monitor, len(gpu_list), my_ints)
        free(my_ints)

    def init(self, gpu_list = None, update_interval=None):
        if(gpu_list == None):
            GPUMon.init_monitor(self._c_monitor, -1, NULL)
        elif  isinstance(gpu_list,list):
            self.init_gpulist(gpu_list)
        elif isinstance(gpu_list, (int,long)):
            self.init_gpulist([gpu_list])
        else:
            print("Please provide list of GPUs as value")
            return
        if isinstance(update_interval, (int,long)):
            GPUMon.monitor_set_updatetime(self._c_monitor, update_interval)

    def start(self):
        GPUMon.monitor_start(self._c_monitor)

    def stop(self):
        GPUMon.monitor_stop(self._c_monitor)

    def __dealloc__(self):
       if self._c_monitor is not NULL:
           GPUMon.monitor_destroy (self._c_monitor)
