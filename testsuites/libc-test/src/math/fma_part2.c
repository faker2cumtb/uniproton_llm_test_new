#include <stdint.h>
#include <stdio.h>
#include "mtest.h"

#define SPECIAL_FMA_PART_2
static struct ddd_d t[] = {
#include "special/fma.h"
};

int fma_part2_test(void)
{
	#pragma STDC FENV_ACCESS ON
	double y;
	float d;
	int e, i, err = 0;
	struct ddd_d *p;

	for (i = 0; i < sizeof t/sizeof *t; i++) {
		p = t + i;

		if (p->r < 0)
			continue;
		fesetround(p->r);
		feclearexcept(FE_ALL_EXCEPT);
		y = fma(p->x, p->x2, p->x3);
		e = fetestexcept(INEXACT|INVALID|DIVBYZERO|UNDERFLOW|OVERFLOW);

		/* do not check inexact by default */
#if defined CHECK_INEXACT || defined CHECK_INEXACT_OMISSION
		if (!checkexceptall(e, p->e, p->r)) {
#else
		if (!checkexceptall(e|INEXACT, p->e|INEXACT, p->r)) {
#endif
			printf("%s:%d: bad fp exception: %s fma(%a,%a,%a)=%a, want %s",
				p->file, p->line, rstr(p->r), p->x, p->x2, p->x3, p->y, estr(p->e));
			printf(" got %s\n", estr(e));
			err++;
		}
		d = ulperr(y, p->y, p->dy);
		if (!checkcr(y, p->y, p->r)) {
			printf("%s:%d: %s fma(%a,%a,%a) want %a got %a ulperr %.3f = %a + %a\n",
				p->file, p->line, rstr(p->r), p->x, p->x2, p->x3, p->y, y, d, d-p->dy, p->dy);
			err++;
		}
	}
	return !!err;
}
