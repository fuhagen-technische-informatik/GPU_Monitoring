# GPU_Monitoring

This is a small tool for GPU monitoring
See test_dir/ for example in C

See pygpumon/test for python examples

Install for python in pygpumon
python3 setup.py install --user

##Monitoring Tool
You can use the libray/module or the util_logger.py skript for continous loggin.

usage:
python3 util_logger.py [gpus] [--update_intervall ms]
e.g.
 python3 util_logger.py 0  --update_intervall 250

will check for a new value each 250ms. 

###Logfile
The logfile stores the data in csv format.
Note that a new value is only stored, if the GPU utilization changes

