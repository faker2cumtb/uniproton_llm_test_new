#ifndef FXMAC_NO_LWIP_H
#define FXMAC_NO_LWIP_H

#ifdef __cplusplus
extern "C" {
#endif

/** Main packet buffer struct */
struct fx_pbuf
{
    /** next pbuf in singly linked pbuf chain */
    struct fx_pbuf *next;

    /** pointer to the actual data in the buffer */
    void *payload;

    /** length of this buffer */
    u16 len;
};

u8 fx_pbuf_free(struct fx_pbuf *p);
struct fx_pbuf *fx_pbuf_alloc(u16 length);
void fx_pbuf_realloc(struct fx_pbuf *p, u16 new_len);
#ifdef __cplusplus
}
#endif

#endif // !