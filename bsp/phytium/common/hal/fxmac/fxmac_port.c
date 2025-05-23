/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: fxmac_port.c
 * Date: 2022-11-01 14:59:22
 * LastEditTime: 2022-11-01 14:59:22
 * Description:  This file is xmac portable code for lwip port input,output,status check.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huanghe      2022/11/3            first release
 *  1.1   liuzhihong    2023/4/11            jumbo support
 */
#include "fparameters.h"
#include "fassert.h"
#include "faarch.h"
#include "fcache.h"
#include "fxmac_port.h"
#include "fxmac.h"
#include "ftypes.h"
#include "fxmac_bdring.h"
#include "eth_ieee_reg.h"
#include "fdebug.h"
#include "prt_sys.h"
#include "prt_tick.h"
#include "prt_config.h"
#include "prt_hwi.h"
#include "cpu_config.h"
#include "securec.h"
#include "stdlib.h"
#include "hwi_router.h"
#include "prt_mem.h"
#include "fxmac_port_nolwip.h"
#include "fxmac_phy.h"
#include "rtconfig.h"

#define FXMAC_PORT_XMAC_DEBUG_TAG "FXMAC_PORT_XMAC"
#define FXMAC_PORT_XMAC_PRINT_E(format, ...) FT_DEBUG_PRINT_E(FXMAC_PORT_XMAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_PORT_XMAC_PRINT_I(format, ...) FT_DEBUG_PRINT_I(FXMAC_PORT_XMAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_PORT_XMAC_PRINT_D(format, ...) FT_DEBUG_PRINT_D(FXMAC_PORT_XMAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_PORT_XMAC_PRINT_W(format, ...) FT_DEBUG_PRINT_W(FXMAC_PORT_XMAC_DEBUG_TAG, format, ##__VA_ARGS__)

#define FXMAC_BD_TO_INDEX(ringptr, bdptr) \
    (((uintptr)bdptr - (uintptr)(ringptr)->base_bd_addr) / (ringptr)->separation)

static void FXmacInitOnError(FXmacPort *instance_p);
static void FXmacSetupIsr(FXmacPort *instance_p);

static FXmacPort fxmac_port_instance[FXMAC_NUM] =
{
    [FXMAC0_ID] = {{{0}}},
    [FXMAC1_ID] = {{{0}}},
#if defined(FXMAC2_ID)
    [FXMAC2_ID] = {{{0}}},
#endif
#if defined(FXMAC3_ID)
    [FXMAC3_ID] = {{{0}}},
#endif
};

/* queue */
void FXmacQueueInit(PqQueue *q)
{
    FASSERT(q != NULL);
    q->head = q->tail = q->len = 0;
}

int FXmacPqEnqueue(PqQueue *q, void *p)
{
    if (q->len == PQ_QUEUE_SIZE)
        return -1;

    q->data[q->head] = (uintptr)p;
    q->head = (q->head + 1) % PQ_QUEUE_SIZE;
    q->len++;

    return 0;
}

void *FXmacPqDequeue(PqQueue *q)
{
    int ptail;

    if (q->len == 0)
        return NULL;

    ptail = q->tail;
    q->tail = (q->tail + 1) % PQ_QUEUE_SIZE;
    q->len--;

    return (void *)q->data[ptail];
}

int FXmacPqQlength(PqQueue *q)
{
    return q->len;
}

/* dma */

/**
 * @name:  IsTxSpaceAvailable
 * @msg:   获取当前bdring 剩余计数
 * @param {ethernetif} *ethernetif_p
 * @return {*} 返回
 */
static u32 IsTxSpaceAvailable(FXmacPort *instance_p)
{
    FXmacBdRing *txring;
    u32 freecnt = 0;
    FASSERT(instance_p != NULL);

    txring = &(FXMAC_GET_TXRING(instance_p->instance));

    /* tx space is available as long as there are valid BD's */
    freecnt = FXMAC_BD_RING_GET_FREE_CNT(txring);
    return freecnt;
}

/**
 * @name: FXmacProcessSentBds
 * @msg:   释放发送队列q参数
 * @return {*}
 * @param {ethernetif} *ethernetif_p
 * @param {FXmacBdRing} *txring
 */
void FXmacProcessSentBds(FXmacPort *instance_p, FXmacBdRing *txring)
{
    FXmacBd *txbdset;
    FXmacBd *curbdpntr;
    u32 n_bds;
    FError status;
    u32 n_pbufs_freed = 0;
    u32 bdindex = 0;
    struct fx_pbuf *p;
    u32 *temp;

    while (1)
    {
        /* obtain processed BD's */
        n_bds = FXmacBdRingFromHwTx(txring, FXMAX_TX_PBUFS_LENGTH, &txbdset);
        if (n_bds == 0)
        {
            return;
        }
        /* free the processed BD's */
        n_pbufs_freed = n_bds;
        curbdpntr = txbdset;
        while (n_pbufs_freed > 0)
        {
            bdindex = FXMAC_BD_TO_INDEX(txring, curbdpntr);
            temp = (u32 *)curbdpntr;
            *temp = 0; /* Word 0 */
            temp++;

            if (bdindex == (FXMAX_TX_PBUFS_LENGTH - 1))
            {
                *temp = 0xC0000000; /* Word 1 ,used/Wrap – marks last descriptor in transmit buffer descriptor list.*/
            }
            else
            {
                *temp = 0x80000000; /* Word 1 , Used – must be zero for GEM to read data to the transmit buffer.*/
            }
            DSB();

            p = (struct fx_pbuf *)instance_p->buffer.tx_pbufs_storage[bdindex];

            if (p != NULL)
            {
                fx_pbuf_free(p);
            }

            instance_p->buffer.tx_pbufs_storage[bdindex] = (uintptr)NULL;
            curbdpntr = FXMAC_BD_RING_NEXT(txring, curbdpntr);
            n_pbufs_freed--;
            if(instance_p->device->tx_complete != RT_NULL){
                instance_p->device->tx_complete(instance_p->device,RT_NULL);
            }
            DSB();
        }

        status = FXmacBdRingFree(txring, n_bds, txbdset);
        if (status != FT_SUCCESS)
        {
            FXMAC_PORT_XMAC_PRINT_E("Failure while freeing in Tx Done ISR\r\n");
        }
    }
    return;
}

