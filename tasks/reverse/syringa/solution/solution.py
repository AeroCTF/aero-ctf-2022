#!/usr/bin/env python3

import os
import enum
import base64
import asyncio
import tempfile
import subprocess
from typing import Tuple, List, Union


BLOCKSIZE = 128
DIRECTORY = 'blocks'


class Operation(enum.Enum):
    Add = enum.auto()
    Sub = enum.auto()
    Mul = enum.auto()
    Div = enum.auto()
    Mod = enum.auto()

    Or = enum.auto()
    And = enum.auto()
    Xor = enum.auto()

    Jump = enum.auto()
    JumpIfZero = enum.auto()
    JumpIfNotZero = enum.auto()

    Shift = enum.auto()
    ShiftIfZero = enum.auto()
    ShiftIfNotZero = enum.auto()

    Pop = enum.auto()
    Push = enum.auto()
    Swap = enum.auto()
    Dup = enum.auto()

    Put = enum.auto()
    Get = enum.auto()

    Nop = enum.auto()
    Stop = enum.auto()


async def my_md5(data: bytes) -> str:
    with tempfile.NamedTemporaryFile('wb') as file:
        file.write(data)
        file.flush()

        process = await asyncio.create_subprocess_exec(
            './my_md5', file.name, stdout = subprocess.PIPE,
        )

        stdout, _ = await process.communicate()

    return stdout.strip().decode()


async def find_collision(prefix: bytes, timeout: int) -> Tuple[bytes, bytes]:
    with tempfile.NamedTemporaryFile('wb') as file:
        file.write(prefix)
        file.flush()

        while True:
            os.unlink('md5_data1')
            os.unlink('md5_data2')

            process = await asyncio.create_subprocess_exec(
                'timeout', str(timeout),
                'stdbuf', '-i0', '-o0', '-e0',
                './my_fastcoll', file.name,
                stdout = subprocess.PIPE,
            )

            await process.communicate()
            await process.wait()

            with open('md5_data1', 'rb') as file1, open('md5_data2', 'rb') as file2:
                block1 = file1.read()
                block2 = file2.read()

            if len(block1) > len(prefix) and len(block2) > len(prefix):
                break

    assert await my_md5(block1) == await my_md5(block2)

    return block1, block2


async def find_collision_parallel(prefix: bytes, timeout: int, parallel: int) -> Tuple[bytes, bytes]:
    coroutines = [
        asyncio.create_task(find_collision(prefix, timeout)) for _ in range(parallel)
    ]

    finished, unfinished = await asyncio.wait(
        coroutines, return_when = asyncio.FIRST_COMPLETED,
    )

    for task in finished:
        result = task.result()

        for task in unfinished:
            task.cancel()

        if len(unfinished) > 0:
            await asyncio.wait(unfinished)

        return result


async def generate_blocks(count: int) -> None:
    if not os.path.isdir(DIRECTORY):
        os.mkdir(DIRECTORY)

    start = 0
    prefix = b''

    for i in range(count):
        if os.path.isfile(f'{DIRECTORY}/{i}a') and os.path.isfile(f'{DIRECTORY}/{i}b'):
            with open(f'{DIRECTORY}/{i}a', 'rb') as file:
                start += 1
                prefix += file.read()
        else:
            break
    
    for i in range(start, count):
        print(f'colliding blocks for {i = }')

        block1, block2 = await find_collision_parallel(prefix, 5, 1)

        with open(f'{DIRECTORY}/{i}a', 'wb') as file1, open(f'{DIRECTORY}/{i}b', 'wb') as file2:
            file1.write(block1[-BLOCKSIZE:])
            file2.write(block2[-BLOCKSIZE:])

        prefix = block1

        print(f'found blocks for {i = }', len(prefix))


def mutate(opcodes: List[int]) -> List[int]:
    return opcodes


