/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-06-01     KyleChan     first version
 * 2024-07-19     LuoYuncong   Port to UniProton
 */

#include <rtapi.h>
#include <rtdevice.h>

static rt_size_t rt_serial_get_linear_buffer(struct rt_ringbuffer       *rb,
                                                    rt_uint8_t         **ptr)
{
    rt_size_t size;

    RT_ASSERT(rb != RT_NULL);

    *ptr = RT_NULL;

    /* whether has enough data  */
    size = rt_ringbuffer_data_len(rb);

    /* no data */
    if (size == 0)
        return 0;

    *ptr = &rb->buffer_ptr[rb->read_index];

    if(rb->buffer_size - rb->read_index > size)
    {
        return size;
    }

    return rb->buffer_size - rb->read_index;
}

static rt_size_t rt_serial_update_read_index(struct rt_ringbuffer    *rb,
                                                    rt_uint16_t       read_index)
{
    rt_size_t size;

    RT_ASSERT(rb != RT_NULL);

    /* whether has enough data  */
    size = rt_ringbuffer_data_len(rb);

    /* no data */
    if (size == 0)
        return 0;

    /* less data */
    if(size < read_index)
        read_index = size;

    if(rb->buffer_size - rb->read_index > read_index)
    {
        rb->read_index += read_index;
        return read_index;
    }

    read_index = rb->buffer_size - rb->read_index;

    /* we are going into the other side of the mirror */
    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = 0;

    return read_index;
}

static rt_size_t rt_serial_update_write_index(struct rt_ringbuffer  *rb,
                                                     rt_uint16_t     write_index)
{
    rt_uint16_t size;
    RT_ASSERT(rb != RT_NULL);

    /* whether has enough space */
    size = rt_ringbuffer_space_len(rb);

    /* no space */
    if (size == 0)
        return 0;

    /* drop some data */
    if (size < write_index)
        write_index = size;

    if (rb->buffer_size - rb->write_index > write_index)
    {
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += write_index;
        return write_index;
    }

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = write_index - (rb->buffer_size - rb->write_index);

    return write_index;
}


/**
  * @brief Serial polling receive data routine, This function will receive data
  *        in a continuous loop by one by one byte.
  * @param dev The pointer of device driver structure
  * @param pos Empty parameter.
  * @param buffer Receive data buffer.
  * @param size Receive data buffer length.
  * @return Return the final length of data received.
  */
rt_size_t _serial_poll_rx(struct rt_device          *dev,
                                 rt_off_t            pos,
                                 void               *buffer,
                                 rt_size_t           size)
{
    struct rt_serial_device *serial;
    rt_size_t getc_size;
    int getc_element;      /* Gets one byte of data received */
    rt_uint8_t *getc_buffer;     /* Pointer to the receive data buffer */

    RT_ASSERT(dev != RT_NULL);

    serial = (struct rt_serial_device *)dev;
    RT_ASSERT(serial != RT_NULL);
    getc_buffer = (rt_uint8_t *)buffer;
    getc_size = size;

    while(size)
    {
        getc_element = serial->ops->getc(serial);
        if (getc_element == -1) break;

        *getc_buffer = getc_element;

        ++ getc_buffer;
        -- size;

        if (serial->parent.open_flag & RT_DEVICE_FLAG_STREAM)
        {
            /* If open_flag satisfies RT_DEVICE_FLAG_STREAM
             * and the received character is '\n', exit the loop directly */
            if (getc_element == '\n') break;
        }
    }

   return getc_size - size;
}

/**
  * @brief Serial polling transmit data routines, This function will transmit
  *        data in a continuous loop by one by one byte.
  * @param dev The pointer of device driver structure
  * @param pos Empty parameter.
  * @param buffer Transmit data buffer.
  * @param size Transmit data buffer length.
  * @return Return the final length of data received.
  */