void FXmacSendHandler(void *arg)
{
    FXmacPort *instance_p;
    FXmacBdRing *txringptr;
    u32 regval;

    instance_p = (FXmacPort *)arg;
    txringptr = &(FXMAC_GET_TXRING(instance_p->instance));
    regval = FXMAC_READREG32(instance_p->instance.config.base_address, FXMAC_TXSR_OFFSET);
    FXMAC_WRITEREG32(instance_p->instance.config.base_address, FXMAC_TXSR_OFFSET, regval); /* 清除中断状态位来停止中断 */

    /* If Transmit done interrupt is asserted, process completed BD's */
    FXmacProcessSentBds(instance_p, txringptr);
}

FError FXmacSgsend(FXmacPort *instance_p, struct fx_pbuf *p)
{
    struct fx_pbuf *q;
    u32 n_pbufs;
    FXmacBd *txbdset, *txbd, *last_txbd = NULL;
    FXmacBd *temp_txbd;
    FError status;
    FXmacBdRing *txring;
    u32 bdindex = 0;
    u32 max_fr_size;
    
    txring = &(FXMAC_GET_TXRING(instance_p->instance));

    /* first count the number of pbufs */
    for (q = p, n_pbufs = 0; q != NULL; q = q->next)
        n_pbufs++;

    /* obtain as many BD's */
    status = FXmacBdRingAlloc(txring, n_pbufs, &txbdset);
    if (status != FT_SUCCESS)
    {
        FXMAC_PORT_XMAC_PRINT_E("sgsend: Error allocating TxBD\r\n");
        return ERR_GENERAL;
    }

    for (q = p, txbd = txbdset; q != NULL; q = q->next)
    {
        bdindex = FXMAC_BD_TO_INDEX(txring, txbd);

        if (instance_p->buffer.tx_pbufs_storage[bdindex])
        {
            FXMAC_PORT_XMAC_PRINT_E("PBUFS not available\r\n");
            goto err;
        }

        /* Send the data from the pbuf to the interface, one pbuf at a
           time. The size of the data in each pbuf is kept in the ->len
           variable. */
        FCacheDCacheFlushRange((uintptr)q->payload, (uintptr)q->len);
        FXMAC_BD_SET_ADDRESS_TX(txbd, (uintptr)q->payload);

        if (instance_p->feature & FXMAC_PORT_CONFIG_JUMBO)
        {
            max_fr_size = FXMAC_MAX_FRAME_SIZE_JUMBO;
        }
        else
        {
            max_fr_size = FXMAC_MAX_FRAME_SIZE;
        }

        if (q->len > max_fr_size)
        {
            FXMAC_BD_SET_LENGTH(txbd, max_fr_size & 0x3FFF);
        }
        else
        {
            FXMAC_BD_SET_LENGTH(txbd, q->len & 0x3FFF);
        }
            

        instance_p->buffer.tx_pbufs_storage[bdindex] = (uintptr)q;

        // pbuf_ref(q);
        last_txbd = txbd;
        FXMAC_BD_CLEAR_LAST(txbd);
        txbd = FXMAC_BD_RING_NEXT(txring, txbd);
    }
    
    FXMAC_BD_SET_LAST(last_txbd);
    /* The bdindex always points to the first free_head in tx_bdrings */
    if ((instance_p->instance).config.caps & FXMAC_CAPS_TAILPTR)
    {   
         bdindex = FXMAC_BD_TO_INDEX(txring, txbd);
    }
    /* For fragmented packets, remember the 1st BD allocated for the 1st
       packet fragment. The used bit for this BD should be cleared at the end
       after clearing out used bits for other fragments. For packets without
       just remember the allocated BD. */
    temp_txbd = txbdset;
    txbd = txbdset;
    txbd = FXMAC_BD_RING_NEXT(txring, txbd);
    q = p->next;
    for (; q != NULL; q = q->next)
    {
        FXMAC_BD_CLEAR_TX_USED(txbd);
        DSB();
        txbd = FXMAC_BD_RING_NEXT(txring, txbd);
    }
    FXMAC_BD_CLEAR_TX_USED(temp_txbd);
    DSB();

    status = FXmacBdRingToHw(txring, n_pbufs, txbdset);
    if (status != FT_SUCCESS)
    {
        FXMAC_PORT_XMAC_PRINT_E("sgsend: Error submitting TxBD\r\n");
        goto err;
    }

    if ((instance_p->instance).config.caps & FXMAC_CAPS_TAILPTR)
    {   
        FXMAC_WRITEREG32((instance_p->instance).config.base_address, FXMAC_TAIL_QUEUE(0), BIT(31)|bdindex);
    }

    /* Start transmit */
    FXMAC_WRITEREG32((instance_p->instance).config.base_address,
                     FXMAC_NWCTRL_OFFSET,
                     (FXMAC_READREG32(instance_p->instance.config.base_address,
                                      FXMAC_NWCTRL_OFFSET) |
                      FXMAC_NWCTRL_STARTTX_MASK));

    return FT_SUCCESS;
err:
    FXmacBdRingUnAlloc(txring, n_pbufs, txbdset);
    return status;
}

void SetupRxBds(FXmacPort *instance_p, FXmacBdRing *rxring)
{
    FXmacBd *rxbd;
    FError status;
    struct fx_pbuf *p;
    u32 freebds;
    u32 bdindex = 0;
    u32 *temp;
    freebds = FXMAC_BD_RING_GET_FREE_CNT(rxring);
    while (freebds > 0)
    {
        freebds--;

        if (instance_p->feature & FXMAC_PORT_CONFIG_JUMBO)
        {
            p = fx_pbuf_alloc(FXMAC_MAX_FRAME_SIZE_JUMBO);
        }
        else
        {
            p = fx_pbuf_alloc(FXMAC_MAX_FRAME_SIZE);
        }

        if (!p)
        {
#if LINK_STATS
            lwip_stats.link.memerr++;
            lwip_stats.link.drop++;
#endif
            FXMAC_PORT_XMAC_PRINT_E("unable to alloc fx_pbuf in recv_handler\r\n");
            return;
        }
        status = FXmacBdRingAlloc(rxring, 1, &rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_PORT_XMAC_PRINT_E("SetupRxBds: Error allocating RxBD\r\n");
            fx_pbuf_free(p);
            return;
        }
        status = FXmacBdRingToHw(rxring, 1, rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_PORT_XMAC_PRINT_E("Error committing RxBD to hardware: ");
            if (status == FXMAC_ERR_SG_LIST)
            {
                FXMAC_PORT_XMAC_PRINT_E("XST_DMA_SG_LIST_ERROR: this function was called out of sequence with FXmacBdRingAlloc()\r\n");
            }
            else
            {
                FXMAC_PORT_XMAC_PRINT_E("set of BDs was rejected because the first BD did not have its start-of-packet bit set, or the last BD did not have its end-of-packet bit set, or any one of the BD set has 0 as length value\r\n");
            }

            fx_pbuf_free(p);
            FXmacBdRingUnAlloc(rxring, 1, rxbd);
            return;
        }

        if (instance_p->feature & FXMAC_PORT_CONFIG_JUMBO)
        {
            FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)FXMAC_MAX_FRAME_SIZE_JUMBO);
        }
        else
        {
            FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)FXMAC_MAX_FRAME_SIZE);
        }

        bdindex = FXMAC_BD_TO_INDEX(rxring, rxbd);
        temp = (u32 *)rxbd;
        if (bdindex == (FXMAX_RX_PBUFS_LENGTH - 1))
        {
            *temp = 0x00000002; /* Mask last descriptor in receive buffer list */
        }
        else
        {
            *temp = 0;
        }
        temp++;
        *temp = 0;
        DSB();
        FXMAC_BD_SET_ADDRESS_RX(rxbd, (uintptr)p->payload);
        instance_p->buffer.rx_pbufs_storage[bdindex] = (uintptr)p;
    }
}

