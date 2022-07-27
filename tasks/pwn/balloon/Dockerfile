FROM python:3.11-rc-slim AS builder

RUN for name in 'ctypes' 'pickle' 'test'; do \
        find /usr/local/lib/python3.11/ -name "*${name}*" -exec rm -rf '{}' '+'; \
    done

FROM scratch

COPY --from=builder /lib/x86_64-linux-gnu /lib/x86_64-linux-gnu
COPY --from=builder /lib64/ld-linux-x86-64.so.2 /lib64/ld-linux-x86-64.so.2
COPY --from=builder /usr/lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu
COPY --from=builder /usr/local/lib/python3.11 /usr/local/lib/python3.11
COPY --from=builder /usr/local/lib/libpython3.11.so.1.0 /usr/lib/libpython3.11.so.1.0

COPY --from=builder /usr/sbin/useradd bin/chmod /bin/false /bin/rm /usr/local/bin/python3.11 /bin/sh /bin/

RUN /bin/useradd --no-create-home --shell /bin/false --uid 999 flag
RUN /bin/useradd --no-create-home --shell /bin/false --uid 1000 challenge

COPY --chown=999:999 win /challenge/win
COPY --chown=999:999 flag.txt /challenge/flag.txt

COPY preload.so balloon.py /challenge/

RUN /bin/chmod 400 /challenge/flag.txt \
    && /bin/chmod 111 /challenge/win \
    && /bin/chmod +s /challenge/win \
    && /bin/chmod 555 /challenge/preload.so \
    && /bin/chmod 444 /challenge/balloon.py

RUN /bin/rm /bin/rm /bin/useradd /bin/chmod /bin/sh

USER 1000:1000

ENV LD_PRELOAD='/challenge/preload.so'

ENTRYPOINT [ "/bin/python3.11", "-u", "/challenge/balloon.py" ]