rt_size_t _serial_poll_tx(struct rt_device           *dev,
                                 rt_off_t             pos,
                                 const void          *buffer,
                                 rt_size_t            size)
{
    struct rt_serial_device *serial;
    rt_size_t putc_size;
    rt_uint8_t *putc_buffer;    /* Pointer to the transmit data buffer */
    RT_ASSERT(dev != RT_NULL);

    serial = (struct rt_serial_device *)dev;
    RT_ASSERT(serial != RT_NULL);

    putc_buffer = (rt_uint8_t *)buffer;
    putc_size = size;

    while (size)
    {
        if (serial->parent.open_flag & RT_DEVICE_FLAG_STREAM)
        {
            /* If open_flag satisfies RT_DEVICE_FLAG_STREAM and the received character is '\n',
             * inserts '\r' character before '\n' character for the effect of carriage return newline */
            if (*putc_buffer == '\n')
                serial->ops->putc(serial, '\r');
        }
        serial->ops->putc(serial, *putc_buffer);

        ++ putc_buffer;
        -- size;
    }

     return putc_size - size;
}

/**
  * @brief Serial receive data routines, This function will receive
  *        data by using fifo
  * @param dev The pointer of device driver structure
  * @param pos Empty parameter.
  * @param buffer Receive data buffer.
  * @param size Receive data buffer length.
  * @return Return the final length of data received.
  */
static rt_size_t _serial_fifo_rx(struct rt_device        *dev,
                                        rt_off_t          pos,
                                        void             *buffer,
                                        rt_size_t         size)
{
    struct rt_serial_device *serial;
    struct rt_serial_rx_fifo *rx_fifo;
    rt_size_t recv_len;  /* The length of data from the ringbuffer */

    RT_ASSERT(dev != RT_NULL);
    if (size == 0) return 0;

    serial = (struct rt_serial_device *)dev;

    RT_ASSERT((serial != RT_NULL) && (buffer != RT_NULL));

    rx_fifo = (struct rt_serial_rx_fifo *) serial->serial_rx;

    if (dev->open_flag & RT_SERIAL_RX_BLOCKING)
    {
        if (size > serial->config.rx_bufsz)
        {
            return 0;
        }
        /* Get the length of the data from the ringbuffer */
        recv_len = rt_ringbuffer_data_len(&(rx_fifo->rb));

        if (recv_len < size)
        {
            /* When recv_len is less than size, rx_cpt_index is updated to the size
            * and rt_current_thread is suspend until rx_cpt_index is equal to 0 */
            rx_fifo->rx_cpt_index = size;
            rt_completion_wait(&(rx_fifo->rx_cpt), RT_WAITING_FOREVER);
        }
    }

    /* This part of the code is open_flag as RT_SERIAL_RX_NON_BLOCKING */

    /* When open_flag is RT_SERIAL_RX_NON_BLOCKING,
     * the data is retrieved directly from the ringbuffer and returned */
    recv_len = rt_ringbuffer_get(&(rx_fifo->rb), buffer, size);

    return recv_len;
}

/**
  * @brief Serial transmit data routines, This function will transmit
  *        data by using blocking_nbuf.
  * @param dev The pointer of device driver structure
  * @param pos Empty parameter.
  * @param buffer Transmit data buffer.
  * @param size Transmit data buffer length.
  * @return Return the final length of data transmit.
  */
static rt_size_t _serial_fifo_tx_blocking_nbuf(struct rt_device        *dev,
                                                      rt_off_t          pos,
                                                const void             *buffer,
                                                      rt_size_t         size)
{
    struct rt_serial_device *serial;
    struct rt_serial_tx_fifo *tx_fifo = RT_NULL;

    RT_ASSERT(dev != RT_NULL);
    if (size == 0) return 0;

    serial = (struct rt_serial_device *)dev;
    RT_ASSERT((serial != RT_NULL) && (buffer != RT_NULL));
    tx_fifo = (struct rt_serial_tx_fifo *) serial->serial_tx;
    RT_ASSERT(tx_fifo != RT_NULL);

    /* When serial transmit in tx_blocking mode,
     * if the activated mode is RT_TRUE, it will return directly */
    if (tx_fifo->activated == RT_TRUE)  return 0;

    tx_fifo->activated = RT_TRUE;
    /* Call the transmit interface for transmission */
    serial->ops->transmit(serial,
                          (rt_uint8_t *)buffer,
                          size,
                          RT_SERIAL_TX_BLOCKING);
    /* Waiting for the transmission to complete */
    rt_completion_wait(&(tx_fifo->tx_cpt), RT_WAITING_FOREVER);

    return size;
}

/**
  * @brief Serial transmit data routines, This function will transmit
  *        data by using blocking_buf.
  * @param dev The pointer of device driver structure
  * @param pos Empty parameter.
  * @param buffer Transmit data buffer.
  * @param size Transmit data buffer length.
  * @return Return the final length of data transmit.
  */
