# UDP File Transfer
A reliable UDP file transfer implementation using stop-and-wait and selective repeat protocol with CRC32 and MD5 verification.
 
## Requirements
- Windows
- GCC compiler (MinGW-w64 recommended)
- Windows Socket library (ws2_32)

## Building
To compile the program, use the following command:
```bash
gcc -o UDP.exe UDP.c md5.c -I. -Wall -Wextra -pedantic -O1 -lws2_32
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

## Linux Port
To port this program to Linux, the following changes are needed:

### UDP.h
```c
// Replace Windows headers
- #include <winsock2.h>
+ #include <sys/socket.h>
+ #include <netinet/in.h>
+ #include <arpa/inet.h>
+ #include <unistd.h>

// Replace Windows types
- SOCKET;
+ int;
```

### UDP.c
```c
// Replace Windows headers
- #define WIN32_LEAN_AND_MEAN
- #include <winsock2.h>
- #include <ws2tcpip.h>
+ #include <sys/socket.h>
+ #include <netinet/in.h>
+ #include <arpa/inet.h>
+ #include <unistd.h>
+ #include <sys/time.h>
+ #include <fcntl.h>
+ #include <errno.h>

// Replace Windows types
- SOCKET;
+ int;

// Replace Windows functions
- closesocket() → close()
- WSAGetLastError() → errno
- ioctlsocket() → fcntl()
- WSAStartup() and WSACleanup() and InitWinsock() → remove completely
- InetPton() → inet_pton()

// Replace Windows constants
- INVALID_SOCKET → -1
- SOCKET_ERROR → -1
- WSAEWOULDBLOCK → EWOULDBLOCK

// Update Windows socket options
- u_long mode = 1;
- ioctlsocket(socket, FIONBIO, &mode);
+ int flags = fcntl(socket, F_GETFL, 0);
+ fcntl(socket, F_SETFL, flags | O_NONBLOCK);

// Update timeout settings
- setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
+ setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const void *)&tv, sizeof tv);
```

### Compilation
```bash
gcc -o UDP UDP.c md5.c -I. -Wall -Wextra -pedantic -O1
```