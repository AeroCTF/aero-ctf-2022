#!/usr/bin/env sage


def L(a, b):
    a_deg = a.degree() if not a.is_zero() else -4000
    b_deg = b.degree() if not b.is_zero() else -4000

    return max(a_deg, b_deg + 1)


def reeds_sloane(pol, F, R, X, p, e):
    a, b = [], []
    a_new, b_new = [], []
    theta, u = [], []

    for i in range(e):
        a.append(R(p**i))
        b.append(R.zero())

        a_new.append(R(p**i))
        b_new.append(R(p**i) * pol[0])

        u.append(0)
        theta.append(((pol * a[i] - b[i]) % X)[0])

        if theta[i].is_zero():
            u[i] = e
            theta[i] = F.one()
        else:
            while theta[i] % p == 0:
                u[i] += 1
                theta[i] //= p

    a_old = [R.zero() for _ in range(e)]
    b_old = [R.zero() for _ in range(e)]
    theta_old = [F.one() for _ in range(e)]
    u_old = [0 for _ in range(e)]
    r = [0 for _ in range(e)]

    for k in range(1, pol.degree()):
        for g in range(e):
            if L(a_new[g], b_new[g]) > L(a[g], b[g]):
                h = e - 1 - u[g]

                a_old[g] = a[h]
                b_old[g] = b[h]
                theta_old[g] = theta[h]
                u_old[g] = u[h]
                r[g] = k - 1

        a = [x for x in a_new]
        b = [x for x in b_new]

        for n in range(e):
            u[n] = 0
            theta[n] = (((pol * a[n] - b[n]) % X**(k + 1))[k])

            if theta[n].is_zero():
                u[n] = e
                theta[n] = F.one()
            else:
                while theta[n] % p == 0:
                    u[n] += 1
                    theta[n] //= p

            g = e - 1 - u[n]

            if u[n] == e:
                a_new[n] = a[n]
                b_new[n] = b[n]
            elif L(a[g], b[g]) == 0:
                a_new[n] = a[n]
                b_new[n] = b[n] + theta[n] * p**u[n] * X**k
            else:
                a_new[n] = a[n] - theta[n] / theta_old[g] * p**(u[n] - u_old[g]) * X**(k - r[g]) * a_old[g]
                b_new[n] = b[n] - theta[n] / theta_old[g] * p**(u[n] - u_old[g]) * X**(k - r[g]) * b_old[g]

    return a_new[0], b_new[0]


def lfsr_inv(poly, state):
    while True:
        value = state[-1]

        for i in range(1, 16):
            value -= poly[i] * state[i - 1]
            value %= 1000

        state0 = int(value / Mod(poly[0], 1000))

        state = [state0] + state[:-1]

        yield state0


def to_zp(F, R, pol):
    return R(
        [F(int(item)) for item in list(pol)]
    )


def from_zp(F, R, pol):
    return R(
        [F(int(item.lift())) for item in list(pol)]
    )


def main():
    ZmodN = Zmod(1000)
    R.<x> = PolynomialRing(ZmodN)

    Zp_2 = Zp(2, prec = 3, type = 'fixed-mod', print_mode = 'series')
    Zp_5 = Zp(5, prec = 3, type = 'fixed-mod', print_mode = 'series')

    R_2.<X> = PolynomialRing(Zp_2)
    R_5.<Y> = PolynomialRing(Zp_5)

    with open('output.txt', 'r') as file:
        output = [
            int(x) for x in file.read().split()
        ]

    leak = output[-63:]
    flag = output[:-63]

    leak_2 = to_zp(Zp_2, R_2, leak)
    poly_2a, poly_2b = reeds_sloane(leak_2, Zp_2, R_2, X, 2, 3)

    print(leak_2 * poly_2a % X**32 == poly_2b)
    
    leak_5 = to_zp(Zp_5, R_5, leak)
    poly_5a, poly_5b = reeds_sloane(leak_5, Zp_5, R_5, Y, 5, 3)

    print(leak_5 * poly_5a % Y**32 == poly_5b)

    poly_2a = from_zp(ZmodN, R, poly_2a)
    poly_5a = from_zp(ZmodN, R, poly_5a)

    poly_2a = -R(list(poly_2a)[::-1])
    poly_5a = -R(list(poly_5a)[::-1])

    guess = R(
        CRT(
            [poly_2a.change_ring(ZZ), poly_5a.change_ring(ZZ)],
            [2 ** 3, 5 ** 3],
        )
    ) % (x ^ 16)

    print(guess)

    stream = lfsr_inv(list(guess), leak[:16])
    plaintext = []

    for i in range(len(flag)):
        plaintext.append(next(stream) ^^ flag[-i - 1])

    print(bytes(plaintext)[::-1])


if __name__ == '__main__':
    main()