static rt_size_t _serial_fifo_tx_blocking_buf(struct rt_device        *dev,
                                                     rt_off_t          pos,
                                               const void             *buffer,
                                                    rt_size_t          size)
{
    struct rt_serial_device *serial;
    struct rt_serial_tx_fifo *tx_fifo = RT_NULL;
    rt_size_t length = size;
    rt_size_t offset = 0;

    if (size == 0) return 0;

    RT_ASSERT(dev != RT_NULL);
    serial = (struct rt_serial_device *)dev;
    RT_ASSERT((serial != RT_NULL) && (buffer != RT_NULL));

    tx_fifo = (struct rt_serial_tx_fifo *) serial->serial_tx;
    RT_ASSERT(tx_fifo != RT_NULL);

    if (serial->parent.open_flag & RT_DEVICE_FLAG_STREAM)
    {
        /* using poll tx when the scheduler not startup or in stream mode */
        return _serial_poll_tx(dev, pos, buffer, size);
    }
    /* When serial transmit in tx_blocking mode,
     * if the activated mode is RT_TRUE, it will return directly */
    if (tx_fifo->activated == RT_TRUE)  return 0;
    tx_fifo->activated = RT_TRUE;

    while (size)
    {
        /* Copy one piece of data into the ringbuffer at a time
         * until the length of the data is equal to size */
        tx_fifo->put_size = rt_ringbuffer_put(&(tx_fifo->rb),
                                               (rt_uint8_t *)buffer + offset,
                                               size);

        offset += tx_fifo->put_size;
        size -= tx_fifo->put_size;
        /* Call the transmit interface for transmission */
        serial->ops->transmit(serial,
                             (rt_uint8_t *)buffer + offset,
                             tx_fifo->put_size,
                             RT_SERIAL_TX_BLOCKING);
        /* Waiting for the transmission to complete */
        rt_completion_wait(&(tx_fifo->tx_cpt), RT_WAITING_FOREVER);
    }

    return length;
}

/**
  * @brief Serial transmit data routines, This function will transmit
  *        data by using nonblocking.
  * @param dev The pointer of device driver structure
  * @param pos Empty parameter.
  * @param buffer Transmit data buffer.
  * @param size Transmit data buffer length.
  * @return Return the final length of data transmit.
  */
static rt_size_t _serial_fifo_tx_nonblocking(struct rt_device        *dev,
                                                    rt_off_t          pos,
                                              const void             *buffer,
                                                    rt_size_t         size)
{
    struct rt_serial_device *serial;
    struct rt_serial_tx_fifo *tx_fifo;
    rt_size_t length;

    RT_ASSERT(dev != RT_NULL);
    if (size == 0) return 0;

    serial = (struct rt_serial_device *)dev;
    RT_ASSERT((serial != RT_NULL) && (buffer != RT_NULL));
    tx_fifo = (struct rt_serial_tx_fifo *) serial->serial_tx;

    if (tx_fifo->activated == RT_FALSE)
    {
        /* When serial transmit in tx_non_blocking mode, if the activated mode is RT_FALSE,
         * start copying data into the ringbuffer */
        tx_fifo->activated = RT_TRUE;
        /* Copying data into the ringbuffer */
        length = rt_ringbuffer_put(&(tx_fifo->rb), buffer, size);

        rt_uint8_t *put_ptr = RT_NULL;
        /* Get the linear length buffer from rinbuffer */
        tx_fifo->put_size = rt_serial_get_linear_buffer(&(tx_fifo->rb), &put_ptr);
        /* Call the transmit interface for transmission */
        serial->ops->transmit(serial,
                              put_ptr,
                              tx_fifo->put_size,
                              RT_SERIAL_TX_NON_BLOCKING);
        /* In tx_nonblocking mode, there is no need to call rt_completion_wait() APIs to wait
         * for the rt_current_thread to resume */
        return length;
    }

    /* If the activated mode is RT_FALSE, it means that serial device is transmitting,
     * where only the data in the ringbuffer and there is no need to call the transmit() API.
     * Note that this part of the code requires disable interrupts
     * to prevent multi thread reentrant */

    /* Copying data into the ringbuffer */
    length = rt_ringbuffer_put(&(tx_fifo->rb), buffer, size);

    return length;
}