void FXmacRecvHandler(void *arg)
{
    struct fx_pbuf *p;
    FXmacBd *rxbdset, *curbdptr;
    FXmacBdRing *rxring;
    volatile u32 bd_processed;
    u32 rx_bytes, k;
    u32 bdindex = 0;
    u32 regval;
    FXmacPort *instance_p;
    FASSERT(arg != NULL);

    instance_p = (FXmacPort *)arg;
    rxring = &FXMAC_GET_RXRING(instance_p->instance);

    /* If Reception done interrupt is asserted, call RX call back function
     to handle the processed BDs and then raise the according flag.*/
    regval = FXMAC_READREG32(instance_p->instance.config.base_address, FXMAC_RXSR_OFFSET);
    FXMAC_WRITEREG32(instance_p->instance.config.base_address, FXMAC_RXSR_OFFSET, regval);

    while (1)
    {
        bd_processed = FXmacBdRingFromHwRx(rxring, FXMAX_RX_PBUFS_LENGTH, &rxbdset);
        if (bd_processed <= 0)
        {
            break;
        }

        for (k = 0, curbdptr = rxbdset; k < bd_processed; k++)
        {
            bdindex = FXMAC_BD_TO_INDEX(rxring, curbdptr);
            p = (struct fx_pbuf *)instance_p->buffer.rx_pbufs_storage[bdindex];
            /*
             * Adjust the buffer size to the actual number of bytes received.
             */
            if (instance_p->feature & FXMAC_PORT_CONFIG_JUMBO)
            {
                rx_bytes = FXMAC_GET_RX_FRAME_SIZE(curbdptr);
            }
            else
            {
                rx_bytes = FXMAC_BD_GET_LENGTH(curbdptr);
            }

            fx_pbuf_realloc(p, rx_bytes);
           /*  
            The value of hash_match indicates the hash result of the received packet 
               0: No hash match 
               1: Unicast hash match 
               2: Multicast hash match
               3: Reserved, the value is not legal
           */
            FXMAC_PORT_XMAC_PRINT_D("hash_match is %d\r\n", FXMAC_BD_GET_HASH_MATCH(curbdptr));
    
            /* Invalidate RX frame before queuing to handle
             * L1 cache prefetch conditions on any architecture.
             */
            FCacheDCacheInvalidateRange((uintptr)p->payload, rx_bytes);

            /* store it in the receive queue,
             * where it'll be processed by a different handler
             */
            if (FXmacPqEnqueue(&instance_p->recv_q, (void *)p) < 0)
            {
#if LINK_STATS
                lwip_stats.link.memerr++;
                lwip_stats.link.drop++;
#endif
                fx_pbuf_free(p);
            }
            instance_p->buffer.rx_pbufs_storage[bdindex] = (uintptr)NULL;
            curbdptr = FXMAC_BD_RING_NEXT(rxring, curbdptr);
            if(instance_p->device->rx_indicate != RT_NULL){
                instance_p->device->rx_indicate(instance_p->device,RT_NULL);
            }
        }

        /* free up the BD's */
        FXmacBdRingFree(rxring, bd_processed, rxbdset);
        SetupRxBds(instance_p, rxring);
    }

    return;
}

void CleanDmaTxdescs(FXmacPort *instance_p)
{
    FXmacBd bdtemplate;
    FXmacBdRing *txringptr;

    txringptr = &FXMAC_GET_TXRING((instance_p->instance));
    FXMAC_BD_CLEAR(&bdtemplate);
    FXMAC_BD_SET_STATUS(&bdtemplate, FXMAC_TXBUF_USED_MASK);

    FXmacBdRingCreate(txringptr, (uintptr)instance_p->buffer.tx_bdspace,
                      (uintptr)instance_p->buffer.tx_bdspace, BD_ALIGNMENT,
                      FXMAX_TX_PBUFS_LENGTH);

    FXmacBdRingClone(txringptr, &bdtemplate, FXMAC_SEND);
}

