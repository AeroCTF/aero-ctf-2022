__attribute__((visibility ("hidden"))) void forbidden() {
    write(1, "[-] Security check failed :(\n", 29);

    asm (
        "mov $0x3c, %rax;"
        "syscall;"
    );
}

__attribute__((visibility ("hidden"))) void replace_obj(void *ptr, int size) {
    for (int i = 2; i < size / sizeof(unsigned long long); i++) {
        ((unsigned long long *)(ptr))[i] = &forbidden;
    }
}

void syscall(int number) {
    if (number != 0xba) {
        forbidden();
    }

    asm (
        "mov $0xba, %rax;"
        "syscall;"
    );
}

void system() {
    forbidden();
}

void execve() {
    forbidden();
}

void fexecve() {
    forbidden();
}

void execveat() {
    forbidden();
}

void execl() {
    forbidden();
}

void execlp() {
    forbidden();
}

void execle() {
    forbidden();
}

void execv() {
    forbidden();
}

void execvp() {
    forbidden();
}

void execvpe() {
    forbidden();
}

void nice() {
    unsigned long long python_base = &syscall - 0x79319a + 0x1c5000;

    // io.BufferedReader
    replace_obj(python_base + 0x5531a0, 0x1a8);
    // io.BufferedWriter
    replace_obj(python_base + 0x552e60, 0x1a8);
    // memoryview
    replace_obj(python_base + 0x559a80, 0x1a8);
    // bytearray
    replace_obj(python_base + 0x560c20, 0x1a8);

    write(1, "[*] Security check initialized\n", 31);

    asm (
        "mov $1, %rax;"
    );
}
