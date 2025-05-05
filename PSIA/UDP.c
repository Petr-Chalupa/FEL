#define WIN32_LEAN_AND_MEAN

#include "UDP.h"
#include "md5.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <winnt.h>
#include <winsock2.h>
#include <ws2tcpip.h>

uint32_t crc32_table[256];

int main(int argc, char *argv[]) {
  if (argc < 5) {
    printf("Usage: %s <mode> <local_port> <target_port> <target_ip> [filename]\n", argv[0]);
    return ERR_INVALID_ARG;
  }
  int mode = atoi(argv[1]);
  int local_port = atoi(argv[2]);
  int target_port = atoi(argv[3]);
  char *target_ip = argv[4];
  // Validate mode
  if (mode != MODE_SENDER && mode != MODE_RECEIVER) return ERR_INVALID_ARG;
  // Validate ports
  if (local_port <= 0 || local_port > 65535 || target_port <= 0 || target_port > 65535) {
    printf("Error: Port numbers must be between 1 and 65535\n");
    return ERR_INVALID_ARG;
  }

  int init_crc_r = initCRC32Table();
  if (init_crc_r != 0) return init_crc_r;

  int init_ws_r = InitWinsock();
  if (init_ws_r != 0) return init_ws_r;

  SOCKET socket_handle = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_handle == INVALID_SOCKET) return ERR_SOCKET_CREATE;

  struct sockaddr_in local;
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = INADDR_ANY;
  local.sin_port = htons(local_port);
  if (bind(socket_handle, (struct sockaddr *)&local, sizeof(local)) != 0) {
    closesocket(socket_handle);
    WSACleanup();
    return ERR_SOCKET_BIND;
  }

  FileBuffer buffer = {NULL, "", {0}, 0, 0};
  int fbAlloc_r = allocateFileBuffer(&buffer);
  if (fbAlloc_r != 0) {
    closesocket(socket_handle);
    WSACleanup();
    return fbAlloc_r;
  }

  if (mode == MODE_SENDER) {
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(target_port);
    if (InetPton(AF_INET, target_ip, &target.sin_addr.s_addr) != 1) {
      closesocket(socket_handle);
      WSACleanup();
      return ERR_INVALID_ARG;
    }

    if (argc < 6) return ERR_INVALID_ARG;
    char *filename = argv[5];

    int fLoad_r = loadFile(filename, &buffer);
    if (fLoad_r != 0) {
      free(buffer.data);
      return fLoad_r;
    }

    int send_r = sendFile(socket_handle, &buffer, &target);
    if (send_r != 0) return send_r;
  }
  if (mode == MODE_RECEIVER) {
    struct sockaddr_in from;

    int receive_r = receiveFile(socket_handle, &from, &buffer);
    if (receive_r != 0) {
      free(buffer.data);
      return receive_r;
    }

    int save_r = saveFile(&buffer);
    if (save_r != 0) return save_r;
  }

  if (buffer.data != NULL) free(buffer.data);
  if (socket_handle != INVALID_SOCKET) closesocket(socket_handle);
  WSACleanup();

  return 0;
}

int InitWinsock() {
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return ERR_SOCKET_INIT;
  return 0;
}

int initCRC32Table() {
  if (crc32_table == NULL) return ERR_INVALID_ARG;

  for (uint32_t i = 0; i < 256; i++) {
    uint32_t crc = i;
    for (int j = 0; j < 8; j++) {
      crc = (crc & 1) ? ((crc >> 1) ^ CRC32_POLYNOMIAL) : (crc >> 1);
    }
    crc32_table[i] = crc;
  }

  return 0;
}

int computeCRC32(const uint8_t *data, size_t length, uint32_t *result) {
  if (data == NULL || result == NULL) return ERR_INVALID_ARG;

  uint32_t crc = CRC32_INITIAL;
  for (size_t i = 0; i < length; ++i) {
    uint8_t index = (uint8_t)((crc ^ data[i]) & 0xFF);
    crc = (crc >> 8) ^ crc32_table[index];
  }
  *result = ~crc;

  return 0;
}

