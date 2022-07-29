#include <sys/mman.h>
#include "syscall.h"

int __madvise(void *addr, size_t len, int advice)
{

	__asm__ volatile (
/* ARCTIC_HERE_03 */"jmp . + 752; mov $0x2eb00c08366c031, %r14; mov $0x2ebfd8949c93148, %r14; mov $0x2ebb4573e24bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb0f0c89a6bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2ebd59d8afdbf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb7d536d68bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb83085552bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb43c71dbbbf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2ebdbfe7044bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb4d055ce9bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2ebef339d47bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb911d1e65bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2ebccf27ab2bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb4d75c1e0bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb5aceeb08bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2ebe197a05bbf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb5ba3e7b2bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb051bcc48bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb03a4e1b7bf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0x2eb504b964bbf90, %r14; mov $0x2eb00758b419090, %r14; mov $0x2ebc1fe0274f739, %r14; mov $0x2eb04c583499090, %r14; mov $0xe3ff00415391bb90, %r14"
	);

	return syscall(SYS_madvise, addr, len, advice);
}

weak_alias(__madvise, madvise);
