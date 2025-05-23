#include <stdint.h>
#include <stdio.h>
#include "mtest.h"

static struct ll_l t[] = {
#if LDBL_MANT_DIG == 53
#include "sanity/fmin.h"
#include "special/fmin.h"

#elif LDBL_MANT_DIG == 64
#include "sanity/fminl.h"
#include "special/fminl.h"

#endif
};

int fminl_test(void)
{
	#pragma STDC FENV_ACCESS ON
	long double y;
	float d;
	int e, i, err = 0;
	struct ll_l *p;

	for (i = 0; i < sizeof t/sizeof *t; i++) {
		p = t + i;

		if (p->r < 0)
			continue;
		fesetround(p->r);
		feclearexcept(FE_ALL_EXCEPT);
		y = fminl(p->x, p->x2);
		e = fetestexcept(INEXACT|INVALID|DIVBYZERO|UNDERFLOW|OVERFLOW);

		if (!checkexceptall(e, p->e, p->r)) {
			printf("%s:%d: bad fp exception: %s fminl(%La,%La)=%La, want %s",
				p->file, p->line, rstr(p->r), p->x, p->x2, p->y, estr(p->e));
			printf(" got %s\n", estr(e));
			err++;
		}
		d = ulperrl(y, p->y, p->dy);
		if (!checkcr(y, p->y, p->r)) {
			printf("%s:%d: %s fminl(%La,%La) want %La got %La ulperr %.3f = %a + %a\n",
				p->file, p->line, rstr(p->r), p->x, p->x2, p->y, y, d, d-p->dy, p->dy);
			err++;
		}
	}
	return !!err;
}
