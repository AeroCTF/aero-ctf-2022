#ifndef _VM_HPP
#define _VM_HPP

#include <cstddef>
#include <cstdint>

#include <stack>
#include <vector>
#include <stdexcept>

namespace Syringa {
    using CodeElement = uint8_t;
    using MemoryElement = uint8_t;

    class VM {
    public:
        VM(size_t stack_size) {
            this->stack_size = stack_size;

            this->code_ptr = 0;
            this->is_stopped = false;
        }

        void LoadCode(const std::vector<CodeElement> &code) {
            this->code.reserve(code.size());
            this->code.assign(code.begin(), code.end());
        }

        void LoadMemory(const std::vector<MemoryElement> &memory) {
            this->memory.reserve(memory.size());
            this->memory.assign(memory.begin(), memory.end());
        }

        std::vector<MemoryElement> Run(size_t max_iterations);

    private:
        std::stack<size_t> stack;
        std::vector<CodeElement> code;
        std::vector<MemoryElement> memory;
        size_t code_ptr, stack_size;
        bool is_stopped;

        std::vector<CodeElement> opcodes {
            0x45, 0x35, 0x3b, 0x67, 0xd7, 0xb0, 0x0c, 0xa5, 0x53, 0x8f, 0xab, 0x05, 0xbe, 0xe7, 0x81, 0x56, 0x9c, 0x3c, 0xec, 0xd0, 0x55, 0xe1,
        };

        void Mutate();
        void Execute();

        std::tuple<size_t, size_t> PrepareBinaryOperation();

        void Add();
        void Sub();
        void Mul();
        void Div();
        void Mod();

        void Or();
        void And();
        void Xor();

        void Jump();
        void JumpIf(bool zero);
        void JumpIfZero();
        void JumpIfNotZero();

        void Shift();
        void ShiftIf(bool zero);
        void ShiftIfZero();
        void ShiftIfNotZero();

        void Pop();
        void Push();
        void Swap();
        void Dup();

        void Put();
        void Get();

        void Nop();
        void Stop();
    };
}

#endif // _VM_HPP
