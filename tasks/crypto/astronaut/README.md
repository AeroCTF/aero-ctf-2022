# Aero CTF 2022 | Crypto | Astronaut

# Description

> Through the milky way
> 
> In my spaceship
> 
> At the speed of light
> 
> I'm gonna make it

# Files

- [public/crypto-astronaut.tar.gz](public/crypto-astronaut.tar.gz)

# Note

## The challenge contains unintented solution that levels out the entire challenge.

## Unfortunately, the author forgot about matrices and have not checked it. The trivial solution is to use matrix equation and recover LFSR polynomial esaily.

## By the way, the intended solution is described below.

# Solution

Look at the `flight()` function:

```python
def flight(galaxy: List[int], black_holes: List[int]) -> Generator[int, None, None]:
    while True:
        yield (
            black_holes := black_holes[1:] + [
                sum(x * y for x, y in zip(galaxy, black_holes))
            ]
        )[0]
```

Looks tricky, but it's a [LFSR](https://en.wikipedia.org/wiki/Linear-feedback_shift_register) actually.

Here is well-known [Berlekamp-Massey algorithm](https://en.wikipedia.org/wiki/Berlekamp%E2%80%93Massey_algorithm), but it will not work, because the module is not prime.

The goal of the challenge is to use [Reeds-Sloane algorithm](https://en.wikipedia.org/wiki/Reeds%E2%80%93Sloane_algorithm), that deals with a composite module.

I have not found any working public implementation of Reeds-Sloane, so here it is: [solution/solver.sage](solution/solver.sage). P-adic numbers are used in order to get working inverses.

Due to unintented it becomes useless, but at least it is the contribution to open source algorithms.

# Flag

```
Aero{LFSR_is_4lw4ys_e4sy_r1ght?}
```
