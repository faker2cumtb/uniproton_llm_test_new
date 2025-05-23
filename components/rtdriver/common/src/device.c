/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2007-01-21     Bernard      the first version
 * 2010-05-04     Bernard      add rt_device_init implementation
 * 2012-10-20     Bernard      add device check in register function,
 *                             provided by Rob <rdent@iinet.net.au>
 * 2012-12-25     Bernard      return RT_EOK if the device interface not exist.
 * 2013-07-09     Grissiom     add ref_count support
 * 2016-04-02     Bernard      fix the open_flag initialization issue.
 * 2021-03-19     Meco Man     remove rt_device_init_all()
 * 2024-07-12     LuoYuncong   Port to UniProton
 */

#include <rtapi.h>
#include <string.h>

#ifdef RT_USING_DEVICE

#define device_init (dev->init)
#define device_open (dev->open)
#define device_close (dev->close)
#define device_read (dev->read)
#define device_write (dev->write)
#define device_control (dev->control)

static rt_list_t _device_list = RT_LIST_OBJECT_INIT(_device_list);

void rt_object_init(struct rt_object *object, const char *name)
{
    if (object != RT_NULL)
    {
        strncpy(object->name, name, RT_NAME_MAX);
        rt_list_insert_after(&_device_list, &(object->list));
    }
}

void rt_object_detach(rt_object_t object)
{
    if (object != RT_NULL)
    {
        rt_list_remove(&(object->list));
    }
}

rt_object_t rt_object_find(const char *name)
{
    struct rt_object *object = RT_NULL;
    struct rt_list_node *node = RT_NULL;

    if (name == RT_NULL)
    {
        return RT_NULL;
    }

    rt_list_for_each(node, &_device_list)
    {
        object = rt_list_entry(node, struct rt_object, list);
        if (strncmp(object->name, name, RT_NAME_MAX) == 0)
        {
            return object;
        }
    }

    return RT_NULL;
}

/**
 * @brief This function finds a device driver by specified name.
 *
 * @param name is the device driver's name.
 *
 * @return the registered device driver on successful, or RT_NULL on failure.
 */
rt_device_t rt_device_find(const char *name)
{
    return (rt_device_t)rt_object_find(name);
}

/**
 * @brief This function registers a device driver with a specified name.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param name is the device driver's name.
 *
 * @param flags is the capabilities flag of device.
 *
 * @return the error code, RT_EOK on initialization successfully.
 */
rt_err_t rt_device_register(rt_device_t dev, const char *name, rt_uint16_t flags)
{
    if (dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    if (rt_device_find(name) != RT_NULL)
    {
        return -RT_ERROR;
    }

    rt_object_init(&(dev->parent), name);
    dev->flag = flags;
    dev->ref_count = 0;
    dev->open_flag = 0;

    return RT_EOK;
}

/**
 * @brief This function removes a previously registered device driver.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @return the error code, RT_EOK on successfully.
 */
rt_err_t rt_device_unregister(rt_device_t dev)
{
    if (dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    rt_object_detach(&(dev->parent));

    return RT_EOK;
}

/**
 * @brief This function will initialize the specified device.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @return the result, RT_EOK on successfully.
 */
rt_err_t rt_device_init(rt_device_t dev)
{
    rt_err_t result = RT_EOK;

    if (dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    if (device_init != RT_NULL)
    {
        if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))
        {
            result = device_init(dev);
            if (result != RT_EOK)
            {
                RT_DEBUG_LOG(RT_DEBUG_DEVICE, ("To initialize device:%s failed. The error code is %d\n",
                                               dev->parent.name, result));
            }
            else
            {
                dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
            }
        }
    }

    return result;
}

/**
 * @brief This function will open a device.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param oflag is the flags for device open.
 *
 * @return the result, RT_EOK on successfully.
 */
rt_err_t rt_device_open(rt_device_t dev, rt_uint16_t oflag)
{
    rt_err_t result = RT_EOK;

    if (dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    /* if device is not initialized, initialize it. */
    if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))
    {
        if (device_init != RT_NULL)
        {
            result = device_init(dev);
            if (result != RT_EOK)
            {
                return result;
            }
        }

        dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
    }

    /* device is a stand alone device and opened */
    if ((dev->flag & RT_DEVICE_FLAG_STANDALONE) && (dev->open_flag & RT_DEVICE_OFLAG_OPEN))
    {
        return -RT_EBUSY;
    }

    if (device_open != RT_NULL)
    {
        result = device_open(dev, oflag);
    }
    else
    {
        dev->open_flag = (oflag & RT_DEVICE_OFLAG_MASK);
    }

    if (result == RT_EOK || result == -RT_ENOSYS)
    {
        dev->open_flag |= RT_DEVICE_OFLAG_OPEN;

        dev->ref_count++;
        /* don't let bad things happen silently. If you are bitten by this assert,
         * please set the ref_count to a bigger type. */
        RT_ASSERT(dev->ref_count != 0);
    }

    return result;
}

