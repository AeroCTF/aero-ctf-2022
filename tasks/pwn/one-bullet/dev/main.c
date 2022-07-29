#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define NULLPTR NULL
#define STDIN_FD 0

char art[] = "\n"\
"            ,___________________________________________/7\n"\
"           |-_______------. `\\                             |\n"\
"       _,/ | _______)     |___\\____________________________|\n"\
"  .__/`((  | _______      | (/))_______________=.\n"\
"     `~) \\ | _______)     |   /----------------_/\n"\
"       `__y|______________|  /\n"\
"       / ________ __________/\n"\
"      / /#####\\(  \\  /     ))\n"\
"     / /#######|\\  \\(     //\n"\
"    / /########|.\\______ad/`\n"\
"   / /###(\\)###||`------``\n"\
"  / /##########||\n"\
" / /###########||\n"\
"( (############||\n"\
" \\ \\####(/)####))\n"\
"  \\ \\#########//\n"\
"   \\ \\#######//\n"\
"    `---|_|--`\n"\
"       ((_))\n"\
"        `-`";

void setup(void);
ssize_t read_int(void);
ssize_t read_into_buffer(char *buffer, ssize_t size);
void feedback(void);
void shoot(void);

int main() {
    size_t shoots = 6;
    while (shoots != 0) {
        puts(art);
        printf("%u shots left!\n", shoots);
        shoot();
        shoots--;
    }
    
    feedback();
    return 0;
}

void __attribute__ ((constructor)) setup(void) {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags     = SA_NODEFER;
    sa.sa_sigaction = exit;

    sigaction(SIGSEGV, &sa, NULL);
}

void shoot(void) {
    ssize_t x, y;
    printf("Where do you want to shoot? x: ");
    x = read_int();
    printf("y: ");
    y = read_int();
    puts("!!!BAAAAAANG!!!");
};

void feedback(void) {
    size_t commentSize;
    char * buf = NULL;

    puts("{!} Please leave a comment about shooting!");
    printf("{?} Comment size: ");
    scanf("%lld", commentSize);

    if (commentSize > 64)
        commentSize = 64;

    buf = (char*) malloc(commentSize);
    printf("{?} Comment: ");
    read_into_buffer(buf, commentSize);
    puts("{+} Thanks!");

    exit(1);
};

ssize_t read_into_buffer(char *buffer, ssize_t size) {
    if (buffer == NULLPTR) {
        return 0;
    }

    if (size == 0) {
        return 0;
    }

    ssize_t nbytes = read(STDIN_FD, (void*)buffer, size);

    if (nbytes <= 0) {
        puts("[-] Error in read into buffer!");
    }

    if (buffer[nbytes - 1] == '\n') {
        buffer[nbytes - 1] = '\0';
    }
    return nbytes;
}

ssize_t read_int(void) {
    size_t bufSize = 16;
    char buf[bufSize];
    int nbytes = read_into_buffer(&buf[0], bufSize);
    
    if (nbytes <= 0) {
        return 0;
    }

    return atoll(buf);
}