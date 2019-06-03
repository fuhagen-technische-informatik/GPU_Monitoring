import signal
import sys
import GPUMon
import time
import click

mon = None
def signal_handler(sig, frame):
        print('Finish recoding')
        mon.stop()
        sys.exit(0)

@click.command()
@click.argument('gpus', nargs=-1, default=None)
@click.option('--update_interval', default=1000)
def start_logger(gpus, update_interval):
    global mon
    mon = GPUMon.PyGPUMon()
    gpu_list = None
    if not gpus is None:
        gpu_list = []
        for i in gpus:
            gpu_list.append(int(i))

    mon.init(gpu_list, update_interval)
    mon.start()
    signal.signal(signal.SIGINT, signal_handler)
    while True:
        time.sleep(100)

if __name__ == '__main__':
    start_logger()
