# C2 Project - John R. Smith
### Build Instructions
This project relies on the tiny-aes library. Those dependencies are located in the /tiny-aes directory. Build them by going into that directory and running make.
Once that is compiled, go into the src directory and run make as well. This will compile the backdoor binary for the week4 machine.
#### Required backdoor files
- example_key: for AES authentication, you can modify this to use a different key and IV
- backdoor (binary): the remote shell
- 

### Configuration & Manual
Once the backdoor is configured properly, no more direct interaction with the victim (week4) machine is needed. As specified in the writeup, the backdoored machine accepts client connections via TCP sockets. Subsequently, you must point the *c2 client* at the *backdoor machine* in order for it to recieve commands. This configuration is done via arguments to allow for maximum flexibility.
The arguments are as follows: ./client <server_ip> <server_port> <user> <password> <number of retries>, where
- user and password are by default 'user' and 'password'. This can be modified in the source (backdoor.c).
- The default port for the backdoor is 1337, however this can be changed via optional command line arguments *to the backdoor* (see above)
- 
