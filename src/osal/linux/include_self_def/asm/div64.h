/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_DIV64_H__
#define __ASM_DIV64_H__

/**
 * do_div - returns 2 values: calculate remainder and update new dividend
 * @n: uint64_t dividend (will be updated)
 * @base: uint32_t divisor
 *
 * Summary:
 * ``uint32_t remainder = n % base;``
 * ``n = n / base;``
 *
 * Return: (uint32_t)remainder
 *
 * NOTE: macro parameter @n is evaluated multiple times,
 * beware of side effects!
 */
# define do_div(n,base) ({                    \
    uint32_t __base = (base);                \
    uint32_t __rem;                        \
    __rem = ((uint64_t)(n)) % __base;            \
    (n) = ((uint64_t)(n)) / __base;                \
    __rem;                            \
 })

#endif /* __ASM_DIV64_H__ */