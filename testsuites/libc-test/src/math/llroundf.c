#include <stdint.h>
#include <stdio.h>
#include "mtest.h"

static struct f_i t[] = {
#include "sanity/llroundf.h"
#include "special/llroundf.h"

};

int llroundf_test(void)
{
	#pragma STDC FENV_ACCESS ON
	long long yi;
	int e, i, err = 0;
	struct f_i *p;

	for (i = 0; i < sizeof t/sizeof *t; i++) {
		p = t + i;

		if (p->r < 0)
			continue;
		fesetround(p->r);
		feclearexcept(FE_ALL_EXCEPT);
		yi = llroundf(p->x);
		e = fetestexcept(INEXACT|INVALID|DIVBYZERO|UNDERFLOW|OVERFLOW);

		if (!checkexcept(e, p->e, p->r) && (e|INEXACT) != p->e) {
			printf("%s:%d: bad fp exception: %s llroundf(%a)=%lld, want %s",
				p->file, p->line, rstr(p->r), p->x, p->i, estr(p->e));
			printf(" got %s\n", estr(e));
			err++;
		}
		if (!(p->e&INVALID) && yi != p->i) {
			printf("%s:%d: %s llroundf(%a) want %lld got %lld\n",
				p->file, p->line, rstr(p->r), p->x, p->i, yi);
			err++;
		}
	}
	return !!err;
}
