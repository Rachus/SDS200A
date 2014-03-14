#!/usr/bin/env python

import usb.core
import usb.util

def co(dev, request_type, request, value, index, data, timeout):
        toSend = None
        length = 0
        if data is not None:
            toSend = HexToByte(data)
            length = len(toSend)
        assert dev.ctrl_transfer(request_type, request, value, index, toSend, timeout) == length
        time.sleep(0.05)

def ci(dev, request_type, request, value, index, length, timeout):
        val = dev.ctrl_transfer(request_type, request, value, index, length, timeout);
        time.sleep(0.05)
        return val;

