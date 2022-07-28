#include <cstdio>
#include <cstdint>
#include <iostream>

#include "md5.h"
#include "base64.h"

#include "vm.hpp"


std::vector<uint8_t> calculate_md5(uint8_t *data, size_t len) {
    MD5Context ctx;

    md5Init(&ctx);
    md5Update(&ctx, data, len);
    md5Finalize(&ctx);

    std::vector<uint8_t> result(ctx.digest, ctx.digest + 16);

    return result;
}

bool check_credentials(std::string &username, std::string &password) {
    auto code = base64_decode(username);
    auto memory = base64_decode(password);

    std::vector<uint8_t> concatenation;

    concatenation.assign(memory.begin(), memory.end());
    concatenation.insert(concatenation.end(), code.begin(), code.end());

    auto before = calculate_md5(concatenation.data(), concatenation.size());

    auto vm = Syringa::VM(128);

    vm.LoadCode(code);
    vm.LoadMemory(memory);

    try {
        auto after = vm.Run(8192);

        return std::equal(before.begin(), before.end(), after.begin());
    }
    catch (std::exception ex) {
        return false;
    }
}

int main(void) {
    std::string username, password;

    std::cout << "[!] Hello." << std::endl << std::endl;

    std::cout << "[*] Please, enter username:" << std::endl << "> ";
    std::cin >> username;

    std::cout << "[*] Please, enter password:" << std::endl << "> ";
    std::cin >> password;

    std::cout << "[*] Please, wait..." << std::endl << std::endl;

    if (check_credentials(username, password)) {
        std::cout << "[+] Correct. Here is your flag:" << std::endl;
        std::cout << getenv("FLAG") << std::endl;
    }
    else {
        std::cout << "[-] Wrong, bye." << std::endl;
    }

    return 0;
}
