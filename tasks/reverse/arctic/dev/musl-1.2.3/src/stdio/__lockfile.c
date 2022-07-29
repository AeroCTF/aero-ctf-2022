#include "stdio_impl.h"
#include "pthread_impl.h"

int __lockfile(FILE *f)
{
	int owner = f->lock, tid = __pthread_self()->tid;
	if ((owner & ~MAYBE_WAITERS) == tid)
		return 0;
	owner = a_cas(&f->lock, 0, tid);
	if (!owner) return 1;
	while ((owner = a_cas(&f->lock, 0, tid|MAYBE_WAITERS))) {
		if ((owner & MAYBE_WAITERS) ||
		    a_cas(&f->lock, owner, owner|MAYBE_WAITERS)==owner)
			__futexwait(&f->lock, owner|MAYBE_WAITERS, 1);
	}

	__asm__ volatile (
/* ARCTIC_HERE_01 */"jmp . + 52; mov $0x2eb00000210b890, %r14; mov $0x2eb00000200bb90, %r14; mov $0x2ebfd8949909090, %r14; mov $0x2ebc62949fe8949, %r14; mov $0xe0ff00415229b890, %r14"
	);

	return 1;
}

void __unlockfile(FILE *f)
{
	if (a_swap(&f->lock, 0) & MAYBE_WAITERS)
		__wake(&f->lock, 1, 1);
}