FError FXmacInitDma(FXmacPort *instance_p)
{
    FXmacBd bdtemplate;
    FXmacBdRing *rxringptr, *txringptr;
    FXmacBd *rxbd;
    struct fx_pbuf *p;
    FError status;
    int i;
    u32 bdindex = 0;
    u32 *temp;

    
    /*
     * The BDs need to be allocated in uncached memory. Hence the 1 MB
     * address range allocated for Bd_Space is made uncached
     * by setting appropriate attributes in the translation table.
     * The Bd_Space is aligned to 1MB and has a size of 1 MB. This ensures
     * a reserved uncached area used only for BDs.
     */

    rxringptr = &FXMAC_GET_RXRING(instance_p->instance);
    txringptr = &FXMAC_GET_TXRING(instance_p->instance);
    FXMAC_PORT_XMAC_PRINT_I("rxringptr: 0x%08x\r\n", rxringptr);
    FXMAC_PORT_XMAC_PRINT_I("txringptr: 0x%08x\r\n", txringptr);

    FXMAC_PORT_XMAC_PRINT_I("rx_bdspace: %p \r\n", instance_p->buffer.rx_bdspace);
    FXMAC_PORT_XMAC_PRINT_I("tx_bdspace: %p \r\n", instance_p->buffer.tx_bdspace);

    /* Setup RxBD space. */
    FXMAC_BD_CLEAR(&bdtemplate);

    /* Create the RxBD ring */
    status = FXmacBdRingCreate(rxringptr, (uintptr)instance_p->buffer.rx_bdspace,
                               (uintptr)instance_p->buffer.rx_bdspace, BD_ALIGNMENT,
                               FXMAX_RX_PBUFS_LENGTH);

    if (status != FT_SUCCESS)
    {
        FXMAC_PORT_XMAC_PRINT_E("Error setting up RxBD space\r\n");
        return ERR_IF;
    }

    status = FXmacBdRingClone(rxringptr, &bdtemplate, FXMAC_RECV);
    if (status != FT_SUCCESS)
    {
        FXMAC_PORT_XMAC_PRINT_E("Error initializing RxBD space\r\n");
        return ERR_IF;
    }

    FXMAC_BD_CLEAR(&bdtemplate);
    FXMAC_BD_SET_STATUS(&bdtemplate, FXMAC_TXBUF_USED_MASK);

    /* Create the TxBD ring */
    status = FXmacBdRingCreate(txringptr, (uintptr)instance_p->buffer.tx_bdspace,
                               (uintptr)instance_p->buffer.tx_bdspace, BD_ALIGNMENT,
                               FXMAX_TX_PBUFS_LENGTH);

    if (status != FT_SUCCESS)
    {
        return ERR_IF;
    }

    /* We reuse the bd template, as the same one will work for both rx and tx. */
    status = FXmacBdRingClone(txringptr, &bdtemplate, FXMAC_SEND);
    if (status != FT_SUCCESS)
    {
        return ERR_IF;
    }

    /*
     * Allocate RX descriptors, 1 RxBD at a time.
     */
    FXMAC_PORT_XMAC_PRINT_I("Allocate RX descriptors, 1 RxBD at a time.");
    for (i = 0; i < FXMAX_RX_PBUFS_LENGTH; i++)
    {
        if (instance_p->feature & FXMAC_PORT_CONFIG_JUMBO)
        {
            PRT_Printf("FXMAC_PORT_CONFIG_JUMBO\n");
            p = fx_pbuf_alloc(FXMAC_MAX_FRAME_SIZE_JUMBO);
        }
        else
        {
            p = fx_pbuf_alloc(FXMAC_MAX_FRAME_SIZE);
        }

        if (!p)
        {
#if LINK_STATS
            lwip_stats.link.memerr++;
            lwip_stats.link.drop++;
#endif
            FXMAC_PORT_XMAC_PRINT_E("unable to alloc fx_pbuf in InitDma\r\n");
            return ERR_IF;
        }
        status = FXmacBdRingAlloc(rxringptr, 1, &rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_PORT_XMAC_PRINT_E("InitDma: Error allocating RxBD\r\n");
            fx_pbuf_free(p);
            return ERR_IF;
        }
        /* Enqueue to HW */
        status = FXmacBdRingToHw(rxringptr, 1, rxbd);
        if (status != FT_SUCCESS)
        {
            FXMAC_PORT_XMAC_PRINT_E("Error: committing RxBD to HW\r\n");
            fx_pbuf_free(p);
            FXmacBdRingUnAlloc(rxringptr, 1, rxbd);
            return ERR_IF;
        }

        bdindex = FXMAC_BD_TO_INDEX(rxringptr, rxbd);
        temp = (u32 *)rxbd;
        *temp = 0;
        if (bdindex == (FXMAX_RX_PBUFS_LENGTH - 1))
        {
            *temp = 0x00000002; /* Marks last descriptor in receive buffer descriptor list */
        }
        temp++;
        *temp = 0; /* Clear word 1 in  descriptor */
        DSB();

        if (instance_p->feature & FXMAC_PORT_CONFIG_JUMBO)
        {
            FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)FXMAC_MAX_FRAME_SIZE_JUMBO);
        }
        else
        {
            FCacheDCacheInvalidateRange((uintptr)p->payload, (uintptr)FXMAC_MAX_FRAME_SIZE);
        }
        FXMAC_BD_SET_ADDRESS_RX(rxbd, (uintptr)p->payload);

        instance_p->buffer.rx_pbufs_storage[bdindex] = (uintptr)p;
    }
    
    FXmacSetQueuePtr(&(instance_p->instance), instance_p->instance.tx_bd_queue.bdring.phys_base_addr, 0, (u16)FXMAC_SEND);
    FXmacSetQueuePtr(&(instance_p->instance), instance_p->instance.rx_bd_queue.bdring.phys_base_addr, 0, (u16)FXMAC_RECV);
    
    if ((instance_p->instance).config.caps & FXMAC_CAPS_TAILPTR)
    {   
        FXMAC_WRITEREG32((instance_p->instance).config.base_address, FXMAC_TAIL_QUEUE(0), BIT(31)|0);
    }

    return 0;
}

static void FreeOnlyTxPbufs(FXmacPort *instance_p)
{
    u32 index;
    struct fx_pbuf *p;

    for (index = 0; index < (FXMAX_TX_PBUFS_LENGTH); index++)
    {
        if (instance_p->buffer.tx_pbufs_storage[index] != 0)
        {
            p = (struct fx_pbuf *)instance_p->buffer.tx_pbufs_storage[index];
            fx_pbuf_free(p);
            instance_p->buffer.tx_pbufs_storage[index] = (uintptr)NULL;
        }
    }
}

static void FreeOnlyRxPbufs(FXmacPort *instance_p)
{
    u32 index;
    struct fx_pbuf *p;

    for (index = 0; index < (FXMAX_RX_PBUFS_LENGTH); index++)
    {
        if (instance_p->buffer.rx_pbufs_storage[index] != 0)
        {
            p = (struct fx_pbuf *)instance_p->buffer.rx_pbufs_storage[index];
            fx_pbuf_free(p);
            instance_p->buffer.rx_pbufs_storage[index] = (uintptr)NULL;
        }
    }
}


static void FreeTxRxPbufs(FXmacPort *instance_p)
{
    u32 rx_queue_len = 0;
    struct fx_pbuf *p;
    /* first :free PqQueue data */

    rx_queue_len = FXmacPqQlength(&instance_p->recv_q);
    
    while (rx_queue_len)
    {
        /* return one packet from receive q */
        p = (struct fx_pbuf *)FXmacPqDequeue(&instance_p->recv_q);
        fx_pbuf_free(p);
        FXMAC_PORT_XMAC_PRINT_W("delete queue %p", p);
        rx_queue_len--;
    }

    FreeOnlyTxPbufs(instance_p);
    FreeOnlyRxPbufs(instance_p);
}


