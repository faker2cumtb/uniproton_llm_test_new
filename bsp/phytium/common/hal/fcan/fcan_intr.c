/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: fcan_intr.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2024-06-24 08:29:10
 * Description:  This files is for the can interrupt functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/5/26  first release
 * 1.1   zhangyan      2022/12/7  improve functions
 * 1.2   huangjin      2024/06/24 modify the FCanIntrHandler function
 */

#include "fcan.h"
#include "fcan_hw.h"
#include "fassert.h"
#include "ftypes.h"
#include "fdrivers_port.h"

#define FT_CAN_DEBUG_TAG "FT_CAN_INTR"
#define FCAN_DEBUG(format, ...) FT_DEBUG_PRINT_D(FT_CAN_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_INFO(format, ...) FT_DEBUG_PRINT_I(FT_CAN_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_WARN(format, ...) FT_DEBUG_PRINT_W(FT_CAN_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_ERROR(format, ...) FT_DEBUG_PRINT_E(FT_CAN_DEBUG_TAG, format, ##__VA_ARGS__)

#define FCAN_CALL_INTR_EVENT_HANDLDER(instance_p, event) \
    if (instance_p->intr_event[event].handler)                 \
        instance_p->intr_event[event].handler(instance_p->intr_event[event].param)

/**
 * @name: FCanRegisterInterruptHandler
 * @msg: register FCanCtrl interrupt handler function
 * @param {FCanCtrl} *instance_p, pointer to the Can instance
 * @param {FCanIntrEventConfig} *intr_event_p, interrupt event type, handler and parameters
 * @return {*}
 */
void FCanRegisterInterruptHandler(FCanCtrl *instance_p, FCanIntrEventConfig *intr_event_p)
{
    FASSERT(instance_p);
    FASSERT(intr_event_p);
    FCanIntrEventType type = intr_event_p->type;
    FASSERT(intr_event_p->type < FCAN_INTR_EVENT_NUM);
    instance_p->intr_event[type].type = type;
    instance_p->intr_event[type].handler = intr_event_p->handler;
    instance_p->intr_event[type].param = intr_event_p->param;
}

/**
 * @name: FCanIntrHandler
 * @msg:  This function is the interrupt handler for the driver.
 *          It must be connected to an interrupt system by the application such that it
 *          can be called when an interrupt occurs.
 * @param vector Irq num, Don't need attention.
 * @param args  contains a pointer to the driver instance
 * @return {*}
 */
void FCanIntrHandler(s32 vector, void *args)
{
    u32 irq_status;
    FCanCtrl *instance_p = (FCanCtrl *)args;
    FCanConfig *config_p;
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    config_p = &instance_p->config;
    /* 获取中断状态 */
    irq_status = FCAN_READ_REG32(config_p->base_address, FCAN_INTR_OFFSET);

    if (0 == irq_status)
    {
        return;
    }

    /* 检查接收FIFO满中断标志 */
    if (irq_status & FCAN_INTR_RFIS_MASK)
    {
        FCAN_DEBUG("rx_fifo is full!!!");
        FCAN_CLEARBIT(config_p->base_address, FCAN_INTR_OFFSET, FCAN_INTR_RFIE_MASK);
        FCAN_SETBIT(config_p->base_address, FCAN_INTR_OFFSET, FCAN_INTR_RFIC_MASK);                        
        FCAN_CALL_INTR_EVENT_HANDLDER(instance_p, FCAN_INTR_EVENT_FIFOFULL);
    }
    /* 检查发送FIFO空中断标志 */
    if (irq_status & FCAN_INTR_TFIS_MASK)
    {
        FCAN_DEBUG("tx_fifo is empty!!!");
        FCAN_CLEARBIT(config_p->base_address, FCAN_INTR_OFFSET, FCAN_INTR_TFIE_MASK);   
        FCAN_SETBIT(config_p->base_address, FCAN_INTR_OFFSET, FCAN_INTR_TFIC_MASK);                     
        FCAN_CALL_INTR_EVENT_HANDLDER(instance_p, FCAN_INTR_EVENT_FIFOEMPTY);  
    }

    /* 检查错误中断的类型并处理 */
    if ( irq_status & (FCAN_INTR_BOIS_MASK | FCAN_INTR_EIS_MASK | FCAN_INTR_PEIS_MASK | FCAN_INTR_PWIS_MASK) )
    {
        FCAN_CLEARBIT(config_p->base_address, FCAN_INTR_OFFSET, (FCAN_INTR_BOIE_MASK | FCAN_INTR_EIE_MASK | FCAN_INTR_PEIE_MASK | FCAN_INTR_PWIE_MASK));  
        if (irq_status & FCAN_INTR_BOIS_MASK)
        {
            FCAN_ERROR("Bus off!!!");            
            FCAN_CALL_INTR_EVENT_HANDLDER(instance_p, FCAN_INTR_EVENT_BUSOFF);    
        }
        if (irq_status & FCAN_INTR_EIS_MASK)
        {
            FCAN_ERROR("Error occurred!!!");                      
            FCAN_CALL_INTR_EVENT_HANDLDER(instance_p, FCAN_INTR_EVENT_ERROR);
        }
        if (irq_status & FCAN_INTR_PEIS_MASK)
        {
            FCAN_ERROR("Passive error occurred!!!");                    
            FCAN_CALL_INTR_EVENT_HANDLDER(instance_p, FCAN_INTR_EVENT_PERROE);   
        }
        if (irq_status & FCAN_INTR_PWIS_MASK)
        {
            FCAN_ERROR("Passive warning!!!");                            
            FCAN_CALL_INTR_EVENT_HANDLDER(instance_p, FCAN_INTR_EVENT_PWARN);     
        }
        FCAN_SETBIT(config_p->base_address, FCAN_INTR_OFFSET, (FCAN_INTR_BOIC_MASK | FCAN_INTR_EIC_MASK | FCAN_INTR_PEIC_MASK | FCAN_INTR_PWIC_MASK));
    }
    
    /* 检查发送完成中断标志 */
    if (irq_status & FCAN_INTR_TEIS_MASK)
    {
        irq_status &= ~FCAN_INTR_REIS_MASK;
        FCAN_SETBIT(config_p->base_address, FCAN_INTR_OFFSET, FCAN_INTR_TEIC_MASK | FCAN_INTR_REIC_MASK);
        FCAN_CALL_INTR_EVENT_HANDLDER(instance_p, FCAN_INTR_EVENT_SEND);     
    }
    /* 检查接收完成中断标志 */
    if (irq_status & FCAN_INTR_REIS_MASK)
    {
        FCAN_SETBIT(config_p->base_address, FCAN_INTR_OFFSET, FCAN_INTR_REIE_MASK);
        FCAN_SETBIT(config_p->base_address, FCAN_INTR_OFFSET, FCAN_INTR_REIC_MASK);
        FCAN_CALL_INTR_EVENT_HANDLDER(instance_p, FCAN_INTR_EVENT_RECV);   
    }
}
