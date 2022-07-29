#!/usr/bin/env python3

import os
import struct
import asyncio
import subprocess
from typing import List


async def get_filename(substring: str) -> str:
    print(substring)

    process = await asyncio.create_subprocess_exec(
        'grep', '-Ral', substring, 'musl-1.2.3/src/', stdout = subprocess.PIPE,
    )

    stdout, _ = await process.communicate()

    return stdout.decode().strip()


async def get_number(code: str) -> int:
    with open('file.s', 'w') as file:
        file.write('BITS 64\n\n')
        file.write(code.replace(';', '\n'))
        file.write('\n')

    process = await asyncio.create_subprocess_exec(
        'nasm', 'file.s', stdout = subprocess.PIPE,
    )

    await process.communicate()

    with open('file', 'rb') as file:
        data = file.read()

    while len(data) < 8:
        data = b'\x90' + data

    if len(data) > 8:
        raise Exception((code, data))

    print(code, data)

    os.unlink('file')
    os.unlink('file.s')

    return int.from_bytes(data, 'little')


async def build_asm(numbers: List[int]) -> str:
    return ''.join([
        '"',
        f'jmp . + {2 + 10 * len(numbers)}; ',
        '; '.join(f'mov ${hex(x)}, %r14' for x in numbers),
        '"',
    ])


async def replace_content(placeholder: str, code: List[str]) -> None:
    numbers = [await get_number(x) for x in code]
    asm = await build_asm(numbers)

    filename = await get_filename(placeholder.replace('*', '\\*'))

    with open(filename, 'r') as file:
        lines = file.read().split('\n')

    for i in range(len(lines)):
        if placeholder in lines[i]:
            lines[i] = placeholder + asm

    with open(filename, 'w') as file:
        file.write('\n'.join(lines))


async def main():
    flag = b'Aero{I_l1ke_jUUUmp_0r13nt3d_pr0gr4mm1ng_1ec6ea6eccc0a28c60cae8c436d9fb3a}'
    assert len(flag) == 73

    ciphertext = [
        0x6353c0dddb255b65, 0x280638028af8e1cc, 0x2df42cc9b357253f, 0x2a352e99849a4330, 0xce7a7054825ea935, 0x2843a085fa911f83, 0x82af923a6aad886b, 0x3178f42d3ac0d784,
    ]

    expected = [
        x ^ y for x, y in zip(
            ciphertext + ciphertext[-1:],
            struct.unpack('<QQQQQQQQQ', flag[:-1]),
        )
    ]

    # __lockfile
    code_01 = [
        "mov eax, 0x210; jmp $+4",
        "mov ebx, 0x200; jmp $+4",
        "mov r13, rdi; jmp $+4",
        "mov r14, rdi; sub r14, rax; jmp $+4",
        "nop; mov rax, 0x415229; jmp rax",
    ]

    # __fpclassifyl
    code_02 = [
        "mov r15, QWORD [r14]; jmp $+4",
        "xor QWORD [r13], r15; jmp $+4",
        "lea r13, [r13 + 8]; jmp $+4",
        "lea r14, [r14 + 8]; jmp $+4",
    ] * 8 + [
        "xor QWORD [r13], r15; jmp $+4",
        "nop; mov rax, 0x41ae43; jmp rax",
    ]

    # __madvise
    code_03 = [
        "xor eax, eax; add ax, 0; jmp $+4",
        "xor rcx, rcx; mov r13, rdi; jmp $+4",
    ]

    for number in expected:
        for part in [number & 0xFFFFFFFF, number >> 32]:
            code_03 += [
                f"mov edi, {hex(part)}; jmp $+4",
                "mov esi, DWORD [r13]; jmp $+4",
                "cmp edi, esi; je x; inc cl; x:; jmp $+4",
                "add r13, 4; jmp $+4",
            ]
    
    code_03 += [
        "nop; mov rbx, 0x415391; jmp rbx",
    ]

    # __signbitl
    code_04 = [
        "xor rdi, rdi; jmp $+4",
        "test cl, cl; jz x; call rax; x:; jmp $+4",
        "nop; mov rbx, 0x41977d; jmp rbx",
    ]

    # __simple_malloc
    code_05 = [
        "mov ecx, 0x4135b4; jmp $+4",
        "mov edi, 0x41c02d; jmp $+4",
        "mov r15d, 0x4134f5; jmp $+4",
        "call rcx; xor edi, edi; jmp r15",
    ]

    replacements = {
        '/* ARCTIC_HERE_01 */': code_01,
        '/* ARCTIC_HERE_02 */': code_02,
        '/* ARCTIC_HERE_03 */': code_03,
        '/* ARCTIC_HERE_04 */': code_04,
        '/* ARCTIC_HERE_05 */': code_05,
    }

    for placeholder, code in replacements.items():
        await replace_content(placeholder, code)


if __name__ == '__main__':
    asyncio.run(main())
