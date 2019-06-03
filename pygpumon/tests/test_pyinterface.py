import GPUMon
import time
mon = GPUMon.PyGPUMon()
mon.init(0)
mon.start()
time.sleep(10)
mon.stop()
