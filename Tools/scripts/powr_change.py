#!/usr/bin/env python

"""
Parses a log file and shows how the power flags changed over time

AP_FLAKE8_CLEAN

"""

from __future__ import print_function

import optparse
import sys
import time

from pymavlink import mavutil


class POWRChange(object):
    def __init__(self, master):
        self.master = master

    def progress(self, text):
        '''emit text with possible timestamps etc'''
        print("%u: %s" % (time.time(), text))

    def bit_description(self, bit_number):
        if 1 << bit_number not in mavutil.mavlink.enums["MAV_POWER_STATUS"]:
            return "UNKNOWN_BIT[%u]" % bit_number

        return mavutil.mavlink.enums["MAV_POWER_STATUS"][1 << bit_number].name

    def run(self):

        self.progress("Creating connection")
        self.conn = mavutil.mavlink_connection(master)

        desired_type = "POWR"
        current = None
        while True:
            m = self.conn.recv_match(type=desired_type)
            if m is None:
                break
            if current is None:
                current_flags = 0
            else:
                current_flags = current.Flags
            flags = m.Flags
            if flags == current_flags:
                continue
            line = ""
            for bit in range(0, 32):  # range?
                mask = 1 << bit
                old_bit_set = current_flags & mask
                new_bit_set = flags & mask
                if new_bit_set and not old_bit_set:
                    line += " +%s" % self.bit_description(bit)
                elif not new_bit_set and old_bit_set:
                    line += " -%s" % self.bit_description(bit)

            current = m

            timestamp = getattr(m, '_timestamp', 0.0)
            formatted_timestamp = "%s.%02u" % (
                time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(timestamp)),
                int(timestamp * 100.0) % 100)

            print("%s: %s" % (formatted_timestamp, line))
            current = m


if __name__ == '__main__':
    parser = optparse.OptionParser("powr_change.py [options]")

    (opts, args) = parser.parse_args()

    if len(args) < 1:
        parser.print_help()
        sys.exit(1)

    master = args[0]

    tester = POWRChange(master)
    tester.run()