/**
 * @brief This function will close a device.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @return the result, RT_EOK on successfully.
 */
rt_err_t rt_device_close(rt_device_t dev)
{
    rt_err_t result = RT_EOK;

    if (dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    if (dev->ref_count == 0)
    {
        return -RT_ERROR;
    }

    dev->ref_count--;

    if (dev->ref_count != 0)
    {
        return RT_EOK;
    }

    if (device_close != RT_NULL)
    {
        result = device_close(dev);
    }

    if (result == RT_EOK || result == -RT_ENOSYS)
    {
        dev->open_flag = RT_DEVICE_OFLAG_CLOSE;
    }

    return result;
}

/**
 * @brief This function will read some data from a device.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param pos is the position when reading.
 *
 * @param buffer is a data buffer to save the read data.
 *
 * @param size is the size of buffer.
 *
 * @return the actually read size on successful, otherwise 0 will be returned.
 *
 * @note the unit of size/pos is a block for block device.
 */
rt_size_t rt_device_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    if (dev == RT_NULL)
    {
        return 0;
    }

    if (dev->ref_count == 0)
    {
        return 0;
    }

    if (device_read != RT_NULL)
    {
        return device_read(dev, pos, buffer, size);
    }

    return 0;
}

/**
 * @brief This function will write some data to a device.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param pos is the position when writing.
 *
 * @param buffer is the data buffer to be written to device.
 *
 * @param size is the size of buffer.
 *
 * @return the actually written size on successful, otherwise 0 will be returned.
 *
 * @note the unit of size/pos is a block for block device.
 */
rt_size_t rt_device_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    if (dev == RT_NULL)
    {
        return 0;
    }

    if (dev->ref_count == 0)
    {
        return 0;
    }

    /* call device_write interface */
    if (device_write != RT_NULL)
    {
        return device_write(dev, pos, buffer, size);
    }

    return 0;
}

/**
 * @brief This function will perform a variety of control functions on devices.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param cmd is the command sent to device.
 *
 * @param arg is the argument of command.
 *
 * @return the result, -RT_ENOSYS for failed.
 */
rt_err_t rt_device_control(rt_device_t dev, int cmd, void *arg)
{
    if (dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    if (device_control != RT_NULL)
    {
        return device_control(dev, cmd, arg);
    }

    return -RT_ENOSYS;
}

/**
 * @brief This function will set the reception indication callback function. This callback function
 *        is invoked when this device receives data.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param rx_ind is the indication callback function.
 *
 * @return RT_EOK
 */
rt_err_t rt_device_set_rx_indicate(rt_device_t dev, rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size))
{
    if (dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    dev->rx_indicate = rx_ind;

    return RT_EOK;
}

/**
 * @brief This function will set a callback function. The callback function
 *        will be called when device has written data to physical hardware.
 *
 * @param dev is the pointer of device driver structure.
 *
 * @param tx_done is the indication callback function.
 *
 * @return RT_EOK
 */
rt_err_t rt_device_set_tx_complete(rt_device_t dev, rt_err_t (*tx_done)(rt_device_t dev, void *buffer))
{
    if (dev == RT_NULL)
    {
        return -RT_ERROR;
    }

    dev->tx_complete = tx_done;

    return RT_EOK;
}

#endif /* RT_USING_DEVICE */