/**
  * @brief Enable serial transmit mode.
  * @param dev The pointer of device driver structure
  * @param rx_oflag The flag of that the serial port opens.
  * @return Return the status of the operation.
  */
static rt_err_t rt_serial_tx_enable(struct rt_device        *dev,
                                           rt_uint16_t       tx_oflag)
{
    struct rt_serial_device *serial;
    struct rt_serial_tx_fifo *tx_fifo = RT_NULL;

    RT_ASSERT(dev != RT_NULL);
    serial = (struct rt_serial_device *)dev;

    if (serial->config.tx_bufsz == 0)
    {
        /* Cannot use RT_SERIAL_TX_NON_BLOCKING when tx_bufsz is 0 */
        if (tx_oflag == RT_SERIAL_TX_NON_BLOCKING)
        {
            return -RT_EINVAL;
        }

        dev->write = _serial_poll_tx;
        dev->open_flag |= RT_SERIAL_TX_BLOCKING;
        return RT_EOK;
    }
    /* Limits the minimum value of tx_bufsz */
    if (serial->config.tx_bufsz < RT_SERIAL_TX_MINBUFSZ)
        serial->config.tx_bufsz = RT_SERIAL_TX_MINBUFSZ;

    if (tx_oflag == RT_SERIAL_TX_BLOCKING)
    {
        /* When using RT_SERIAL_TX_BLOCKING, it is necessary to determine
         * whether serial device needs to use buffer */
        rt_err_t optmode;  /* The operating mode used by serial device */
        /* Call the Control() API to get the operating mode */
        optmode = serial->ops->control(serial,
                                       RT_DEVICE_CHECK_OPTMODE,
                                       (void *)RT_DEVICE_FLAG_TX_BLOCKING);
        if (optmode == RT_SERIAL_TX_BLOCKING_BUFFER)
        {
            /* If use RT_SERIAL_TX_BLOCKING_BUFFER, the ringbuffer is initialized */
            tx_fifo = (struct rt_serial_tx_fifo *) rt_malloc
                    (sizeof(struct rt_serial_tx_fifo) + serial->config.tx_bufsz);
            RT_ASSERT(tx_fifo != RT_NULL);

            rt_ringbuffer_init(&(tx_fifo->rb),
                                tx_fifo->buffer,
                                serial->config.tx_bufsz);
            serial->serial_tx = tx_fifo;
            dev->write = _serial_fifo_tx_blocking_buf;
        }
        else
        {
            /* If not use RT_SERIAL_TX_BLOCKING_BUFFER,
             * the control() API is called to configure the serial device */
            tx_fifo = (struct rt_serial_tx_fifo*) rt_malloc
                    (sizeof(struct rt_serial_tx_fifo));
            RT_ASSERT(tx_fifo != RT_NULL);

            serial->serial_tx = tx_fifo;

            dev->write = _serial_fifo_tx_blocking_nbuf;

            /* Call the control() API to configure the serial device by RT_SERIAL_TX_BLOCKING*/
            serial->ops->control(serial,
                                RT_DEVICE_CTRL_CONFIG,
                                (void *)RT_SERIAL_TX_BLOCKING);
        }

        tx_fifo->activated = RT_FALSE;
        tx_fifo->put_size = 0;
        rt_completion_init(&(tx_fifo->tx_cpt));
        dev->open_flag |= RT_SERIAL_TX_BLOCKING;

        return RT_EOK;
    }
    /* When using RT_SERIAL_TX_NON_BLOCKING, ringbuffer needs to be initialized,
     * and initialize the tx_fifo->activated value is RT_FALSE.
     */
    tx_fifo = (struct rt_serial_tx_fifo *) rt_malloc
            (sizeof(struct rt_serial_tx_fifo) + serial->config.tx_bufsz);
    RT_ASSERT(tx_fifo != RT_NULL);

    tx_fifo->activated = RT_FALSE;
    tx_fifo->put_size = 0;
    rt_ringbuffer_init(&(tx_fifo->rb),
                        tx_fifo->buffer,
                        serial->config.tx_bufsz);
    serial->serial_tx = tx_fifo;

    dev->write = _serial_fifo_tx_nonblocking;
    dev->open_flag |= RT_SERIAL_TX_NON_BLOCKING;
    /* Call the control() API to configure the serial device by RT_SERIAL_TX_NON_BLOCKING*/
    serial->ops->control(serial,
                        RT_DEVICE_CTRL_CONFIG,
                        (void *)RT_SERIAL_TX_NON_BLOCKING);

    return RT_EOK;
}


