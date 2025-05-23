set(SRCS
    pcie_snps_dw.c
    pcie_rk_phy.c
    pcie_config.c
    pcie.c
    )
    
add_library(pcie OBJECT ${SRCS})
