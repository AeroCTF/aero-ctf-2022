FROM docker:20.10

RUN apk add socat

COPY service /tmp/balloon

COPY entrypoint.sh run.sh /tmp/

RUN chmod +x /tmp/entrypoint.sh /tmp/run.sh

ENTRYPOINT [ "/tmp/entrypoint.sh" ]
