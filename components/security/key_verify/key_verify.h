
#ifndef __BOARD_VERIFY_H__
#define __BOARD_VERIFY_H__

#include <prt_buildef.h>

#ifndef OS_OPTION_MICA
#error "OS_OPTION_MICA is not defined\n"
#endif

int key_verify(void);

#endif