/* Reset Tx and Rx DMA pointers after FXmacStop */
void ResetDma(FXmacPort *instance_p)
{
    FXmacBdRing *txringptr = &FXMAC_GET_TXRING(instance_p->instance);
    FXmacBdRing *rxringptr = &FXMAC_GET_RXRING(instance_p->instance);

    FXmacBdringPtrReset(txringptr, instance_p->buffer.tx_bdspace);
    FXmacBdringPtrReset(rxringptr, instance_p->buffer.rx_bdspace);

    FXmacSetQueuePtr(&(instance_p->instance), instance_p->instance.tx_bd_queue.bdring.phys_base_addr, 0, (u16)FXMAC_SEND);
    FXmacSetQueuePtr(&(instance_p->instance), instance_p->instance.rx_bd_queue.bdring.phys_base_addr, 0, (u16)FXMAC_RECV);
}

/* interrupt */
static void FXmacHandleDmaTxError(FXmacPort *instance_p)
{
    s32_t status = FT_SUCCESS;
    u32 dmacrreg;

    FreeTxRxPbufs(instance_p);
    status = FXmacCfgInitialize(&instance_p->instance, &instance_p->instance.config);

    if (status != FT_SUCCESS)
    {
        FXMAC_PORT_XMAC_PRINT_E("In %s:EmacPs Configuration Failed....\r\n", __func__);
    }

    /* initialize the mac */
    FXmacInitOnError(instance_p); /* need to set mac filter address */
    dmacrreg = FXMAC_READREG32(instance_p->instance.config.base_address, FXMAC_DMACR_OFFSET);
    dmacrreg = dmacrreg | (FXMAC_DMACR_ORCE_DISCARD_ON_ERR_MASK); /* force_discard_on_err */
    FXMAC_WRITEREG32(instance_p->instance.config.base_address, FXMAC_DMACR_OFFSET, dmacrreg);
    FXmacSetupIsr(instance_p);
    FXmacInitDma(instance_p);

    FXmacStart(&instance_p->instance);
}

void FXmacHandleTxErrors(FXmacPort *instance_p)
{
    u32 netctrlreg;

    netctrlreg = FXMAC_READREG32(instance_p->instance.config.base_address,
                                 FXMAC_NWCTRL_OFFSET);
    netctrlreg = netctrlreg & (~FXMAC_NWCTRL_TXEN_MASK);
    FXMAC_WRITEREG32(instance_p->instance.config.base_address,
                     FXMAC_NWCTRL_OFFSET, netctrlreg);
    FreeOnlyTxPbufs(instance_p);

    CleanDmaTxdescs(instance_p);
    netctrlreg = FXMAC_READREG32(instance_p->instance.config.base_address, FXMAC_NWCTRL_OFFSET);
    netctrlreg = netctrlreg | (FXMAC_NWCTRL_TXEN_MASK);
    FXMAC_WRITEREG32(instance_p->instance.config.base_address, FXMAC_NWCTRL_OFFSET, netctrlreg);
}

void FXmacErrorHandler(void *arg, u8 direction, u32 error_word)
{
    FXmacBdRing *txring;
    FXmacPort *instance_p;

    instance_p = (FXmacPort *)(arg);
    txring = &FXMAC_GET_TXRING((instance_p->instance));

    if (error_word != 0)
    {
        switch (direction)
        {
        case FXMAC_RECV:
            if (error_word & FXMAC_RXSR_HRESPNOK_MASK)
            {
                FXMAC_PORT_XMAC_PRINT_E("Receive DMA error\r\n");
                FXmacHandleDmaTxError(instance_p);
            }
            if (error_word & FXMAC_RXSR_RXOVR_MASK)
            {
                FXMAC_PORT_XMAC_PRINT_E("Receive over run\r\n");
                FXmacRecvHandler(arg);
            }
            if (error_word & FXMAC_RXSR_BUFFNA_MASK)
            {
                FXMAC_PORT_XMAC_PRINT_E("Receive buffer not available\r\n");
                FXmacRecvHandler(arg);
            }
            break;
        case FXMAC_SEND:
            if (error_word & FXMAC_TXSR_HRESPNOK_MASK)
            {
                FXMAC_PORT_XMAC_PRINT_E("Transmit DMA error\r\n");
                FXmacHandleDmaTxError(instance_p);
            }
            if (error_word & FXMAC_TXSR_URUN_MASK)
            {
                FXMAC_PORT_XMAC_PRINT_E("Transmit under run\r\n");
                FXmacHandleTxErrors(instance_p);
            }
            if (error_word & FXMAC_TXSR_BUFEXH_MASK)
            {
                FXMAC_PORT_XMAC_PRINT_E("Transmit buffer exhausted\r\n");
                FXmacHandleTxErrors(instance_p);
            }
            if (error_word & FXMAC_TXSR_RXOVR_MASK)
            {
                FXMAC_PORT_XMAC_PRINT_E("Transmit retry excessed limits\r\n");
                FXmacHandleTxErrors(instance_p);
            }
            if (error_word & FXMAC_TXSR_FRAMERX_MASK)
            {
                FXMAC_PORT_XMAC_PRINT_E("Transmit collision\r\n");
                FXmacProcessSentBds(instance_p, txring);
            }
            break;
        }
    }
}

void FXmacLinkChange(void *args)
{
    u32 ctrl;
    u32 link, link_status;
    FXmac *xmac_p;
    FXmacPort *instance_p;

    instance_p = (FXmacPort *)args;
    xmac_p = &instance_p->instance;

    if (xmac_p->config.interface == FXMAC_PHY_INTERFACE_MODE_SGMII)
    {
        FXMAC_PORT_XMAC_PRINT_I("xmac_p->config.base_address is %p", xmac_p->config.base_address);
        ctrl = FXMAC_READREG32(xmac_p->config.base_address, FXMAC_PCS_AN_LP_OFFSET);
        link = (ctrl & FXMAC_PCS_LINK_PARTNER_NEXT_PAGE_STATUS) >> FXMAC_PCS_LINK_PARTNER_NEXT_PAGE_OFFSET;

        switch (link)
        {
        case 0:
            FXMAC_PORT_XMAC_PRINT_I("link status is down");
            link_status = FXMAC_LINKDOWN;
            break;
        case 1:
            FXMAC_PORT_XMAC_PRINT_I("link status is up");
            link_status = FXMAC_LINKUP;
            break;
        default:
            FXMAC_PORT_XMAC_PRINT_E("link status is error 0x%x \r\n", link);
            return;
        }

        if (link_status == FXMAC_LINKUP)
        {
            if (link_status != xmac_p->link_status)
            {
                xmac_p->link_status = FXMAC_NEGOTIATING;
                FXMAC_PORT_XMAC_PRINT_I("need NEGOTIATING");
            }
        }
        else
        {
            xmac_p->link_status = FXMAC_LINKDOWN;
        }
    }
}