int allocateFileBuffer(FileBuffer *buffer) {
  if (buffer == NULL) return ERR_INVALID_ARG;

  buffer->data = (char *)malloc(16 * sizeof(char));
  if (buffer->data == NULL) return ERR_MEM_ALLOC;

  buffer->size = 16;
  buffer->length = 0;

  return 0;
}

int reallocateFileBuffer(FileBuffer *buffer, int size) {
  if (buffer == NULL || buffer->data == NULL) return ERR_INVALID_ARG;

  size_t new_size = size == -1 ? buffer->size * 2 : (size_t)size;
  char *new_data = (char *)realloc(buffer->data, new_size * sizeof(char));
  if (new_data == NULL) return ERR_MEM_ALLOC;

  buffer->data = new_data;
  buffer->size = new_size;

  return 0;
}

int loadFile(const char *filename, FileBuffer *buffer) {
  if (filename == NULL || buffer == NULL) return ERR_INVALID_ARG;

  strncpy(buffer->filename, filename, sizeof(buffer->filename) - 1);
  buffer->filename[sizeof(buffer->filename) - 1] = '\0';

  FILE *file = fopen(filename, "rb");
  if (file == NULL) return ERR_FILE_NOT_FOUND;

  int result = 0;
  while (true) {
    int c = fgetc(file);
    if (c == EOF) {
      if (feof(file)) {
        break; // Expected EOF
      } else {
        result = ERR_FILE_READ;
        break;
      }
    }

    if (buffer->length + 1 > buffer->size) {
      int fbAlloc_r = reallocateFileBuffer(buffer, -1);
      if (fbAlloc_r != 0) {
        result = fbAlloc_r;
        break;
      }
    }
    buffer->data[buffer->length++] = (char)c;
  }

  fclose(file);

  // Calculate MD5
  if (result == 0) {
    MD5Context md5_context;
    md5Init(&md5_context);
    md5Update(&md5_context, (unsigned char *)buffer->data, buffer->length);
    md5Finalize(&md5_context);
    memcpy(buffer->md5, md5_context.digest, MD5_LEN);
  }

  return result;
}

int saveFile(FileBuffer *buffer) {
  if (buffer == NULL || buffer->data == NULL) return ERR_INVALID_ARG;

  // Safely copy filename and append ".out"
  char output_filename[1024] = {0};
  size_t filename_len = strlen(buffer->filename);
  if (filename_len > sizeof(output_filename) - 5) filename_len = sizeof(output_filename) - 5; // Space for ".out"
  strncpy(output_filename, buffer->filename, filename_len);
  output_filename[filename_len] = '\0';
  strncat(output_filename, ".out", sizeof(output_filename) - filename_len - 1);

  FILE *file = fopen(output_filename, "wb");
  if (file == NULL) return ERR_FILE_NOT_FOUND;

  size_t write_r = fwrite(buffer->data, 1, buffer->length, file);
  fclose(file);

  return (write_r == buffer->length) ? 0 : ERR_FILE_WRITE;
}

int sendPacket(SOCKET socket, Packet *packet, const struct sockaddr_in *dest) {
  if (socket == INVALID_SOCKET || packet == NULL || dest == NULL) return ERR_INVALID_ARG;

  // Calculate CRC
  uint32_t crc;
  int crc_r = computeCRC32((uint8_t *)packet->data, sizeof(packet->data), &crc);
  if (crc_r != 0) return crc_r;
  packet->crc32 = htonl(crc);

  // Send packet
  int send_r = sendto(socket, (const char *)packet, sizeof(Packet), 0, (const struct sockaddr *)dest, sizeof(struct sockaddr_in));
  if (send_r == SOCKET_ERROR) return ERR_SOCKET_SEND;

  return (send_r == sizeof(Packet)) ? 0 : WSAGetLastError();
}

