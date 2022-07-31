# Aero CTF 2022 | Web | Empathy

# Description

> Can you hear the silence? Can you see the dark?

# Files

- [public/web-empathy.tar.gz](public/web-empathy.tar.gz)

# Solution

Look at `requirements.txt`:

```
fastapi==0.73.0
uvicorn==0.17.4
cryptography==36.0.1
PyJWT==1.7.1
pydantic==1.9.0
aioredis==2.0.1
```

Version of PyJWT library is too old. In this version of library you don't need to specify an algorithm in `jwt.encode()` and `jwt.decode()`. It means that the attacker makes the choice of the algorithm.

The server by default uses RS256, but we will choice ES256, it's based on elliptic curves.

It's well-known fact that the single ECDSA signature can be valid for two different messages, because the point `(X, Y)` is serialized to one single coordinate `X`, so `(X, Y)` and `(X, -Y)` are indistinguishable for ECDSA.

According to this property, we will construct a ECDSA private key that will sign two different messages equally:

```
{'username': 'attacker'}
{'username': 'admin'}
```

Here is a problem: PyJWT does not allow to use a private ECDSA key as a RS256 key. We can bypass this easily, just encode the key in the OpenSSH format, the PyJWT omits this check.

The algorithm:

1. Register with crafted ECDSA key as a password. The server will create JWT using RS256
2. Create another JWT manually using ES256 and crafted ECDSA key. It will be validated on the server, because the user's password contains the same ECDSA key.
3. Reuse the signature part of JWT and replace the username with `admin`. Since the signature is still presented in database, the full validation of the token will be omitted. It will be considered correct.
4. Read the flag as `admin`

Example solver: [solution/solver.py](solution/solver.py)

# Flag

```
Aero{y0u_mu5t_w0rk_1n_symm3try_y0u_mu5t_e4rn_th31r_emp4thy}
```
