#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

int NET_Gai_strerror() {
    // 测试一些常见的错误代码
    int error_codes[] = {
        0,  // 0: Success
        EAI_AGAIN,  // Temporary failure in name resolution
        EAI_FAIL,   // Non-recoverable failure in name resolution
        EAI_MEMORY, // Memory allocation failure
        EAI_NONAME, // Name or service not known
        EAI_SERVICE, // Service not supported for socket type
        -1 // Invalid error code
    };

    size_t num_errors = sizeof(error_codes) / sizeof(error_codes[0]);

    // 遍历错误代码并打印对应的错误信息
    for (size_t i = 0; i < num_errors; i++) {
        int code = error_codes[i];
        const char *error_message = gai_strerror(code);
        printf("Error code %d: %s\n", code, error_message);
    }

    return 0;
}