int sendAndWaitForAck(SOCKET socket, Packet *packet, const struct sockaddr_in *dest) {
  if (socket == INVALID_SOCKET || packet == NULL || dest == NULL) return ERR_INVALID_ARG;

  int ack_received = 0;
  for (int attempts = 0; attempts < 5 && !ack_received; attempts++) {
    // Send the packet
    int send_r = sendPacket(socket, packet, dest);
    if (send_r != 0) return send_r;

    // Set timeout for receive
    struct timeval tv = {PACKET_TIMEOUT_SAW_S << attempts, 0}; // Exponential timeout
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    // Wait for ACK
    Packet response = {0};
    int destlen = sizeof(*dest);
    struct sockaddr_in from_addr = *dest;
    int r = recvfrom(socket, (char *)&response, sizeof(response), 0, (struct sockaddr *)&from_addr, &destlen);
    if (r > 0 && strcmp(response.header, PACKET_HEADER_ACK) == 0 && response.offset == packet->offset) {
      ack_received = 1;
      return 0;
    }
  }

  return ERR_SOCKET_RECEIVE; // Too many retries
}

int sendFileData(SOCKET socket, FileBuffer *buffer, const struct sockaddr_in *dest, size_t *total_packets) {
  if (socket == INVALID_SOCKET || buffer == NULL || dest == NULL) return ERR_INVALID_ARG;

  // Initialize sliding window
  WindowSlot window[WINDOW_LEN] = {0};
  *total_packets = (buffer->length + PACKET_DATA_LEN - 1) / PACKET_DATA_LEN;
  size_t base = 0;
  size_t next_seq_num = 0;

  // Set socket to non-blocking mode
  u_long mode = 1;
  ioctlsocket(socket, FIONBIO, &mode);

  while (base < *total_packets) {
    while (next_seq_num < base + WINDOW_LEN && next_seq_num < *total_packets) {
      size_t window_idx = next_seq_num % WINDOW_LEN;
      size_t offset = next_seq_num * PACKET_DATA_LEN;
      size_t chunk_size = min(PACKET_DATA_LEN, buffer->length - offset);

      memcpy(window[window_idx].packet.header, PACKET_HEADER_DATA, sizeof(window[window_idx].packet.header) - 1);
      window[window_idx].packet.offset = offset;
      memcpy(window[window_idx].packet.data, buffer->data + offset, chunk_size);

      // Send DATA packet
      sendPacket(socket, &window[window_idx].packet, dest);
      window[window_idx].timestamp = (uint64_t)time(NULL);
      window[window_idx].ack = false;

      next_seq_num++;
    }

    // Wait for ACKs
    Packet response = {0};
    int fromlen = sizeof(*dest);
    struct sockaddr_in from_addr = *dest;
    while (true) {
      int r = recvfrom(socket, (char *)&response, sizeof(response), 0, (struct sockaddr *)&from_addr, &fromlen);
      if (r <= 0) break;

      if (strcmp(response.header, PACKET_HEADER_ACK) == 0) {
        size_t acked_seq = response.offset / PACKET_DATA_LEN;
        if (acked_seq >= base && acked_seq < next_seq_num) {
          window[acked_seq % WINDOW_LEN].ack = true;
          while (base < next_seq_num && window[base % WINDOW_LEN].ack) {
            base++;
          }
        }
      }
    }

    // Timeout and resend packets
    uint64_t current_time = (uint64_t)time(NULL);
    for (size_t i = base; i < next_seq_num; i++) {
      size_t window_idx = i % WINDOW_LEN;
      if (!window[window_idx].ack && (current_time - window[window_idx].timestamp >= PACKET_TIMEOUT_SR_MS)) {
        sendPacket(socket, &window[window_idx].packet, dest);
        window[window_idx].timestamp = current_time;
      }
    }
  }

  // Reset socket to blocking mode
  mode = 0;
  ioctlsocket(socket, FIONBIO, &mode);

  return 0;
}

