#ifndef __GET_UUID_H_
#define __GET_UUID_H_

#define UUID_BUFFER_LEN 20

int get_cpu_uuid(char* uuid_buf, unsigned int buf_len, unsigned int index);
int get_mac_uuid(char* uuid_buf, unsigned int buf_len, unsigned int index);
int get_mmc_uuid(char* uuid_buf, unsigned int buf_len, unsigned int index);
void format_uuid(const char *uuid_in, char *uuid_out, unsigned int format_len);

#endif