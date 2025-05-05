# UDP File Transfer
A reliable UDP file transfer implementation using stop-and-wait and selective repeat protocol with CRC32 and MD5 verification.
 
## Building
To compile the program, use the following command:
```bash
gcc -o UDP.exe UDP.c -I. -Wall -Wextra -pedantic -O1 -lws2_32
```

## Usage
The program can run in two modes: sender (0) or receiver (1).
```bash
UDP.exe <mode> <local_port> <target_port> <target_ip> <filename>
```

### Sender Mode Example:
```bash
UDP.exe 0 5002 5001 127.0.0.1 file.txt
```

### Receiver Mode Example:
```bash
UDP.exe 1 5001 5002 127.0.0.1
```

### Parameters
- `mode`: 0 for sender, 1 for receiver
- `local_port`: Port to bind to locally (1-65535)
- `target_port`: Port of the target machine (1-65535)
- `target_ip`: IP address of the target machine
- `filename`: Path to file to send (sender mode only)