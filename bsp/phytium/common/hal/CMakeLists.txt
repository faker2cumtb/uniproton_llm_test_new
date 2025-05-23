set(SRCS
    common/fassert.c
    common/fcache.c

    fgpio/fgpio_g.c
    fgpio/fgpio_intr.c
    fgpio/fgpio_sinit.c
    fgpio/fgpio.c

    fcan/fcan_g.c
    fcan/fcan_hw.c
    fcan/fcan_intr.c
    fcan/fcan_sinit.c
    fcan/fcan.c

    fi2c/fi2c_g.c
    fi2c/fi2c_hw.c
    fi2c/fi2c_intr.c
    fi2c/fi2c_master.c
    fi2c/fi2c_sinit.c
    fi2c/fi2c.c

    fiopad/fiopad_g.c
    fiopad/fiopad_hw.c
    fiopad/fiopad_sinit.c
    fiopad/fiopad.c

    fmio/fmio_g.c
    fmio/fmio_hw.c
    fmio/fmio_sinit.c
    fmio/fmio.c

    fpl011/fpl011_g.c
    fpl011/fpl011_hw.c
    fpl011/fpl011_intr.c
    fpl011/fpl011_options.c
    fpl011/fpl011_sinit.c
    fpl011/fpl011.c

    fspim/fspim_g.c
    fspim/fspim_hw.c
    fspim/fspim_intr.c
    fspim/fspim_sinit.c
    fspim/fspim.c

    ftimer_tacho/ftimer_tacho_g.c
    ftimer_tacho/ftimer_tacho_intr.c
    ftimer_tacho/ftimer.c
    ftimer_tacho/ftacho.c

    port/fdrivers_port.c

    fxmac/fxmac_bdring.c
    fxmac/fxmac_debug.c
    fxmac/fxmac_g.c
    fxmac/fxmac_intr.c
    fxmac/fxmac_options.c
    fxmac/fxmac_phy.c
    fxmac/fxmac_sinit.c
    fxmac/fxmac.c
    fxmac/fxmac_port.c
    fxmac/fxmac_port_nolwip.c
    )
    
add_library(hal OBJECT ${SRCS})