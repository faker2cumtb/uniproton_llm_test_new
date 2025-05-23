#include "ftypes.h"
#include "fxmac_port_nolwip.h"
#include "stdlib.h"
#include "stdio.h"

u8 fx_pbuf_free(struct fx_pbuf *p)
{
    u32 ret = 0;
    if(p == NULL){
        printf("fx_pbuf_free NULL\n");
        return -1;
    }

    free(p->payload);
    free(p);
    return ret;
}

struct fx_pbuf *fx_pbuf_alloc(u16 length)
{
    struct fx_pbuf *allocpbuf = (struct fx_pbuf *)malloc(sizeof(struct fx_pbuf));
    if (allocpbuf == NULL) {
        printf("alloc pbuf err\n");
        return NULL;
    }

    allocpbuf->payload = malloc(length);
    if (allocpbuf->payload == NULL){
        printf("alloc pbuf->payload err\n");
        return NULL;
    }

    allocpbuf->next = NULL;
    allocpbuf->len = length;

    return allocpbuf;
}

void fx_pbuf_realloc(struct fx_pbuf *p, u16 new_len)
{
    if (p == NULL) {
        printf("fx_pbuf_realloc: p != NULL\n");
    }
    /* desired length larger than current length? */
    if (new_len > p->len)
    {
        /* enlarging not yet supported */
        printf("fx_pbuf_realloc err length\n");
        return;
    }

    p->len = new_len;
}
