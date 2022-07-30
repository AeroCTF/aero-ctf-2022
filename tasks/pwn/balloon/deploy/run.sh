#!/bin/sh

uuid=$(
    cat /proc/sys/kernel/random/uuid | cut -d '-' -f 1
)

clear_func="
    docker kill balloon_${uuid} && docker rm balloon_${uuid} || true
"

nohup sh -c "sleep 60 && ${clear_func}" >/dev/null 2>/dev/null &
nohup_pid=$!

docker run \
    --rm \
    --interactive \
    --cpus 0.1 \
    --memory 64M \
    --memory-swap -1 \
    --pids-limit 32 \
    --read-only \
    --name balloon_${uuid} \
    --network none \
    -- \
    balloon

eval $clear_func >/dev/null 2>/dev/null
kill -9 $nohup_pid >/dev/null 2>/dev/null
