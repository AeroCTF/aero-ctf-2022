#!/bin/sh

sleep 10

docker build --tag balloon /tmp/balloon

while true; do
    socat TCP-LISTEN:31337,reuseaddr,fork,user=1000,group=1000 EXEC:"/tmp/run.sh",stderr
done
