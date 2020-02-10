import serial
import string
import math
import struct
import time
import json
import array
import codecs
import threading
import os
import datetime

# Terminating String which the listener would send after each packet.
serial_terminate = b'\x2A\x0D\x0A'

# Serial Connection.
# To be modified as per the port.
ser = serial.Serial("/dev/tty.usbmodem145201", timeout=None,
                    baudrate=115200, xonxoff=False, rtscts=False, dsrdtr=False)


"""
Method to parse the data
sent by the Device to Host
"""


def junk_data_parser(serial_data):
    # #For Packet ID
    packet_id = '%s' % struct.unpack_from("B", serial_data, 2)
    print("Packet ID - ", packet_id)


thread_list = []
# Attempt using Threading.


class JUNKDataParser(threading.Thread):
    def __init__(self, serial_data):
        super(JUNKDataParser, self).__init__()
        self.serial_data = serial_data

    def run(self):
        junk_data_parser(self.serial_data)


total_number_of_packets = 0
ser.write('0')

while (True):
    print("............................................................................................")
    ser_bytes = ser.read_until(serial_terminate, size=None)

    ser_byte_length = len(ser_bytes)

    if(ser_bytes[0] == b'\x2B'):

        # Junk Data received from Remote Device
        if(ser_bytes[1] == 'A' and ser_byte_length == 932):
            total_number_of_packets = total_number_of_packets + 1
            # Attempt using Threading
            thread = JUNKDataParser(ser_bytes)
            thread_list.append(thread)
            thread.start()

        else:
            print(
                "!!!!!!!!!!!!!!! Received Unknown packet - " + str(ser_bytes[1]) + ", Length - " + str(ser_byte_length) + ", receiving again !!!!!!!!!!!!!!!")

        print("Received Packet Type :-" +
              ser_bytes[1] + " Length :- " + str(ser_byte_length))

    else:
        print("************** Received incomplete packet, receiving again **************")
