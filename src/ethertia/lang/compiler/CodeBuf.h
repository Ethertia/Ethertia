//
// Created by Dreamtowards on 2022/6/3.
//

#ifndef ETHERTIA_CODEBUF_H
#define ETHERTIA_CODEBUF_H

#include <map>

#include <ethertia/lang/compiler/Opcodes.h>

typedef u16 t_ip;  // instruction/opcode pos/index
typedef u32 t_ptr;

class CodeBuf
{
public:

    std::vector<SymbolVariable*> localvars;

    std::vector<u8> buf;

    t_ip loop_beg = -1;
    std::vector<t_ip> loop_end_mgoto;


    void defvar(SymbolVariable* sv) {
        assert(ldvar(sv) == -1);

        localvars.push_back(sv);
    }
    int ldvar(SymbolVariable* sv) {
        for (int i = 0; i < localvars.size(); ++i) {
            if (localvars[i]->getSimpleName() == sv->getSimpleName())
                return i;
        }
        return -1;
    }






    void _ldl(SymbolVariable* sv) {
        _ldl(ldvar(sv));
    }
    void _ldl(u8 lidx) {
        cpush8(Opcodes::LDL);
        cpush8(lidx);
    }

    void _mov(u16 size) {
        cpush8(Opcodes::MOV);
        cpush16(size);
    }
    void _mov_pop(u16 size) {
        cpush8(Opcodes::MOV_POP);
        cpush16(size);
    }
    void _mov_push(u16 size) {
        cpush8(Opcodes::MOV_PUSH);
        cpush16(size);
    }

    void _ldc_(u8 type) {
        cpush8(Opcodes::LDC);
        cpush8(type);
        // And Data
    }
    void _ldc_i32(i32 i) {
        _ldc_(Opcodes::LDC_I32);
        cpush32(i);
    }


    void _pop(u16 size) {
        cpush8(Opcodes::POP);
        cpush16(size);
    }


    void _add_i32() {
        cpush8(Opcodes::ADD_I32);
    }

    void _dup(u16 sz) {
        cpush8(Opcodes::DUP);
        cpush16(sz);
    }
    void _dup_ptr() {
        _dup(4);
    }

    [[nodiscard]]
    t_ip _goto() {
        cpush8(Opcodes::GOTO);
        t_ip ip = bufpos();
        cpush16(0);
        return ip;
    }
    void _goto(t_ip ip) {
        cpush8(Opcodes::GOTO);
        cpush16(ip);
    }

    [[nodiscard]]
    t_ip _goto_f() {
        cpush8(Opcodes::GOTO_F);
        t_ip ip = bufpos();
        cpush16(0);
        return ip;
    }


    void _icmp(u8 cond, u8 typ) {
        cpush8(Opcodes::ICMP);
        cpush8(cond);
        cpush8(typ);
    }


    void _verbo(std::string_view s) {
        cpush8(Opcodes::VERBO);
        cpush8(s.length());
        for (char i : s) {
            cpush8(i);
        }
    }

    void _nop() {
        cpush8(Opcodes::NOP);
    }

    // append code
    void cpush8(u8 v) {
        buf.push_back(v);
    }
    void cpush16(u16 v) {
        buf.push_back(v >> 8);
        buf.push_back(v);
    }
    void cpush32(u32 v) {
        buf.push_back(v >> 24);
        buf.push_back(v >> 16);
        buf.push_back(v >> 8);
        buf.push_back(v);
    }
//    void* bufeptr() {
//        return &buf.back();
//    }
    t_ip bufpos() {
        return buf.size();
    }

    u8* bufptr(t_ip ip) {
        return &buf[ip];
    }
};

#endif //ETHERTIA_CODEBUF_H