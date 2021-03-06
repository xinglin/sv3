#!/usr/bin/env python3

import struct
import sys

trace_event_struct = 'QBBHI'

cpu_hz = 1000*int(open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq").read())
event_names = ["BLOCK", "WAKEUP", "PACKET_RX", "PACKET_TX", "WENT_IDLE",
               "IRQ", "QUIESCENT"]

def print_event(time_rel, time, event, port, length, dummy):
    if (time != 0):
        print("%016x +%08dns: %12s port=%03s length=%04x" %
              (time, 1000000000*time_rel/cpu_hz,
               event_names[event],
               port, length))

def main(tracefile, *selected_events):
    events = []
    with open(tracefile, 'rb') as tf:
        while True:
            bytes_read = tf.read(struct.calcsize(trace_event_struct))
            if not bytes_read: break
            event = struct.unpack(trace_event_struct, bytes_read)
            event_name = event_names[event[1]]
            if event[0] != 0 and (len(selected_events) == 0 or event_name in selected_events):
                events.append(event)
    last_time = None
    for e in sorted(events, key = lambda t: t[0]):
        print_event(e[0] - last_time if last_time else 0,
                    *e)
        last_time = e[0]

if __name__ == "__main__":
    main(*sys.argv[1:])