/**
  * @brief Enable serial receive mode.
  * @param dev The pointer of device driver structure
  * @param rx_oflag The flag of that the serial port opens.
  * @return Return the status of the operation.
  */
static rt_err_t rt_serial_rx_enable(struct rt_device        *dev,
                                           rt_uint16_t       rx_oflag)
{
    struct rt_serial_device *serial;
    struct rt_serial_rx_fifo *rx_fifo = RT_NULL;

    RT_ASSERT(dev != RT_NULL);
    serial = (struct rt_serial_device *)dev;

    if (serial->config.rx_bufsz == 0)
    {
        /* Cannot use RT_SERIAL_RX_NON_BLOCKING when rx_bufsz is 0 */
        if (rx_oflag == RT_SERIAL_RX_NON_BLOCKING)
        {
            return -RT_EINVAL;
        }

        dev->read = _serial_poll_rx;
        dev->open_flag |= RT_SERIAL_RX_BLOCKING;
        return RT_EOK;
    }
    /* Limits the minimum value of rx_bufsz */
    if (serial->config.rx_bufsz < RT_SERIAL_RX_MINBUFSZ)
        serial->config.rx_bufsz = RT_SERIAL_RX_MINBUFSZ;

    rx_fifo = (struct rt_serial_rx_fifo *) rt_malloc
            (sizeof(struct rt_serial_rx_fifo) + serial->config.rx_bufsz);

    RT_ASSERT(rx_fifo != RT_NULL);
    rt_ringbuffer_init(&(rx_fifo->rb), rx_fifo->buffer, serial->config.rx_bufsz);

    serial->serial_rx = rx_fifo;
    dev->read = _serial_fifo_rx;

    if (rx_oflag == RT_SERIAL_RX_NON_BLOCKING)
    {
        dev->open_flag |= RT_SERIAL_RX_NON_BLOCKING;
        /* Call the control() API to configure the serial device by RT_SERIAL_RX_NON_BLOCKING*/
        serial->ops->control(serial,
                            RT_DEVICE_CTRL_CONFIG,
                            (void *) RT_SERIAL_RX_NON_BLOCKING);

        return RT_EOK;
    }
    /* When using RT_SERIAL_RX_BLOCKING, rt_completion_init() and rx_cpt_index are initialized */
    rx_fifo->rx_cpt_index = 0;
    rt_completion_init(&(rx_fifo->rx_cpt));
    dev->open_flag |= RT_SERIAL_RX_BLOCKING;
    /* Call the control() API to configure the serial device by RT_SERIAL_RX_BLOCKING*/
    serial->ops->control(serial,
                        RT_DEVICE_CTRL_CONFIG,
                        (void *) RT_SERIAL_RX_BLOCKING);

    return RT_EOK;
}

/**
  * @brief Disable serial receive mode.
  * @param dev The pointer of device driver structure
  * @param rx_oflag The flag of that the serial port opens.
  * @return Return the status of the operation.
  */
static rt_err_t rt_serial_rx_disable(struct rt_device        *dev,
                                            rt_uint16_t       rx_oflag)
{
    struct rt_serial_device *serial;
    struct rt_serial_rx_fifo *rx_fifo;

    RT_ASSERT(dev != RT_NULL);
    serial = (struct rt_serial_device *)dev;

    dev->read = RT_NULL;

    if (serial->serial_rx == RT_NULL) return RT_EOK;

    do
    {
        if (rx_oflag == RT_SERIAL_RX_NON_BLOCKING)
        {
            dev->open_flag &= ~ RT_SERIAL_RX_NON_BLOCKING;
            serial->ops->control(serial,
                                RT_DEVICE_CTRL_CLR_INT,
                                (void *)RT_SERIAL_RX_NON_BLOCKING);
            break;
        }

        dev->open_flag &= ~ RT_SERIAL_RX_BLOCKING;
        serial->ops->control(serial,
                            RT_DEVICE_CTRL_CLR_INT,
                            (void *)RT_SERIAL_RX_BLOCKING);
    } while (0);

    rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
    RT_ASSERT(rx_fifo != RT_NULL);
    rt_free(rx_fifo);
    serial->serial_rx = RT_NULL;

    return RT_EOK;
}

