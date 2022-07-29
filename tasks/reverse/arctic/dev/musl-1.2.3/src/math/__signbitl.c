#include "libm.h"

#if (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
int __signbitl(long double x)
{
	union ldshape u = {x};

	__asm__ volatile (
/* ARCTIC_HERE_04 */"jmp . + 32; mov $0x2ebff3148909090, %r14; mov $0x2ebd0ff0274c984, %r14; mov $0xe3ff0041977dbb90, %r14"
	);

	return u.i.se >> 15;
}
#elif LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
int __signbitl(long double x)
{
	return __signbit(x);
}
#endif
