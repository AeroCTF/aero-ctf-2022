FROM ubuntu:22.04

RUN apt update && apt install -y socat

COPY service /tmp/syringa

RUN chmod +x /tmp/syringa/entrypoint.sh

USER 1000:1000

ENTRYPOINT [ "/tmp/syringa/entrypoint.sh" ]