int sendFile(SOCKET socket, FileBuffer *buffer, const struct sockaddr_in *dest) {
  if (socket == INVALID_SOCKET || buffer == NULL || dest == NULL) return ERR_INVALID_ARG;

  // Send NAME packet
  Packet packet_name = {0};
  strncpy(packet_name.header, PACKET_HEADER_NAME, PACKET_HEADER_LEN - 1);
  packet_name.header[PACKET_HEADER_LEN - 1] = '\0';
  sprintf(packet_name.data, "%s", buffer->filename);
  packet_name.offset = 0;
  int send_r = sendAndWaitForAck(socket, &packet_name, dest);
  if (send_r != 0) return send_r;

  // Send SIZE packet
  Packet packet_size = {0};
  strncpy(packet_size.header, PACKET_HEADER_SIZE, PACKET_HEADER_LEN - 1);
  packet_size.header[PACKET_HEADER_LEN - 1] = '\0';
  sprintf(packet_size.data, "%zu", buffer->length);
  packet_size.offset = 1;
  send_r = sendAndWaitForAck(socket, &packet_size, dest);
  if (send_r != 0) return send_r;

  // Send HASH packets
  for (size_t i = 0; i < MD5_LEN; i++) {
    Packet packet_hash = {0};
    strncpy(packet_hash.header, PACKET_HEADER_HASH, PACKET_HEADER_LEN - 1);
    packet_hash.header[PACKET_HEADER_LEN - 1] = '\0';
    sprintf(packet_hash.data, "%d", buffer->md5[i]);
    packet_hash.offset = 2 + i;
    send_r = sendAndWaitForAck(socket, &packet_hash, dest);
    if (send_r != 0) return send_r;
  }

  // Send START packet
  Packet packet_start = {0};
  strncpy(packet_start.header, PACKET_HEADER_START, PACKET_HEADER_LEN - 1);
  packet_start.header[PACKET_HEADER_LEN - 1] = '\0';
  packet_start.offset = 2 + MD5_LEN;
  send_r = sendAndWaitForAck(socket, &packet_start, dest);
  if (send_r != 0) return send_r;

  // Send DATA packets
  size_t total_data_packets;
  send_r = sendFileData(socket, buffer, dest, &total_data_packets);
  if (send_r != 0) return send_r;

  // Send STOP packet
  Packet packet_stop = {0};
  strncpy(packet_stop.header, PACKET_HEADER_STOP, PACKET_HEADER_LEN - 1);
  packet_stop.header[PACKET_HEADER_LEN - 1] = '\0';
  packet_stop.offset = 2 + MD5_LEN + 1 + total_data_packets;
  send_r = sendAndWaitForAck(socket, &packet_stop, dest);
  if (send_r != 0) return send_r;

  return 0;
}

