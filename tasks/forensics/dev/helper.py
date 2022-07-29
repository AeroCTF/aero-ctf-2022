import hashlib
import binascii
from msilib.schema import Error
from Crypto.Cipher import AES
import os
import random

def xor_s(s1: str, s2: str) -> str:
    res = ''
    for i in range(len(s1)):
        res += chr(ord(s1[i]) ^ ord(s2[i % len(s2)]))
    return res

h1 = lambda x: hashlib.sha1(x).hexdigest()
h2 = lambda x: hashlib.sha256(x).hexdigest()
h5 = lambda x: hashlib.md5(x).hexdigest()
dh = lambda x: binascii.unhexlify(x)
hd = lambda x: binascii.hexlify(x)

def do_aes(data: bytes) -> bytes:
    if len(data) % 16 != 0:
        data += b'\x00' * (16 - (len(data) % 16))
    key = os.urandom(16)
    iv = os.urandom(16)
    #print(key, iv)
    ctx = AES.new(key, AES.MODE_CBC, iv=iv)
    return ctx.encrypt(data)

def readfile(filename: str) -> bytes:
    buf = b''
    try:
        buf = open(filename, 'rb').read()
    except Error as e:
        print("Error in file read! <{}>".format(e))
    return buf

def do_shuff(data: bytes) -> bytes:
    s = list(data)
    random.seed()
    random.shuffle(s)
    return bytes(s)