/* phy */

/**
 * @name: phy_link_detect
 * @msg:  获取当前link status
 * @note:
 * @param {FXmac} *fxmac_p
 * @param {u32} phy_addr
 * @return {*} 1 is link up , 0 is link down
 */
static u32 phy_link_detect(FXmac *xmac_p, u32 phy_addr)
{
    u16 status;

    /* Read Phy Status register twice to get the confirmation of the current
     * link status.
     */

    FXmacPhyRead(xmac_p, phy_addr, PHY_STATUS_REG_OFFSET, &status);

    if (status & PHY_STAT_LINK_STATUS)
        return 1;
    return 0;
}

static u32 phy_autoneg_status(FXmac *xmac_p, u32 phy_addr)
{
    u16 status;

    /* Read Phy Status register twice to get the confirmation of the current
     * link status.
     */
    FXmacPhyRead(xmac_p, phy_addr, PHY_STATUS_REG_OFFSET, &status);

    if (status & PHY_STATUS_AUTONEGOTIATE_COMPLETE)
        return 1;
    return 0;
}

enum port_link_status FXmacPortLinkDetect(FXmacPort *instance_p)
{
    u32 phy_link_status;
    FXmac *xmac_p = &instance_p->instance;

    if (xmac_p->is_ready != (u32)FT_COMPONENT_IS_READY)
    {
        return ETH_LINK_UNDEFINED;
    }

    phy_link_status = phy_link_detect(xmac_p, xmac_p->phy_address);

    if(xmac_p->config.auto_neg == 0)
    {
        return phy_link_status ? ETH_LINK_UP : ETH_LINK_DOWN;
    }

    if ((xmac_p->link_status == FXMAC_LINKUP) && (!phy_link_status))
        xmac_p->link_status = FXMAC_LINKDOWN;

    switch (xmac_p->link_status)
    {
    case FXMAC_LINKUP:
        return ETH_LINK_UP;
    case FXMAC_LINKDOWN:
        xmac_p->link_status = FXMAC_NEGOTIATING;
        FXMAC_PORT_XMAC_PRINT_D("Ethernet Link down");
        return ETH_LINK_DOWN;
    case FXMAC_NEGOTIATING:
        if ((phy_link_status == FXMAC_LINKUP) && phy_autoneg_status(xmac_p, xmac_p->phy_address))
        {
            err_t phy_ret;
            phy_ret = FXmacPhyInit(xmac_p, xmac_p->config.speed, xmac_p->config.duplex, xmac_p->config.auto_neg,XMAC_PHY_RESET_DISABLE);

            if (phy_ret != FT_SUCCESS)
            {
                FXMAC_PORT_XMAC_PRINT_E("FXmacPhyInit is error \r\n");
                return ETH_LINK_DOWN;
            }
            FXmacSelectClk(xmac_p);
            FXmacInitInterface(xmac_p);

            /* Initiate Phy setup to get link speed */
            xmac_p->link_status = FXMAC_LINKUP;
            FXMAC_PORT_XMAC_PRINT_D("Ethernet Link up");
            return ETH_LINK_UP;
        }
        return ETH_LINK_DOWN;
    default:
        return ETH_LINK_DOWN;
    }
}



enum port_link_status FXmacPhyReconnect(FXmacPort *instance_p)
{
    FXmac *xmac_p;

    FASSERT(instance_p != NULL);
    xmac_p = &(instance_p->instance);

    if (xmac_p->config.interface == FXMAC_PHY_INTERFACE_MODE_SGMII)
    {
        PRT_HwiDisable(xmac_p->config.queue_irq_num[0]);
        if (xmac_p->link_status == FXMAC_NEGOTIATING)
        {
            /* 重新自协商 */
            err_t phy_ret;
            phy_ret = FXmacPhyInit(xmac_p, xmac_p->config.speed, xmac_p->config.duplex, xmac_p->config.auto_neg,XMAC_PHY_RESET_DISABLE);
            if (phy_ret != FT_SUCCESS)
            {
                FXMAC_PORT_XMAC_PRINT_E("FXmacPhyInit is error \r\n");
                PRT_HwiEnable(xmac_p->config.queue_irq_num[0]);
                return ETH_LINK_DOWN;
            }
            FXmacSelectClk(xmac_p);
            FXmacInitInterface(xmac_p);
            xmac_p->link_status = FXMAC_LINKUP;
        }

        PRT_HwiEnable(xmac_p->config.queue_irq_num[0]);

        switch (xmac_p->link_status)
        {
        case FXMAC_LINKDOWN:
            return ETH_LINK_DOWN;
        case FXMAC_LINKUP:
            return ETH_LINK_UP;
        default:
            return ETH_LINK_DOWN;
        }
    }
    else if ((xmac_p->config.interface == FXMAC_PHY_INTERFACE_MODE_RMII) || (xmac_p->config.interface == FXMAC_PHY_INTERFACE_MODE_RGMII))
    {
        return FXmacPortLinkDetect(instance_p);
    }
    else if(xmac_p->config.interface == FXMAC_PHY_INTERFACE_MODE_USXGMII)
    {
        if(FXmacUsxLinkStatus(&instance_p->instance))
        {
            return ETH_LINK_UP;
        }
        else
        {
            return ETH_LINK_DOWN;
        }
    }
    else
    {
        switch (xmac_p->link_status)
        {
        case FXMAC_LINKDOWN:
            return ETH_LINK_DOWN;
        case FXMAC_LINKUP:
            return ETH_LINK_UP;
        default:
            return ETH_LINK_DOWN;
        }
    }
}

static void FXmacPortQueue0IntrHandler(void *args)
{
    FXmacIntrHandler(((FXmac *)args)->config.queue_irq_num[0], args);
}

