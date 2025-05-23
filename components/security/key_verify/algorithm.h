#ifndef __BOARD_VERIFY_H__
#define __BOARD_VERIFY_H__

#define FAILURE 1
#define SUCCESS 0

void xor_encrypt(char *input, const char *key);
void hash_string(const char *input, unsigned char output[]);
int sub_start(const char* str, size_t start, char* buffer, size_t buffer_size);
int sub_end(const char* str, size_t end, char* buffer, size_t buffer_size);

int combination_algorithm(char* buf, unsigned int buf_len);
#define ALGO "mac0.hash()+mmc0.xor("sdfsdf")+mmc0.substart(5)"

#endif