/**
  * @brief Disable serial tranmit mode.
  * @param dev The pointer of device driver structure
  * @param rx_oflag The flag of that the serial port opens.
  * @return Return the status of the operation.
  */
static rt_err_t rt_serial_tx_disable(struct rt_device        *dev,
                                            rt_uint16_t       tx_oflag)
{
    struct rt_serial_device *serial;
    struct rt_serial_tx_fifo *tx_fifo;

    RT_ASSERT(dev != RT_NULL);
    serial = (struct rt_serial_device *)dev;

    dev->write = RT_NULL;

    if (serial->serial_tx == RT_NULL) return RT_EOK;

    tx_fifo = (struct rt_serial_tx_fifo *)serial->serial_tx;
    RT_ASSERT(tx_fifo != RT_NULL);

    do
    {
        if (tx_oflag == RT_SERIAL_TX_NON_BLOCKING)
        {
            dev->open_flag &= ~ RT_SERIAL_TX_NON_BLOCKING;

            serial->ops->control(serial,
                                RT_DEVICE_CTRL_CLR_INT,
                                (void *)RT_SERIAL_TX_NON_BLOCKING);
            break;
        }

        rt_completion_done(&(tx_fifo->tx_cpt));
        dev->open_flag &= ~ RT_SERIAL_TX_BLOCKING;
        serial->ops->control(serial,
                            RT_DEVICE_CTRL_CLR_INT,
                            (void *)RT_SERIAL_TX_BLOCKING);
    } while (0);

    rt_free(tx_fifo);
    serial->serial_tx = RT_NULL;

    return RT_EOK;
}

/**
  * @brief Initialize the serial device.
  * @param dev The pointer of device driver structure
  * @return Return the status of the operation.
  */
static rt_err_t rt_serial_init(struct rt_device *dev)
{
    rt_err_t result = RT_EOK;
    struct rt_serial_device *serial;

    RT_ASSERT(dev != RT_NULL);
    serial = (struct rt_serial_device *)dev;
    RT_ASSERT(serial->ops->transmit != RT_NULL);

    /* initialize rx/tx */
    serial->serial_rx = RT_NULL;
    serial->serial_tx = RT_NULL;

    /* apply configuration */
    if (serial->ops->configure)
        result = serial->ops->configure(serial, &serial->config);

    return result;
}

/**
  * @brief Open the serial device.
  * @param dev The pointer of device driver structure
  * @param oflag The flag of that the serial port opens.
  * @return Return the status of the operation.
  */
static rt_err_t rt_serial_open(struct rt_device *dev, rt_uint16_t oflag)
{
    struct rt_serial_device *serial;

    RT_ASSERT(dev != RT_NULL);
    serial = (struct rt_serial_device *)dev;

    /* Check that the device has been turned on */
    if ((dev->open_flag) & (15 << 12))
    {
        return RT_EOK;
    }

    /* By default, the receive mode of a serial devide is RT_SERIAL_RX_NON_BLOCKING */
    if ((oflag & RT_SERIAL_RX_BLOCKING) == RT_SERIAL_RX_BLOCKING)
        dev->open_flag |= RT_SERIAL_RX_BLOCKING;
    else
        dev->open_flag |= RT_SERIAL_RX_NON_BLOCKING;

    /* By default, the transmit mode of a serial devide is RT_SERIAL_TX_BLOCKING */
    if ((oflag & RT_SERIAL_TX_NON_BLOCKING) == RT_SERIAL_TX_NON_BLOCKING)
        dev->open_flag |= RT_SERIAL_TX_NON_BLOCKING;
    else
        dev->open_flag |= RT_SERIAL_TX_BLOCKING;

    /* set steam flag */
    if ((oflag & RT_DEVICE_FLAG_STREAM) ||
        (dev->open_flag & RT_DEVICE_FLAG_STREAM))
        dev->open_flag |= RT_DEVICE_FLAG_STREAM;

    /* initialize the Rx structure according to open flag */
    if (serial->serial_rx == RT_NULL)
        rt_serial_rx_enable(dev, dev->open_flag &
                            (RT_SERIAL_RX_BLOCKING | RT_SERIAL_RX_NON_BLOCKING));

    /* initialize the Tx structure according to open flag */
    if (serial->serial_tx == RT_NULL)
        rt_serial_tx_enable(dev, dev->open_flag &
                            (RT_SERIAL_TX_BLOCKING | RT_SERIAL_TX_NON_BLOCKING));

    return RT_EOK;
}


