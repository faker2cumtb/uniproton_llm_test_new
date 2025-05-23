#ifndef _MICA_SERVICE_H_
#define _MICA_SERVICE_H_

#include <stdint.h>
#include "prt_buildef.h"

#ifndef OS_OPTION_MICA
#error "OS_OPTION_MICA is not defined\n"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int mica_service_init(void);
void mica_service_uninit(void);

#if defined(OS_OPTION_POWER_DOWN_PROTECT)
uint32_t powerdown_prot_service_init(void);
void set_powerdown_prot_ipi_handler(void (*handler)(void));
uint32_t write_powerdown_prot_data(uint8_t *data, uint32_t len);
#endif /* defined(OS_OPTION_POWER_DOWN_PROTECT) */

#ifdef __cplusplus
}
#endif

#endif /* _MICA_SERVICE_H_ */