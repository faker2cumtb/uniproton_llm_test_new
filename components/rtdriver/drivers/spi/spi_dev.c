/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-19     LuoYuncong   Port to UniProton
 */

#include <rtapi.h>
#include <drivers/spi.h>

/* SPI bus device interface, compatible with RT-Thread 0.3.x/1.0.x */
static rt_size_t _spi_bus_device_read(rt_device_t dev,
                                      rt_off_t    pos,
                                      void       *buffer,
                                      rt_size_t   size)
{
    struct rt_spi_bus *bus;

    bus = (struct rt_spi_bus *)dev;
    RT_ASSERT(bus != RT_NULL);
    RT_ASSERT(bus->owner != RT_NULL);

    return rt_spi_transfer(bus->owner, RT_NULL, buffer, size);
}

static rt_size_t _spi_bus_device_write(rt_device_t dev,
                                       rt_off_t    pos,
                                       const void *buffer,
                                       rt_size_t   size)
{
    struct rt_spi_bus *bus;

    bus = (struct rt_spi_bus *)dev;
    RT_ASSERT(bus != RT_NULL);
    RT_ASSERT(bus->owner != RT_NULL);

    return rt_spi_transfer(bus->owner, buffer, RT_NULL, size);
}

rt_err_t rt_spi_bus_device_init(struct rt_spi_bus *bus, const char *name)
{
    struct rt_device *device;
    RT_ASSERT(bus != RT_NULL);

    device = &bus->parent;

    /* set device type */
    device->type    = RT_Device_Class_SPIBUS;
    /* initialize device interface */
    device->init    = RT_NULL;
    device->open    = RT_NULL;
    device->close   = RT_NULL;
    device->read    = _spi_bus_device_read;
    device->write   = _spi_bus_device_write;
    device->control = RT_NULL;

    /* register to device manager */
    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR);
}

/* SPI Dev device interface, compatible with RT-Thread 0.3.x/1.0.x */
static rt_size_t _spidev_device_read(rt_device_t dev,
                                     rt_off_t    pos,
                                     void       *buffer,
                                     rt_size_t   size)
{
    struct rt_spi_device *device;

    device = (struct rt_spi_device *)dev;
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);

    return rt_spi_transfer(device, RT_NULL, buffer, size);
}

static rt_size_t _spidev_device_write(rt_device_t dev,
                                      rt_off_t    pos,
                                      const void *buffer,
                                      rt_size_t   size)
{
    struct rt_spi_device *device;

    device = (struct rt_spi_device *)dev;
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);

    return rt_spi_transfer(device, buffer, RT_NULL, size);
}

static rt_err_t _spidev_device_control(rt_device_t dev,
                                       int         cmd,
                                       void       *args)
{
    switch (cmd)
    {
    case 0: /* set device */
        break;
    case 1:
        break;
    }

    return RT_EOK;
}

rt_err_t rt_spidev_device_init(struct rt_spi_device *dev, const char *name)
{
    struct rt_device *device;
    RT_ASSERT(dev != RT_NULL);

    device = &(dev->parent);

    /* set device type */
    device->type    = RT_Device_Class_SPIDevice;

    device->init    = RT_NULL;
    device->open    = RT_NULL;
    device->close   = RT_NULL;
    device->read    = _spidev_device_read;
    device->write   = _spidev_device_write;
    device->control = _spidev_device_control;

    /* register to device manager */
    return rt_device_register(device, name, RT_DEVICE_FLAG_RDWR);
}
