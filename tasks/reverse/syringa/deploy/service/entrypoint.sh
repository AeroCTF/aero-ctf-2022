#!/bin/bash

while true; do
    socat TCP-LISTEN:31337,reuseaddr,fork EXEC:"/tmp/syringa/syringa"
done