/**
  * @brief Close the serial device.
  * @param dev The pointer of device driver structure
  * @return Return the status of the operation.
  */
static rt_err_t rt_serial_close(struct rt_device *dev)
{
    struct rt_serial_device *serial;

    RT_ASSERT(dev != RT_NULL);
    serial = (struct rt_serial_device *)dev;

    /* this device has more reference count */
    if (dev->ref_count > 1) return -RT_ERROR;
    /* Disable serial receive mode. */
    rt_serial_rx_disable(dev, dev->open_flag &
                        (RT_SERIAL_RX_BLOCKING | RT_SERIAL_RX_NON_BLOCKING));
    /* Disable serial tranmit mode. */
    rt_serial_tx_disable(dev, dev->open_flag &
                        (RT_SERIAL_TX_BLOCKING | RT_SERIAL_TX_NON_BLOCKING));

    /* Clear the callback function */
    serial->parent.rx_indicate = RT_NULL;
    serial->parent.tx_complete = RT_NULL;

    /* Call the control() API to close the serial device */
    serial->ops->control(serial, RT_DEVICE_CTRL_CLOSE, RT_NULL);
    dev->flag &= ~RT_DEVICE_FLAG_ACTIVATED;

    return RT_EOK;
}

/**
  * @brief Control the serial device.
  * @param dev The pointer of device driver structure
  * @param cmd The command value that controls the serial device
  * @param args The parameter value that controls the serial device
  * @return Return the status of the operation.
  */
static rt_err_t rt_serial_control(struct rt_device *dev,
                                  int               cmd,
                                  void             *args)
{
    rt_err_t ret = RT_EOK;
    struct rt_serial_device *serial;

    RT_ASSERT(dev != RT_NULL);
    serial = (struct rt_serial_device *)dev;

    switch (cmd)
    {
        case RT_DEVICE_CTRL_SUSPEND:
            /* suspend device */
            dev->flag |= RT_DEVICE_FLAG_SUSPENDED;
            break;

        case RT_DEVICE_CTRL_RESUME:
            /* resume device */
            dev->flag &= ~RT_DEVICE_FLAG_SUSPENDED;
            break;

        case RT_DEVICE_CTRL_CONFIG:
            if (args != RT_NULL)
            {
                struct serial_configure *pconfig = (struct serial_configure *) args;
                if (serial->parent.ref_count)
                {
                    /*can not change buffer size*/
                    return -RT_EBUSY;
                }
               /* set serial configure */
                serial->config = *pconfig;
                serial->ops->configure(serial,
                                    (struct serial_configure *) args);
            }

            break;

        case RT_DEVICE_GET_DATA_LEN_OF_RX_BUFF:
            *((rt_size_t*)args) = rt_ringbuffer_data_len(&((struct rt_serial_rx_fifo*)serial->serial_rx)->rb);
            ret = RT_EOK;
            break;

        default :
            /* control device */
            ret = serial->ops->control(serial, cmd, args);
            break;
    }

    return ret;
}

/**
  * @brief Register the serial device.
  * @param serial RT-thread serial device.
  * @param name The device driver's name
  * @param flag The capabilities flag of device.
  * @param data The device driver's data.
  * @return Return the status of the operation.
  */
rt_err_t rt_hw_serial_register(struct rt_serial_device *serial,
                               const char              *name,
                               rt_uint32_t              flag,
                               void                    *data)
{
    rt_err_t ret;
    struct rt_device *device;
    RT_ASSERT(serial != RT_NULL);

    device = &(serial->parent);

    device->type        = RT_Device_Class_Char;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

    device->init        = rt_serial_init;
    device->open        = rt_serial_open;
    device->close       = rt_serial_close;
    device->read        = RT_NULL;
    device->write       = RT_NULL;
    device->control     = rt_serial_control;

    device->user_data   = data;

    /* register a character device */
    ret = rt_device_register(device, name, flag);

    return ret;
}

/**
  * @brief ISR for serial interrupt
  * @param serial RT-thread serial device.
  * @param event ISR event type.
  */