static void FXmacSetupIsr(FXmacPort *instance_p)
{
    u32 ret;

    /* Setup callbacks */
    FXmacSetHandler(&instance_p->instance, FXMAC_HANDLER_DMASEND, FXmacSendHandler, instance_p);
    FXmacSetHandler(&instance_p->instance, FXMAC_HANDLER_DMARECV, FXmacRecvHandler, instance_p);
    FXmacSetHandler(&instance_p->instance, FXMAC_HANDLER_ERROR, FXmacErrorHandler, instance_p);
    FXmacSetHandler(&instance_p->instance, FXMAC_HANDLER_LINKCHANGE, FXmacLinkChange, instance_p);
    
    ret = PRT_HwiSetAttr(instance_p->instance.config.queue_irq_num[0], BSP_XMAC_INT_PRIO, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        FXMAC_PORT_XMAC_PRINT_E("FXmacSetupIsr: Error PRT_HwiSetAttr\r\n");
        return;
    }

    ret = PRT_HwiCreate(instance_p->instance.config.queue_irq_num[0], (HwiProcFunc)FXmacPortQueue0IntrHandler, (HwiArg)(&(instance_p->instance)));
    if (ret != OS_OK) {
        FXMAC_PORT_XMAC_PRINT_E("FXmacSetupIsr: Error PRT_HwiCreate\r\n");
        return;
    }

    ret = PRT_HwiEnable(instance_p->instance.config.queue_irq_num[0]);
    if (ret != OS_OK)
    {
        FXMAC_PORT_XMAC_PRINT_E("FXmacSetupIsr: Error PRT_HwiEnable\r\n");
        return;
    }

    PRT_HwiSetRouter(instance_p->instance.config.queue_irq_num[0]);
}

/*  init fxmac instance */

static void FXmacInitOnError(FXmacPort *instance_p)
{
    FXmac *xmac_p;
    u32 status = FT_SUCCESS;
    xmac_p = &instance_p->instance;

    /* set mac address */
    status = FXmacSetMacAddress(xmac_p, (void *)(instance_p->hwaddr), instance_p->xmac_port_config.instance_id);
    if (status != FT_SUCCESS)
    {
        FXMAC_PORT_XMAC_PRINT_E("In %s:Emac Mac Address set failed...\r\n", __func__);
    }
}
static FError FXmacPortConfigConvert(FXmacPort *instance_p,FXmacConfig* xmac_config_p)
{
    
    const FXmacConfig *mac_config_p;
    FXmacPhyInterface interface = FXMAC_PHY_INTERFACE_MODE_SGMII;

    mac_config_p = FXmacLookupConfig(instance_p->xmac_port_config.instance_id);
    if (mac_config_p == NULL)
    {
        FXMAC_PORT_XMAC_PRINT_E("FXmacLookupConfig is error , instance_id is %d", instance_p->xmac_port_config.instance_id);
        return FREERTOS_XMAC_INIT_ERROR;
    }
    
    memcpy(xmac_config_p,mac_config_p,sizeof (FXmacConfig));

    switch (instance_p->xmac_port_config.interface)
    {
    case FXMAC_PORT_INTERFACE_SGMII:
        interface = FXMAC_PHY_INTERFACE_MODE_SGMII;
        FXMAC_PORT_XMAC_PRINT_I("SGMII select");
        break;
    case FXMAC_PORT_INTERFACE_RMII:
        interface = FXMAC_PHY_INTERFACE_MODE_RMII;
        FXMAC_PORT_XMAC_PRINT_I("RMII select");
        break;
    case FXMAC_PORT_INTERFACE_RGMII:
        FXMAC_PORT_XMAC_PRINT_I("RGMII select");
        interface = FXMAC_PHY_INTERFACE_MODE_RGMII;
        break;
    case FXMAC_PORT_INTERFACE_USXGMII:
        FXMAC_PORT_XMAC_PRINT_I("USXGMII select");
        instance_p->xmac_port_config.phy_speed = FXMAC_PHY_SPEED_10G ;
        interface = FXMAC_PHY_INTERFACE_MODE_USXGMII;
        break;
    default:
        FXMAC_PORT_XMAC_PRINT_E("update interface is error , interface is %d", instance_p->xmac_port_config.instance_id);
        return FREERTOS_XMAC_INIT_ERROR;
    }
    xmac_config_p->interface = interface;

    if (instance_p->xmac_port_config.autonegotiation)
    {
        xmac_config_p->auto_neg = 1;
    }
    else
    {
        xmac_config_p->auto_neg = 0;
    }

    switch (instance_p->xmac_port_config.phy_speed)
    {
    case FXMAC_PHY_SPEED_10M:
        xmac_config_p->speed = FXMAC_SPEED_10;
        break;
    case FXMAC_PHY_SPEED_100M:
        xmac_config_p->speed = FXMAC_SPEED_100;
        break;
    case FXMAC_PHY_SPEED_1000M:
        xmac_config_p->speed = FXMAC_SPEED_1000;
        break;
    case FXMAC_PHY_SPEED_10G:
        FXMAC_PORT_XMAC_PRINT_I("select FXMAC_PHY_SPEED_10G");
        xmac_config_p->speed = FXMAC_SPEED_10000;
        break;
    default:
        FXMAC_PORT_XMAC_PRINT_E("setting speed is not valid , speed is %d", instance_p->xmac_port_config.phy_speed);
        return FREERTOS_XMAC_INIT_ERROR;
    }

    switch (instance_p->xmac_port_config.phy_duplex)
    {
    case FXMAC_PHY_HALF_DUPLEX:
        xmac_config_p->duplex = 0;
        break;
    case FXMAC_PHY_FULL_DUPLEX:
        xmac_config_p->duplex = 1;
        break;
    }

    return FT_SUCCESS;
}


void FxmacFeatureSetOptions(u32 feature,FXmac* xmac_p)
{
    u32 options=0;
    if (feature & FXMAC_PORT_CONFIG_JUMBO)
    {
        FXMAC_PORT_XMAC_PRINT_I("FXMAC_JUMBO_ENABLE_OPTION is ok");
        options |= FXMAC_JUMBO_ENABLE_OPTION;
    }
    
    if (feature & FXMAC_PORT_CONFIG_UNICAST_ADDRESS_FILITER)
    {
        FXMAC_PORT_XMAC_PRINT_I("FXMAC_UNICAST_OPTION is ok");
        options |= FXMAC_UNICAST_OPTION;
    }

    if (feature & FXMAC_PORT_CONFIG_MULTICAST_ADDRESS_FILITER)
    {
        FXMAC_PORT_XMAC_PRINT_I("FXMAC_MULTICAST_OPTION is ok");
        options |= FXMAC_MULTICAST_OPTION;
    }
    /* enable copy all frames */
    if (feature & FXMAC_PORT_CONFIG_COPY_ALL_FRAMES)
    {
        FXMAC_PORT_XMAC_PRINT_I("FXMAC_PROMISC_OPTION is ok");
        options |= FXMAC_PROMISC_OPTION;
    }
      /* close fcs check */
    if (feature & FXMAC_PORT_CONFIG_CLOSE_FCS_CHECK)
    {
        FXMAC_PORT_XMAC_PRINT_I("FXMAC_FCS_STRIP_OPTION is ok");
        options |= FXMAC_FCS_STRIP_OPTION;
    }
    FXmacSetOptions(xmac_p, options, 0);
}
/* step 1: initialize instance */
/* step 2: depend on config set some options : JUMBO / IGMP */
/* step 3: FXmacSelectClk */
/* step 4: FXmacInitInterface */
/* step 5: initialize phy */
/* step 6: initialize dma */
/* step 7: initialize interrupt */
/* step 8: start mac */