def assemble_code(code: List[Union[Operation, int]]) -> bytes:
    opcodes = [
        0x45, 0x35, 0x3b, 0x67, 0xd7, 0xb0, 0x0c, 0xa5, 0x53, 0x8f, 0xab, 0x05, 0xbe, 0xe7, 0x81, 0x56, 0x9c, 0x3c, 0xec, 0xd0, 0x55, 0xe1,
    ]

    operations = [
        Operation.Add, Operation.Sub, Operation.Mul, Operation.Div, Operation.Mod, Operation.Or, Operation.And, Operation.Xor, Operation.Jump, Operation.JumpIfZero, Operation.JumpIfNotZero, Operation.Shift, Operation.ShiftIfZero, Operation.ShiftIfNotZero, Operation.Pop, Operation.Push, Operation.Swap, Operation.Dup, Operation.Put, Operation.Get, Operation.Nop, Operation.Stop,
    ]

    result = []

    for i, x in enumerate(code):
        if isinstance(x, int):
            result.append(x)
            continue

        transition = {
            operation: opcode for opcode, operation in zip(opcodes, operations)
        }

        result.append(transition[x])

        opcodes = mutate(opcodes)

    return bytes(result)


def construct_code(change: int) -> bytes:
    code = [
        Operation.Nop,
    ]

    for i in range(16):
        # put 0 at [i]
        code += [
            Operation.Push, 0, 0, 0, 0, 0, 0, 0, 0,
            Operation.Push, i, 0, 0, 0, 0, 0, 0, 0,
            Operation.Put,
        ]

    code += [
        # put 2 at [16]
        Operation.Push, 2, 0, 0, 0, 0, 0, 0, 0,
        Operation.Push, 16, 0, 0, 0, 0, 0, 0, 0,
        Operation.Put,

        # put 1 at [17]
        Operation.Push, 1, 0, 0, 0, 0, 0, 0, 0,
        Operation.Push, 17, 0, 0, 0, 0, 0, 0, 0,
        Operation.Put,
    ]

    for byte in range(16):
        code += [
            # put 1 at [18]
            Operation.Push, 1, 0, 0, 0, 0, 0, 0, 0,
            Operation.Push, 18, 0, 0, 0, 0, 0, 0, 0,
            Operation.Put,
        ]

        for bit in range(8):
            index = 8 * byte + bit
            position_hint = index + BLOCKSIZE * BLOCKSIZE
            position_block = change + BLOCKSIZE * index

            code += [
                # sub [change[bit]] and [hint[bit]] and shift by 19 if zero
                Operation.Push, position_block % 256, position_block // 256, 0, 0, 0, 0, 0, 0,
                Operation.Push, position_hint % 256, position_hint // 256, 0, 0, 0, 0, 0, 0,
                Operation.Push, position_block % 256, position_block // 256, 0, 0, 0, 0, 0, 0,
                Operation.Sub,
                Operation.Push, 19, 0, 0, 0, 0, 0, 0, 0,
                Operation.ShiftIfNotZero,

                # else do [byte] += [18]
                Operation.Push, 18, 0, 0, 0, 0, 0, 0, 0,
                Operation.Push, byte, 0, 0, 0, 0, 0, 0, 0,
                Operation.Add,

                # do [18] *= 2
                Operation.Push, 16, 0, 0, 0, 0, 0, 0, 0,
                Operation.Push, 18, 0, 0, 0, 0, 0, 0, 0,
                Operation.Mul,
            ]

    code += [
        # end
        Operation.Stop,
    ]

    return assemble_code(code)


def construct_memory(data: bytes, change: int) -> bytes:
    blocks_a = []
    blocks_b = []

    for i in range(BLOCKSIZE):
        with open(f'{DIRECTORY}/{i}a', 'rb') as file1, open(f'{DIRECTORY}/{i}b', 'rb') as file2:
            block1 = file1.read()
            block2 = file2.read()

        blocks_a.append(block1)
        blocks_b.append(block2)

    hint = bytes(block[change] for block in blocks_a)

    parts = []
    index = 0

    for byte in data:
        bits = bin(byte)[2:].zfill(8)[::-1]

        for bit in bits:
            part1 = blocks_a[index]
            part2 = blocks_b[index]

            if int(bit) == 1:
                parts.append(part1)
            else:
                parts.append(part2)

            index += 1

    return b''.join(parts) + hint


async def main():
    # await generate_blocks(128)

    change = 19
    target = bytes.fromhex('bc574f4766457f7f95fa1392084b1682')

    code = construct_code(change)
    memory = construct_memory(target, change)

    print(base64.b64encode(code).decode())
    print(base64.b64encode(memory).decode())


if __name__ == '__main__':
    asyncio.run(main())
