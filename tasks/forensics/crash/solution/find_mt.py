#!/usr/bin/env python3

import sys
import struct

u32le = lambda x : struct.unpack("<L", x)[0]

if __name__ == "__main__":
    if len(sys.argv) > 1:
        filename = sys.argv[1]
    else:
        print("{-} Usage: python3 {} <filename>".format(sys.argv[0]))
        sys.exit(1)
    
    buf = open(filename, 'rb').read()

    for i in range(0x236330, len(buf)):
        val = u32le(buf[i:i+4])
        if val > 0 and val < 625 and u32le(buf[i+4:i+8]) != 0:
            if u32le(buf[i+0x9c5:i+0x9c5+4]) == 2147483648:
                if buf[i+0x9c4] == 0:
                    flg = 0

                    for j in range(8, 0x9c4, 8):
                    #    print(hex(i + j), buf[i + j:i + j+4].hex())
                        if u32le(buf[i + j:i + j+4]) == 0:
                            flg = 1
                            break
                    if not flg:
                        print(hex(i))
                        vals = []
                        for k in range(i+4, i+0x9c4, 4):
                            vals.append(u32le(buf[k:k+4]))
                        print(vals)