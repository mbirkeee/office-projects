# System Imports
import socket
import sys
import random

import time
import struct
import select

try:
    import simplejson as json
except:
    import json

MAX_AGE = 80
CMD_LIST_PVS    = 0x4DA3B921
CMD_NETSTAT     = 0x7F201C3B

HEARTBEAT_PORT = 5066

def print_binary(data, prnt=True):

    def print_line(offset, dec, char, prnt=True):
        if len(dec) == 0:
            return

        line_offset = '%08X: ' % offset

        line_dec = ''
        line_char = ''
        for d in dec:
            line_dec += '%02X ' % d

        for c in char:
            line_char += '%c' % c

        pad = 16 - len(dec)

        for _ in xrange(pad):
            line_dec += '   '

        line_total = line_offset + line_dec + '  ' + line_char

        if offset == 0 and prnt:
            print '-' * len(line_total)

        if prnt:
            print line_total
            return

        return line_total

    char = []
    dec = []

    result = []

    offset = 0

    i = 0
    for c in data:
        d = ord(c)
        dec.append(d)
        if d >= 32 and d < 127:
            char.append(c)
        else:
            char.append('.')

        i += 1
        if i == 16:
            result.append(print_line(offset, dec, char, prnt=prnt))
            offset += 16
            i = 0
            char = []
            dec = []

    # print last line
    result.append(print_line(offset, dec, char, prnt=prnt))

    if prnt:
        return

    return result

class Runner(object):

    def __init__(self):
        print "Listening for HEARTBEATS"

    def run(self):

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('', HEARTBEAT_PORT))

        while True:
            data_string, address = s.recvfrom(10000)

            # print address
            # print_binary(data_string)
            ip_address = address[0]
            tcp_port = address[1]

            try:
                data = json.loads(data_string)
            except Exception:
                print_binary(data_string)
                continue


            # print data
            seq = int(data.get('seq'))
            img = data.get('img')
            cwd = data.get('cwd')

            print "IP: %16s -- PORT: %6d SEQ: %8d -- CWD: %s" % \
                  (ip_address, tcp_port, seq, cwd)

            self.netstat(address, seq)

            # data[KEY.ADDRESS] = address
            # HEARTBEATS.beat(data)

    def netstat(self, address, sequence):
                # Send to the originally detected address, NOT the mapped address
        addr = address
        counter = sequence

        # udp_port = addr[1]
        # print "originating UDP PORT: %d" % int(udp_port)
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(('', 0))
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        pvs = []
        offset = 0
        expect = 0

        while True:

            handle = random.randint(0, 100000)

            # Format the message to send
            m = struct.pack("!LLLL", CMD_NETSTAT, counter, offset, handle)

            sock.settimeout(5)
            sock.sendto(m, addr)

            try:
                data_str, rx_addr = sock.recvfrom(100000)

            except Exception, e:
                print "Error getting NETSTAT data", str(e)
                break

            offset += 1

            try:
                data = json.loads(data_str)
            #     # print "checking for handle", handle, int(data['handle'])
            #     if int(data[KEY.HANDLE]) != handle:
            #         print "GOT unexpected handle"
            #         continue


            except:
                print_binary(data_str)
                print "FAILED TO DECODE RESPONSE"
                break

            print data

            try:
                rx_handle = int(data.get('handle', 0))

            except:
                print "Error getting handle from response"
                break

            if rx_handle != handle:
                print "INVALID HANDLE IN RESPONSE!!"
                break

            line = data.get('line')
            if line == "__NONE__":
                print "We are done"
                break

            print "LINE: %s" % line

        print "Done getting netstat data"

            # try:
            #     data = json.loads(data_str)
            #     # print "checking for handle", handle, int(data['handle'])
            #     if int(data[KEY.HANDLE]) != handle:
            #         print "GOT unexpected handle"
            #         continue
            #
            # except Exception, e:
            #     if TEMP_COUNTER >= 10:
            #         continue
            #     else:
            #         print "Exception: bad packet: %s: %s" % (rx_addr, repr(e))
            #
            #         print_binary(data_str)
            #
            #         TEMP_COUNTER += 1
            #         continue







if __name__ == "__main__":

    runner = Runner()
    runner.run()
