set(SRCS
    dwc_eth_qos.c
    phy.c
    rk_gmac.c
    )
    
add_library(gmac OBJECT ${SRCS})
