# Aero CTF 2022 | Reverse | Arctic

# Description

> Please, be careful: this is the Arctic, not the Antarctic!

# Files

- [public/reverse-arctic.tar.gz](public/reverse-arctic.tar.gz)

# Solution

The server implements [Threefish cipher](https://en.wikipedia.org/wiki/Threefish). The algorithm is following:

```c
ThreefishSetKey(&ctx, key, tweak);

for (size_t i = 0; i < 64; i++) {
    ((uint8_t *)plaintext)[i] ^= flag[i];

    ThreefishEncrypt(&ctx, plaintext, plaintext);
    ThreefishEncrypt(&ctx, plaintext + 4, plaintext + 4);
}

int result = 0;

for (size_t i = 0; i < 8; i++) {
    if (plaintext[i] != ciphertext[i]) {
        result = 1;
    }
}
```

It's trivial to understand that `flag` could not be recovered using `plaintext` and `ciphertext`: the algorithm would be exponential. Therefore, the _real_ flag is hidden somewhere else.

Let's try to send something long:

```
$ ./arctic
[*] Hello!
[*] Please, enter the flag:
> AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
[1]    2082054 segmentation fault  ./arctic
```

Here is a buffer overflow. Run checksec:

```
> checksec ./arctic
    Arch:     amd64-64-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE (0x400000)
```

GCC adds a stack canary by default, but the challenge does not contain it. It means that the author have disabled it manually.

Let's think about this: what we can do with buffer overflow? There is an option: if we overwrite `ret` pointer on the stack, the execution would go the another way. So, instead of going to the parent function, we can go somewhere else.

The flag format is `Aero{}`, and the challenge uses `scanf()`. It means that the buffer ends with `}\x00`.

The actual ret is `0x4134bc` (`<main+120>`).

1. If we overwrite 1 byte, it becomes `0x413400`:

```
gef > x/16i 0x413400
   0x413400 <validate+63>:	lock add BYTE PTR [rax],al
   0x413403 <validate+66>:	add    BYTE PTR [rax],al
   0x413405 <validate+68>:	mov    QWORD PTR [rbp-0x8],0x0
   0x41340d <validate+76>:	lea    rdi,[rip+0x7bec]        # 0x41b000
   0x413414 <validate+83>:	mov    eax,0x0
   0x413419 <validate+88>:	call   0x41350f <printf>
   0x41341e <validate+93>:	lea    rax,[rbp-0x40]
   0x413422 <validate+97>:	mov    rsi,rax
   0x413425 <validate+100>:	lea    rdi,[rip+0x7bf3]        # 0x41b01f
   0x41342c <validate+107>:	mov    eax,0x0
   0x413431 <validate+112>:	call   0x413657 <scanf>
   0x413436 <validate+117>:	lea    rax,[rbp-0x40]
   0x41343a <validate+121>:	mov    rdi,rax
   0x41343d <validate+124>:	call   0x410000 <check_flag>
   0x413442 <validate+129>:	leave  
   0x413443 <validate+130>:	ret  
```

Seems not usable.

2. If we overwrite 2 byte, it becomes `0x41007d`:

```
gef > x/16i 0x41007d
   0x41007d <check_flag+125>:	nop
   0x41007e <check_flag+126>:	mov    eax,0x415506
   0x410083 <check_flag+131>:	jmp    rax
   0x410085 <check_flag+133>:	mov    QWORD PTR [rbp-0x198],rax
   0x41008c <check_flag+140>:	movabs rax,0x9aa4a4f5f15e2aa7
   0x410096 <check_flag+150>:	mov    QWORD PTR [rbp-0x190],rax
   0x41009d <check_flag+157>:	movabs rax,0x221d54f140500e8
   0x4100a7 <check_flag+167>:	mov    QWORD PTR [rbp-0x188],rax
   0x4100ae <check_flag+174>:	movabs rax,0x6353c0dddb255b65
   0x4100b8 <check_flag+184>:	mov    QWORD PTR [rbp-0x200],rax
   0x4100bf <check_flag+191>:	movabs rax,0x280638028af8e1cc
   0x4100c9 <check_flag+201>:	mov    QWORD PTR [rbp-0x1f8],rax
   0x4100d0 <check_flag+208>:	movabs rax,0x2df42cc9b357253f
   0x4100da <check_flag+218>:	mov    QWORD PTR [rbp-0x1f0],rax
   0x4100e1 <check_flag+225>:	movabs rax,0x2a352e99849a4330
   0x4100eb <check_flag+235>:	mov    QWORD PTR [rbp-0x1e8],rax
```

Oops! Here is a `jmp`. Look at `0x415506`:

```
gef > x/16i 0x415506
   0x415506:	nop
   0x415507:	mov    eax,0x210
   0x41550c:	jmp    0x415510
   0x41550e:	movabs r14,0x2eb00000200bb90
   0x415518:	movabs r14,0x2ebfd8949909090
   0x415522:	movabs r14,0x2ebc62949fe8949
   0x41552c:	movabs r14,0xe0ff00415229b890
```

And at 

```
gef > x/16i 0x415510
   0x415510:	nop
   0x415511:	mov    ebx,0x200
   0x415516:	jmp    0x41551a
   0x415518:	movabs r14,0x2ebfd8949909090
   0x415522:	movabs r14,0x2ebc62949fe8949
   0x41552c:	movabs r14,0xe0ff00415229b890
```

It seems that we're jumping inside that big numbers. So there is an example of jump-oriented programming.

The file [dev/construct.py](dev/construct.py) contains the full listing of this hidden assembler parts. It just does `XOR(flag, ciphertext)` and compares with bunch of int32 numbers. `XOR` is reversible, so the rest is just `XOR(ciphertext, bunch_of_int32)`.

# Flag

```
Aero{I_l1ke_jUUUmp_0r13nt3d_pr0gr4mm1ng_1ec6ea6eccc0a28c60cae8c436d9fb3a}
```
