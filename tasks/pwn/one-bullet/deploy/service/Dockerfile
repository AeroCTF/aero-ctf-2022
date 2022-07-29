FROM keltecc/nsjail:latest

RUN apt update

RUN useradd -M -s /bin/false ctf

COPY entrypoint.sh /tmp/

RUN chmod 555 /tmp/entrypoint.sh

COPY one_bullet flag.txt /var/service/

USER ctf

ENTRYPOINT /tmp/entrypoint.sh