FError FXmacPortInit(FXmacPort *instance_p)
{
    FXmacConfig mac_config;
    FXmac *xmac_p;
    u32 dmacrreg;
    FError status;
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->xmac_port_config.instance_id < FXMAC_NUM);

    xmac_p = &instance_p->instance; 
    FXMAC_PORT_XMAC_PRINT_I("instance_id IS %d \r\n", instance_p->xmac_port_config.instance_id);

    status = FXmacPortConfigConvert(instance_p,&mac_config);
    if (status != FT_SUCCESS)
    {
        FXMAC_PORT_XMAC_PRINT_E("In %s: FXmacPortControl Convert to FXmacConfig Failed....\r\n", __func__);
    }

    status = FXmacCfgInitialize(xmac_p, &mac_config);
    if (status != FT_SUCCESS)
    {
        FXMAC_PORT_XMAC_PRINT_E("In %s:EmacPs Configuration Failed....\r\n", __func__);
    }

    if ((instance_p->instance).config.caps & FXMAC_CAPS_TAILPTR)
    {   
        FXmacSetOptions(xmac_p, FXMAC_TAIL_PTR_OPTION, 0);
        xmac_p->mask &= (~FXMAC_IXR_TXUSED_MASK);
    }

    FxmacFeatureSetOptions(instance_p->feature,xmac_p);

    status = FXmacSetMacAddress(xmac_p, (void *)(instance_p->hwaddr), 0);
    if (status != FT_SUCCESS)
    {
        FXMAC_PORT_XMAC_PRINT_E("In %s:Emac Mac Address set failed...\r\n", __func__);
    }

    if(mac_config.interface != FXMAC_PHY_INTERFACE_MODE_USXGMII)
    {
        /* initialize phy */
        status = FXmacPhyInit(xmac_p, xmac_p->config.speed, xmac_p->config.duplex, xmac_p->config.auto_neg,XMAC_PHY_RESET_ENABLE);
        if (status != FT_SUCCESS)
        {
            FXMAC_PORT_XMAC_PRINT_W("FXmacPhyInit is error \r\n");
        }
    }
    else
    {
        
    }

    FXmacSelectClk(xmac_p);
    FXmacInitInterface(xmac_p);

    /* initialize dma */
    dmacrreg = FXMAC_READREG32(xmac_p->config.base_address, FXMAC_DMACR_OFFSET);
    dmacrreg &= ~(FXMAC_DMACR_BLENGTH_MASK);
    dmacrreg = dmacrreg | FXMAC_DMACR_INCR16_AHB_AXI_BURST; /* Attempt to use bursts of up to 16. */
    FXMAC_WRITEREG32(xmac_p->config.base_address, FXMAC_DMACR_OFFSET, dmacrreg);
    FXmacInitDma(instance_p);

    /* initialize interrupt */
    FXmacSetupIsr(instance_p);

    return FT_SUCCESS;
}


/**
 * @name: FXmacPortRx
 * @msg:  void *FXmacPortRx(FXmacPort *instance_p)
 * @note:
 * @param {FXmacPort} *instance_p
 * @return {*}
 */
void *FXmacPortRx(FXmacPort *instance_p)
{
    FASSERT(instance_p != NULL);
    struct fx_pbuf *p;

    /* see if there is data to process */
    if (FXmacPqQlength(&instance_p->recv_q) == 0)
        return NULL;
    /* return one packet from receive q */
    p = (struct fx_pbuf *)FXmacPqDequeue(&instance_p->recv_q);

    return p;
}

static FError FXmacPortOutput(FXmacPort *instance_p, struct fx_pbuf *p)
{
    FError status = 0;
    status = FXmacSgsend(instance_p, p);
    if (status != FT_SUCCESS)
    {
#if LINK_STATS
        lwip_stats.link.drop++;
#endif
    }

#if LINK_STATS
    lwip_stats.link.xmit++;
#endif /* LINK_STATS */

    return status;
}

FError FXmacPortTx(FXmacPort *instance_p, void *tx_buf)
{
    u32 freecnt;
    FXmacBdRing *txring;
    FError ret = FT_SUCCESS;
    struct fx_pbuf *p;
    FASSERT(instance_p != NULL);
    if (tx_buf == NULL)
    {
        FXMAC_PORT_XMAC_PRINT_E("tx_buf is null \r\n");
        return FREERTOS_XMAC_PARAM_ERROR;
    }

    p = tx_buf;
    PRT_TaskLock();
    /* check if space is available to send */
    freecnt = IsTxSpaceAvailable(instance_p);

    if (freecnt <= 5)
    {
        txring = &(FXMAC_GET_TXRING(instance_p->instance));
        FXmacProcessSentBds(instance_p, txring);
    }

    if (IsTxSpaceAvailable(instance_p))
    {
        ret = FXmacPortOutput(instance_p, p);
    }
    else
    {
#if LINK_STATS
        lwip_stats.link.drop++;
#endif
        FXMAC_PORT_XMAC_PRINT_E("pack dropped, no space\r\n");
        ret = FREERTOS_XMAC_NO_VALID_SPACE;
    }

    PRT_TaskUnlock();
    return ret;
}

FXmacPort * FXmacPortGetInstancePointer(u32 FXmacPortInstanceID)
{
    FASSERT(FXmacPortInstanceID < FXMAC_NUM);
    
    FXmacPort *instance_p;
    instance_p = &fxmac_port_instance[FXmacPortInstanceID];
    return instance_p;
}


void FXmacPortStop(FXmacPort *instance_p)
{
    FASSERT(instance_p != NULL);

//need to add deinit interupt
    /* step 1 close mac controler  */
    FXmacStop(&instance_p->instance);
    /* step 2 free all fx_pbuf */
    FreeTxRxPbufs(instance_p);
}

void FXmacPortStart(FXmacPort *instance_p)
{
    FASSERT(instance_p != NULL);
    
    /* start mac */
    FXmacStart(&instance_p->instance);
}

