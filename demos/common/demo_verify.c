#include <stdio.h>
#include <key_verify/key_verify.h>
#include <demo.h>
#include <mica_service.h>

void verify_demo(void)
{
    mica_service_init();

    if (key_verify() < 0) {
        printf("verify fail. stop running.\n");
        return;
    }
    printf("verify successfully. keep running.\n");
}
