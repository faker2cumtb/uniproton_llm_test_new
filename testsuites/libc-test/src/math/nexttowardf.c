#include <stdint.h>
#include <stdio.h>
#include "mtest.h"

// TODO: separate ldbl dir
static struct ll_l t[] = {
#if LDBL_MANT_DIG == 53
#include "sanity/nexttowardf.h"
#include "special/nexttowardf.h"

#elif LDBL_MANT_DIG == 64
#include "sanity/nexttowardf.h"
#include "special/nexttowardf.h"

#endif
};

int nexttowardf_test(void)
{
	#pragma STDC FENV_ACCESS ON
	float y;
	float d;
	int e, i, err = 0;
	struct ll_l *p;

	for (i = 0; i < sizeof t/sizeof *t; i++) {
		p = t + i;

		if (p->r < 0)
			continue;
		fesetround(p->r);
		feclearexcept(FE_ALL_EXCEPT);
		y = nexttowardf(p->x, p->x2);
		e = fetestexcept(INEXACT|INVALID|DIVBYZERO|UNDERFLOW|OVERFLOW);

		if (!checkexceptall(e, p->e, p->r)) {
			printf("%s:%d: bad fp exception: %s nexttowardf(%La,%La)=%La, want %s",
				p->file, p->line, rstr(p->r), p->x, p->x2, p->y, estr(p->e));
			printf(" got %s\n", estr(e));
			err++;
		}
		d = ulperrf(y, p->y, p->dy);
		if (!checkcr(y, p->y, p->r)) {
			printf("%s:%d: %s nexttowardf(%La,%La) want %La got %a ulperr %.3f = %a + %a\n",
				p->file, p->line, rstr(p->r), p->x, p->x2, p->y, y, d, d-p->dy, p->dy);
			err++;
		}
	}
	return !!err;
}
