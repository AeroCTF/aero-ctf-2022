#include <unordered_map>

#include "vm.hpp"


namespace Syringa {
    using Operation = void (VM::*)();

    const static size_t OperationsCount = 22;

    void VM::Execute() {
        if (this->code_ptr < 0 || this->code_ptr >= this->code.size()) {
            throw std::exception();
        }

        std::vector<Operation> operations {
            &VM::Add, &VM::Sub, &VM::Mul, &VM::Div, &VM::Mod, &VM::Or, &VM::And, &VM::Xor, &VM::Jump, &VM::JumpIfZero, &VM::JumpIfNotZero, &VM::Shift, &VM::ShiftIfZero, &VM::ShiftIfNotZero, &VM::Pop, &VM::Push, &VM::Swap, &VM::Dup, &VM::Put, &VM::Get, &VM::Nop, &VM::Stop,
        };

        std::unordered_map<CodeElement, Operation> transitions;

        for (size_t i = 0; i < OperationsCount; i++) {
            transitions[this->opcodes.at(i)] = operations.at(i);
        }

        auto opcode = this->code.at(this->code_ptr);

        if (!transitions.contains(opcode)) {
            throw std::exception();
        }

        auto operation = transitions.at(opcode);

        (this->*operation)();
    }

    void VM::Mutate() {
        
    }

    std::tuple<size_t, size_t> VM::PrepareBinaryOperation() {
        if (this->stack.size() < 2) {
            throw std::exception();
        }

        size_t dst = this->stack.top();
        this->stack.pop();
        
        size_t src = this->stack.top();
        this->stack.pop();

        if (dst < 0 || dst >= this->memory.size()) {
            throw std::exception();
        }

        if (src < 0 || src >= this->memory.size()) {
            throw std::exception();
        }

        return {dst, src};
    }

    void VM::Add() {
        auto [dst, src] = PrepareBinaryOperation();

        this->memory[dst] = this->memory.at(dst) + this->memory.at(src);
    }

    void VM::Sub() {
        auto [dst, src] = PrepareBinaryOperation();

        this->memory[dst] = this->memory.at(dst) - this->memory.at(src);
    }

    void VM::Mul() {
        auto [dst, src] = PrepareBinaryOperation();

        this->memory[dst] = this->memory.at(dst) * this->memory.at(src);
    }

    void VM::Div() {
        auto [dst, src] = PrepareBinaryOperation();

        this->memory[dst] = this->memory.at(dst) / this->memory.at(src);
    }

    void VM::Mod() {
        auto [dst, src] = PrepareBinaryOperation();

        this->memory[dst] = this->memory.at(dst) % this->memory.at(src);
    }

    void VM::Or() {
        auto [dst, src] = PrepareBinaryOperation();

        this->memory[dst] = this->memory.at(dst) | this->memory.at(src);
    }

    void VM::And() {
        auto [dst, src] = PrepareBinaryOperation();

        this->memory[dst] = this->memory.at(dst) & this->memory.at(src);
    }

    void VM::Xor() {
        auto [dst, src] = PrepareBinaryOperation();

        this->memory[dst] = this->memory.at(dst) ^ this->memory.at(src);
    }

    void VM::Jump() {
        if (this->stack.size() < 1) {
            throw std::exception();
        }

        size_t dst = this->stack.top();
        this->stack.pop();

        if (dst < 0 || dst >= this->code.size()) {
            throw std::exception();
        }

        this->code_ptr = dst;
    }

    void VM::JumpIf(bool zero) {
        if (this->stack.size() < 2) {
            throw std::exception();
        }

        size_t dst = this->stack.top();
        this->stack.pop();

        size_t cond = this->stack.top();
        this->stack.pop();

        if (dst < 0 || dst >= this->code.size()) {
            throw std::exception();
        }

        if (cond < 0 || cond >= this->memory.size()) {
            throw std::exception();
        }

        if ((this->memory.at(cond) == 0) == zero) {
            this->code_ptr = dst;
        }
    }

    void VM::JumpIfZero() {
        return VM::JumpIf(true);
    }

    void VM::JumpIfNotZero() {
        return VM::JumpIf(false);
    }

    void VM::Shift() {
        if (this->stack.size() < 1) {
            throw std::exception();
        }

        size_t offset = this->stack.top();
        this->stack.pop();

        if (this->code_ptr + offset < 0 || this->code_ptr + offset >= this->code.size()) {
            throw std::exception();
        }

        this->code_ptr += offset;
    }

    void VM::ShiftIf(bool zero) {
        if (this->stack.size() < 2) {
            throw std::exception();
        }

        size_t offset = this->stack.top();
        this->stack.pop();

        size_t cond = this->stack.top();
        this->stack.pop();

        if (this->code_ptr + offset < 0 || this->code_ptr + offset >= this->code.size()) {
            throw std::exception();
        }

        if (cond < 0 || cond >= this->memory.size()) {
            throw std::exception();
        }

        if ((this->memory.at(cond) == 0) == zero) {
            this->code_ptr += offset;
        }
    }

    void VM::ShiftIfZero() {
        return VM::ShiftIf(true);
    }

    void VM::ShiftIfNotZero() {
        return VM::ShiftIf(false);
    }

    void VM::Pop() {
        if (this->stack.size() < 1) {
            throw std::exception();
        }

        this->stack.pop();
    }

    void VM::Push() {
        if (this->code_ptr + sizeof(size_t) >= this->code.size()) {
            throw std::exception();
        }

        size_t value = reinterpret_cast<size_t *>(&(this->code.at(this->code_ptr + 1)))[0];
        this->stack.push(value);

        this->code_ptr += sizeof(size_t);
    }

    void VM::Swap() {
        if (this->stack.size() < 2) {
            throw std::exception();
        }

        size_t first = this->stack.top();
        this->stack.pop();

        size_t second = this->stack.top();
        this->stack.pop();

        this->stack.push(first);
        this->stack.push(second);
    }
    
    void VM::Dup() {
        if (this->stack.size() < 1) {
            throw std::exception();
        }

        size_t element = this->stack.top();
        this->stack.push(element);
    }

    void VM::Put() {
        if (this->stack.size() < 2) {
            throw std::exception();
        }

        size_t dst = this->stack.top();
        this->stack.pop();

        MemoryElement value = static_cast<MemoryElement>(this->stack.top());
        this->stack.pop();

        if (dst < 0 || dst >= this->memory.size()) {
            throw std::exception();
        }

        this->memory[dst] = value;
    }

    void VM::Get() {
        if (this->stack.size() < 1) {
            throw std::exception();
        }

        size_t dst = this->stack.top();
        this->stack.pop();

        if (dst < 0 || dst >= this->memory.size()) {
            throw std::exception();
        }

        size_t value = static_cast<size_t>(this->memory[dst]);

        this->stack.push(value);
    }

    void VM::Nop() {
        
    }

    void VM::Stop() {
        if (this->is_stopped) {
            throw std::exception();
        }

        this->is_stopped = true;
    }

    std::vector<MemoryElement> VM::Run(size_t max_iterations) {
        for (size_t i = 0; i < max_iterations; i++) {
            this->Execute();

            if (this->is_stopped) {
                break;
            }

            if (this->stack.size() > this->stack_size) {
                throw std::exception();
            }

            this->code_ptr += 1;

            this->Mutate();
        }

        if (!this->is_stopped) {
            throw std::exception();
        }

        std::vector<MemoryElement> result;

        result.assign(this->memory.begin(), this->memory.end());

        return result;
    }
};
