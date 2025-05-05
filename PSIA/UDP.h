#ifndef UDP_H
#define UDP_H

#include <stdbool.h>
#include <stdint.h>
#include <winsock2.h>

#define MODE_SENDER 0
#define MODE_RECEIVER 1
//
#define CRC32_POLYNOMIAL 0xEDB88320
#define CRC32_INITIAL 0xFFFFFFFF
#define MD5_LEN 16
//
#define WINDOW_LEN 4
#define PACKET_MAX_SIZE 1024
#define PACKET_HEADER_LEN 5                                                                         // Including \0
#define PACKET_DATA_LEN (PACKET_MAX_SIZE - PACKET_HEADER_LEN - sizeof(uint32_t) - sizeof(uint32_t)) // Offset + crc32
//
#define PACKET_HEADER_NAME "NAME"
#define PACKET_HEADER_SIZE "SIZE"
#define PACKET_HEADER_HASH "HASH"
#define PACKET_HEADER_START "STRT"
#define PACKET_HEADER_DATA "DATA"
#define PACKET_HEADER_STOP "STOP"
#define PACKET_HEADER_ACK "ACK"
#define PACKET_HEADER_NACK "NACK"
#define PACKET_HEADER_RESEND "RSND"
#define PACKET_HEADER_NEXT "NEXT"
//
#define PACKET_TIMEOUT_SAW_S 1    // Base, can be increased on following attempt
#define PACKET_TIMEOUT_SR_MS 1000 // 1s

#define min(a, b) (((a) < (b)) ? (a) : (b))

typedef enum {
  ERR_INVALID_ARG = 100,
  ERR_MEM_ALLOC = 101,
  ERR_FILE_NOT_FOUND = 102,
  ERR_FILE_READ = 103,
  ERR_FILE_WRITE = 104,
  ERR_SOCKET_INIT = 200,
  ERR_SOCKET_CREATE = 201,
  ERR_SOCKET_BIND = 202,
  ERR_SOCKET_SEND = 203,
  ERR_SOCKET_RECEIVE = 204,
  ERR_PACKET_CRC = 301,
  ERR_PACKET_MD5 = 302
} Error;

// Buffer to store the whole file
typedef struct {
  char *data;
  char filename[PACKET_DATA_LEN];
  uint8_t md5[MD5_LEN];
  size_t length;
  size_t size;
} FileBuffer;

// Packet of max size 1024 bytes
typedef struct {
  char header[PACKET_HEADER_LEN];
  uint32_t offset;
  uint32_t crc32;
  char data[PACKET_DATA_LEN];
} Packet;

// Slot for a packet inside sliding window
typedef struct {
  Packet packet;
  bool ack;
  uint64_t timestamp;
} WindowSlot;

int InitWinsock();
int initCRC32Table();
int computeCRC32(const uint8_t *data, size_t length, uint32_t *result);
int allocateFileBuffer(FileBuffer *buffer);
int reallocateFileBuffer(FileBuffer *buffer, int size);
int loadFile(const char *filename, FileBuffer *buffer);
int sendPacket(SOCKET socket, Packet *packet, const struct sockaddr_in *dest);
int sendAndWaitForAck(SOCKET socket, Packet *packet, const struct sockaddr_in *dest);
int sendFileData(SOCKET socket, FileBuffer *buffer, const struct sockaddr_in *dest, size_t *total_packets);
int sendFile(SOCKET socket, FileBuffer *buffer, const struct sockaddr_in *dest);
int receiveFile(SOCKET socket, struct sockaddr_in *from, FileBuffer *buffer);
int saveFile(FileBuffer *buffer);

#endif /* UDP_H */