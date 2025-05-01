#define _POSIX_C_SOURCE 200112L

#define MAX_NUM_LEN 20

#define SERIAL_PORT_BASE 0xffffc000
#define SERP_RX_ST_REG_o 0x00
#define SERP_RX_ST_REG_READY_m 0x1
#define SERP_RX_DATA_REG_o 0x04
#define SERP_TX_ST_REG_o 0x08
#define SERP_TX_ST_REG_READY_m 0x1
#define SERP_TX_DATA_REG_o 0x0c

void receive_number(char *n, int *len);
void add_numbers(char *a, int a_len, char *b, int b_len, char *c, int *c_len);
void send_number(char *n);

int main(int argc, char *argv[]) {
  char a[MAX_NUM_LEN];
  int a_len = 0;
  receive_number(a, &a_len);

  char b[MAX_NUM_LEN];
  int b_len = 0;
  receive_number(b, &b_len);

  char c[MAX_NUM_LEN];
  int c_len = 0;
  add_numbers(a, a_len, b, b_len, c, &c_len);

  send_number(c);

  return 0;
}

void receive_number(char *n, int *len) {
  volatile int *receiver_status = (volatile int *)(SERIAL_PORT_BASE + SERP_RX_ST_REG_o);
  volatile int *receiver_data = (volatile int *)(SERIAL_PORT_BASE + SERP_RX_DATA_REG_o);

  while (*len < MAX_NUM_LEN - 1) {
    // Reciever not ready, wait
    while (!(*receiver_status & SERP_RX_ST_REG_READY_m))
      ;

    // Read char
    char c = *receiver_data & 0xFF;
    if (c == '\n') break;
    n[(*len)++] = c;
  }

  // Terminate the number
  n[*len] = '\0';
}

void add_numbers(char *a, int a_len, char *b, int b_len, char *c, int *c_len) {
  int carry = 0;
  int i = a_len - 1;
  int j = b_len - 1;
  int k = 0;

  // Process digits from right to left
  while (i >= 0 || j >= 0 || carry) {
    int a_digit = (i >= 0) ? a[i] - '0' : 0;
    int b_digit = (j >= 0) ? b[j] - '0' : 0;

    int sum = a_digit + b_digit + carry;
    carry = sum / 10;
    int digit_c = sum % 10;

    c[k++] = digit_c + '0';
    i--;
    j--;
  }

  // Terminate the number
  c[k] = '\0';
  *c_len = k;

  // Reverse the number
  for (i = 0; i < k / 2; i++) {
    char temp = c[i];
    c[i] = c[k - i - 1];
    c[k - i - 1] = temp;
  }
}

void send_number(char *n) {
  volatile int *transmitter_status = (volatile int *)(SERIAL_PORT_BASE + SERP_TX_ST_REG_o);
  volatile int *transmitter_data = (volatile int *)(SERIAL_PORT_BASE + SERP_TX_DATA_REG_o);
  int i = 0;

  while (n[i] != '\0') {
    // Transmitter not ready, wait
    while (!(*transmitter_status & SERP_TX_ST_REG_READY_m))
      ;

    // Send char
    *transmitter_data = n[i++];
  }

  // Terminate the number
  while (!(*transmitter_status & SERP_TX_ST_REG_READY_m))
    ;
  *transmitter_data = '\n';
}