void rt_hw_serial_isr(struct rt_serial_device *serial, int event)
{
    RT_ASSERT(serial != RT_NULL);

    switch (event & 0xff)
    {
        /* Interrupt receive event */
        case RT_SERIAL_EVENT_RX_IND:
        case RT_SERIAL_EVENT_RX_DMADONE:
        {
            struct rt_serial_rx_fifo *rx_fifo;
            rt_size_t rx_length = 0;
            rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
            RT_ASSERT(rx_fifo != RT_NULL);

            /* If the event is RT_SERIAL_EVENT_RX_IND, rx_length is equal to 0 */
            rx_length = (event & (~0xff)) >> 8;

            if (rx_length)
                rt_serial_update_write_index(&(rx_fifo->rb), rx_length);

            /* Get the length of the data from the ringbuffer */
            rx_length = rt_ringbuffer_data_len(&rx_fifo->rb);
            if (rx_length == 0) break;

            if (serial->parent.open_flag & RT_SERIAL_RX_BLOCKING)
            {
                if (rx_fifo->rx_cpt_index && rx_length >= rx_fifo->rx_cpt_index )
                {
                    rx_fifo->rx_cpt_index = 0;
                    rt_completion_done(&(rx_fifo->rx_cpt));
                }
            }
            /* Trigger the receiving completion callback */
            if (serial->parent.rx_indicate != RT_NULL)
                serial->parent.rx_indicate(&(serial->parent), rx_length);
            break;
        }

        /* Interrupt transmit event */
        case RT_SERIAL_EVENT_TX_DONE:
        {
            struct rt_serial_tx_fifo *tx_fifo;
            rt_size_t tx_length = 0;
            tx_fifo = (struct rt_serial_tx_fifo *)serial->serial_tx;
            RT_ASSERT(tx_fifo != RT_NULL);

            /* Get the length of the data from the ringbuffer */
            tx_length = rt_ringbuffer_data_len(&tx_fifo->rb);
            /* If there is no data in tx_ringbuffer,
             * then the transmit completion callback is triggered*/
            if (tx_length == 0)
            {
                tx_fifo->activated = RT_FALSE;
                /* Trigger the transmit completion callback */
                if (serial->parent.tx_complete != RT_NULL)
                    serial->parent.tx_complete(&serial->parent, RT_NULL);

                if (serial->parent.open_flag & RT_SERIAL_TX_BLOCKING)
                    rt_completion_done(&(tx_fifo->tx_cpt));

                break;
            }

            /* Call the transmit interface for transmission again */
            /* Note that in interrupt mode, tx_fifo->buffer and tx_length
             * are inactive parameters */
            serial->ops->transmit(serial,
                                tx_fifo->buffer,
                                tx_length,
                                serial->parent.open_flag & ( \
                                RT_SERIAL_TX_BLOCKING | \
                                RT_SERIAL_TX_NON_BLOCKING));
            break;
        }

        case RT_SERIAL_EVENT_TX_DMADONE:
        {
            struct rt_serial_tx_fifo *tx_fifo;
            tx_fifo = (struct rt_serial_tx_fifo *)serial->serial_tx;
            RT_ASSERT(tx_fifo != RT_NULL);

            tx_fifo->activated = RT_FALSE;

            /* Trigger the transmit completion callback */
            if (serial->parent.tx_complete != RT_NULL)
                serial->parent.tx_complete(&serial->parent, RT_NULL);

            if (serial->parent.open_flag & RT_SERIAL_TX_BLOCKING)
            {
                rt_completion_done(&(tx_fifo->tx_cpt));
                break;
            }

            rt_serial_update_read_index(&tx_fifo->rb, tx_fifo->put_size);
            /* Get the length of the data from the ringbuffer.
             * If there is some data in tx_ringbuffer,
             * then call the transmit interface for transmission again */
            if (rt_ringbuffer_data_len(&tx_fifo->rb))
            {
                tx_fifo->activated = RT_TRUE;

                rt_uint8_t *put_ptr  = RT_NULL;
                /* Get the linear length buffer from rinbuffer */
                tx_fifo->put_size = rt_serial_get_linear_buffer(&(tx_fifo->rb), &put_ptr);
                /* Call the transmit interface for transmission again */
                serial->ops->transmit(serial,
                                    put_ptr,
                                    tx_fifo->put_size,
                                    RT_SERIAL_TX_NON_BLOCKING);
            }

            break;
        }

        default:
            break;
    }
}