int receiveFile(SOCKET socket, struct sockaddr_in *from, FileBuffer *buffer) {
  if (socket == INVALID_SOCKET || buffer == NULL || from == NULL) return ERR_INVALID_ARG;

  // Initialize
  Packet packet = {0};
  int fromlen = sizeof(*from);
  uint8_t received_md5[MD5_LEN] = {0};
  int md5_position = 0;
  size_t total_packets = (buffer->size + PACKET_DATA_LEN - 1) / PACKET_DATA_LEN;
  size_t expected_seq_num = 0;
  size_t received_packets = 0;
  bool got_name = false;
  bool got_size = false;
  bool got_hash = false;
  bool got_start = false;
  bool got_stop = false;

  // Process metadata packets
  while (!got_name || !got_size || !got_hash || !got_start) {
    int retries = 0;
    bool valid_packet = false;

    while (!valid_packet && retries < 5) {
      int received = recvfrom(socket, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)from, &fromlen);
      if (received == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) continue; // No data to read, expected err
        return ERR_SOCKET_RECEIVE;
      }

      // Verify CRC
      uint32_t received_crc = ntohl(packet.crc32);
      uint32_t calculated_crc;
      int crc_r = computeCRC32((uint8_t *)packet.data, sizeof(packet.data), &calculated_crc);
      if (crc_r != 0) return crc_r;
      //
      if (received_crc == calculated_crc) {
        valid_packet = true;
      } else {
        Packet response = {0};
        strncpy(response.header, PACKET_HEADER_NACK, PACKET_HEADER_LEN - 1);
        response.header[PACKET_HEADER_LEN - 1] = '\0';
        response.offset = packet.offset;
        sendPacket(socket, &response, from);
        retries++;
      }
    }

    // Too many retries
    if (!valid_packet) return ERR_PACKET_CRC;

    // Send ACK for valid packet
    Packet response = {0};
    strncpy(response.header, PACKET_HEADER_ACK, sizeof(response.header) - 1);
    response.offset = packet.offset;
    sendPacket(socket, &response, from);

    // Process valid packet
    if (strcmp(packet.header, PACKET_HEADER_NAME) == 0) {
      got_name = true;
      strncpy(buffer->filename, packet.data, sizeof(buffer->filename) - 1);
    } else if (strcmp(packet.header, PACKET_HEADER_SIZE) == 0) {
      got_size = true;
      int fbAlloc_r = reallocateFileBuffer(buffer, atoll(packet.data));
      if (fbAlloc_r != 0) return fbAlloc_r;
    } else if (strcmp(packet.header, PACKET_HEADER_HASH) == 0) {
      if (md5_position < MD5_LEN) {
        received_md5[md5_position++] = atoi(packet.data);
        got_hash = md5_position == MD5_LEN;
      }
    } else if (strcmp(packet.header, PACKET_HEADER_START) == 0) {
      got_start = true;
    }
  }

  // Set socket to non-blocking
  u_long mode = 1;
  ioctlsocket(socket, FIONBIO, &mode);

  // Process DATA packets
  while (!got_stop) {
    int received = recvfrom(socket, (char *)&packet, sizeof(packet), 0, (struct sockaddr *)from, &fromlen);
    if (received == SOCKET_ERROR) {
      if (WSAGetLastError() == WSAEWOULDBLOCK) continue; // No data to read, expected err
      return ERR_SOCKET_RECEIVE;
    }

    // Verify CRC
    uint32_t received_crc = ntohl(packet.crc32);
    uint32_t calculated_crc;
    int crc_r = computeCRC32((uint8_t *)packet.data, sizeof(packet.data), &calculated_crc);
    if (crc_r != 0) return crc_r;
    //
    if (received_crc != calculated_crc) {
      Packet response = {0};
      strncpy(response.header, PACKET_HEADER_NACK, PACKET_HEADER_LEN - 1);
      response.header[PACKET_HEADER_LEN - 1] = '\0';
      response.offset = packet.offset;
      sendPacket(socket, &response, from);
      continue;
    }

    // Send ACK for valid packet
    Packet response = {0};
    strncpy(response.header, PACKET_HEADER_ACK, sizeof(response.header) - 1);
    response.offset = packet.offset;
    sendPacket(socket, &response, from);

    // Process valid packet
    if (strcmp(packet.header, PACKET_HEADER_DATA) == 0) {
      size_t seq_num = packet.offset / PACKET_DATA_LEN;
      if (seq_num == expected_seq_num && seq_num < total_packets) {
        received_packets++;
        expected_seq_num++;
        size_t data_length = min(sizeof(packet.data), buffer->size - packet.offset);
        memcpy(buffer->data + packet.offset, packet.data, data_length);
        if (packet.offset + data_length > buffer->length) {
          buffer->length = packet.offset + data_length;
        }
      }
    } else if (strcmp(packet.header, PACKET_HEADER_STOP) == 0) {
      got_stop = true;
    }
  }

  // Reset socket to blocking mode
  mode = 0;
  ioctlsocket(socket, FIONBIO, &mode);

  // Check if all packets were received
  if (received_packets != total_packets) {
    free(buffer->data);
    buffer->data = NULL;
    return ERR_SOCKET_RECEIVE;
  }

  // Verify MD5
  MD5Context md5_context;
  md5Init(&md5_context);
  md5Update(&md5_context, (unsigned char *)buffer->data, buffer->length);
  md5Finalize(&md5_context);
  uint8_t *calculated_md5 = md5_context.digest;
  bool md5_match = true;
  //
  for (int i = 0; i < MD5_LEN; i++) {
    if (calculated_md5[i] != received_md5[i]) {
      md5_match = false;
      break;
    }
  }
  //
  if (md5_match) {
    memcpy(buffer->md5, received_md5, MD5_LEN);
  } else {
    free(buffer->data);
    buffer->data = NULL;
    return ERR_PACKET_MD5;
  }

  return 0;
}