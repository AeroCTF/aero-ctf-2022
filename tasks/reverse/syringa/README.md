# Aero CTF 2022 | Reverse | Syringa

# Description

> Ahh, I really love flowers.

# Files

- [public/reverse-syringa.tar.gz](public/reverse-syringa.tar.gz)

# Solution

The server implements a simple virtual machine ([dev/inc/vm.hpp](dev/inc/vm.hpp)). The architecture is [Harvard](https://en.wikipedia.org/wiki/Harvard_architecture), so the code and the memory are separated.

The program loads code and memory from user's input and executes it. The result of execution is stored in first 16 bytes of memory. The goal of the challenge is to craft the program with the following property:

```
MD5(memory || code) = execute(code, memory)
```

The program needs to write it's own hash. It's looks like a [quine](https://en.wikipedia.org/wiki/Quine_(computing)), but for hashes it called _hashquine_.

Here is [an example](https://www.rogdham.net/2017/03/12/gif-md5-hashquine.en) of hashquine for GIF format. We will use the same technique:

1. Generate 128 pairs of MD5 collisions. It could be done in ~30 minutes using [fastcoll](https://github.com/upbit/clone-fastcoll). Each pair is `(block_a, block_b)`, `MD5(block_a) == md5(block_b)`.

2. Write in memory `block1_a || block1_b || block2_a || block2_b || ...` 

3. Craft the following program:

```
if block1_a == block1_b:
    output_bit(1)
else:
    output_bit(0)

if block2_a == block2_b:
    output_bit(1)
else:
    output_bit(0)

if block3_a == block3_b:
    output_bit(1)
else:
    output_bit(0)

...
```

Doing this 128 times we will output the full MD5 hash.

We can arbitrary swap `block[i]_a` and `block[i]_b` because they are collisions and MD5 is equal.

4. Calculate `hash = MD5(memory || code)` and to the following:

```
new_memory = []

for i in range(128):
    if hash[i] == 1:
        new_memory.append(block[i]_a)
        new_memory.append(block[i]_a)
    ele:
        new_memory.append(block[i]_a)
        new_memory.append(block[i]_b)
```

So if the `hash[i] == 1`, the blocks will be equal, otherwise they will be different. Again, since they are collisions, MD5 hash will be the same.

5. Assert that `MD5(new_memory || code) == hash`

6. Send the program to the server and get the flag.

Example solver: [solution/solution.py](solution/solution.py)

P.S. The challenge changed the constants of MD5, so it's required to patch fastcoll library.

# Flag

```
Aero{c0nGR4tz_y0U_h4ve_maD3_a_h4shq1une}
```
