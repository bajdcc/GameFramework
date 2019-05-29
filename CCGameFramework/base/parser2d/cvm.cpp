//
// Project: CMiniLang
// Author: bajdcc
//

#include "stdafx.h"
#include <cassert>
#include <memory.h>
#include <cstring>
#include <regex>
#include <random>
#include "cvm.h"
#include "cgen.h"
#include "cexception.h"
#include "cgui.h"
#include "cnet.h"
#include "../json/cjparser.h"

#define LOG_INS 0
#define LOG_STACK 0
#define LOG_SYSTEM 1

int g_argc;
char** g_argv;

namespace clib {

#define INC_PTR 4

    cvm::global_state_t cvm::global_state;

    uint32_t cvm::pmm_alloc(bool reusable) {
        auto ptr = (uint32_t)memory.alloc_array<byte>(PAGE_SIZE * 2);
        if (!ptr)
            error("alloc page failed");
        if (reusable)
            ctx->allocation.push_back(ptr);
        auto page = PAGE_ALIGN_UP(ptr);
        memset((void*)page, 0, PAGE_SIZE);
        return page;
    }

    void cvm::vmm_init() {
        pgd_kern = (pde_t*)malloc(PTE_SIZE * sizeof(pde_t));
        memset(pgd_kern, 0, PTE_SIZE * sizeof(pde_t));
        pte_kern = (pte_t*)malloc(PTE_COUNT * PTE_SIZE * sizeof(pte_t));
        memset(pte_kern, 0, PTE_COUNT * PTE_SIZE * sizeof(pte_t));
        pgdir = pgd_kern;

        uint32_t i;

        // map 4G memory, physcial address = virtual address
        for (i = 0; i < PTE_SIZE; i++) {
            pgd_kern[i] = (uint32_t)pte_kern[i] | PTE_P | PTE_R | PTE_K;
        }

        uint32_t* pte = (uint32_t*)pte_kern;
        for (i = 1; i < PTE_COUNT * PTE_SIZE; i++) {
            pte[i] = (i << 12) | PTE_P | PTE_R | PTE_K; // i是页表号
        }

        for (i = 0; i < TASK_NUM; ++i) {
            tasks[i].flag = 0;
            tasks[i].id = i;
            tasks[i].parent = -1;
            tasks[i].state = CTS_DEAD;
        }
        for (i = 0; i < HANDLE_NUM; ++i) {
            handles[i].type = h_none;
        }

        init_fs();
    }

    void cvm::init_fs() {
        fs.as_root(true);
        fs.mkdir("/sys");
        fs.func("/sys/ps", this);
        fs.func("/sys/mem", this);
        fs.mkdir("/proc");
        fs.mkdir("/dev");
        fs.func("/dev/random", this);
        fs.func("/dev/null", this);
        fs.func("/dev/console", this);
        fs.magic("/http", this);
        fs.as_root(false);
        fs.load("/usr/logo.txt");
        fs.load("/usr/badapple.txt");
    }

    // 虚页映射
    // va = 虚拟地址  pa = 物理地址
    void cvm::vmm_map(uint32_t va, uint32_t pa, uint32_t flags) {
        uint32_t pde_idx = PDE_INDEX(va); // 页目录号
        uint32_t pte_idx = PTE_INDEX(va); // 页表号

        pte_t* pte = (pte_t*)(pgdir[pde_idx] & PAGE_MASK); // 页表

        if (!pte) { // 缺页
            if (va >= USER_BASE) { // 若是用户地址则转换
                pte = (pte_t*)pmm_alloc(false); // 申请物理页框，用作新页表
                pgdir[pde_idx] = (uint32_t)pte | PTE_P | flags; // 设置页表
                pte[pte_idx] = (pa & PAGE_MASK) | PTE_P | flags; // 设置页表项
            }
            else { // 内核地址不转换
                pte = (pte_t*)(pgd_kern[pde_idx] & PAGE_MASK); // 取得内核页表
                pgdir[pde_idx] = (uint32_t)pte | PTE_P | flags; // 设置页表
            }
        }
        else { // pte存在
            pte[pte_idx] = (pa & PAGE_MASK) | PTE_P | flags; // 设置页表项
        }

#if 0
        ATLTRACE("MEMMAP> V=%08X P=%08X\n", va, pa);
#endif
    }

    // 释放虚页
    void cvm::vmm_unmap(uint32_t va) {
        uint32_t pde_idx = PDE_INDEX(va);
        uint32_t pte_idx = PTE_INDEX(va);

        pte_t* pte = (pte_t*)(pgdir[pde_idx] & PAGE_MASK);

        if (!pte) {
            return;
        }

#if 0
        ATLTRACE("MEMUNMAP> V=%08X\n", va);
#endif
        pte[pte_idx] = 0; // 清空页表项，此时有效位为零
    }

    // 是否已分页
    int cvm::vmm_ismap(uint32_t va, uint32_t * pa) const {
        uint32_t pde_idx = PDE_INDEX(va);
        uint32_t pte_idx = PTE_INDEX(va);

        pte_t* pte = (pte_t*)(pgdir[pde_idx] & PAGE_MASK);
        if (!pte) {
            return 0; // 页表不存在
        }
        if (pte[pte_idx] != 0 && (pte[pte_idx] & PTE_P) && pa) {
            *pa = pte[pte_idx] & PAGE_MASK; // 计算物理页面
            return 1; // 页面存在
        }
        return 0; // 页表项不存在
    }

    string_t cvm::vmm_getstr(uint32_t va) const {
        std::stringstream ss;
        char c;
        while (c = vmm_get<char>(va++), c) ss << c;
        return ss.str();
    }

    template<class T>
    T cvm::vmm_get(uint32_t va) const {
        if (va == 0)
            error("vmm::get nullptr deref!!");
        if (!(ctx->flag & CTX_KERNEL))
            va |= ctx->mask;
        uint32_t pa;
        if (vmm_ismap(va, &pa)) {
            return *(T*)((byte*)pa + OFFSET_INDEX(va));
        }
        //vmm_map(va, pmm_alloc(), PTE_U | PTE_P | PTE_R);
#if 1
        ATLTRACE("[SYSTEM] MEM  | Invalid VA: %08X\n", va);
#endif
        error("vmm::get error");
        return vmm_get<T>(va);
    }

    template<class T>
    T cvm::vmm_set(uint32_t va, T value) {
        if (!(ctx->flag & CTX_KERNEL)) {
            if ((ctx->flag & CTX_USER_MODE) && (va & 0xF0000000) == USER_BASE) {
                error("code segment cannot be written");
            }
            va |= ctx->mask;
        }
        uint32_t pa;
        if (vmm_ismap(va, &pa)) {
            *(T*)((byte*)pa + OFFSET_INDEX(va)) = value;
            return value;
        }
        //vmm_map(va, pmm_alloc(), PTE_U | PTE_P | PTE_R);
#if 1
        ATLTRACE("[SYSTEM] MEM  | Invalid VA: %08X\n", va);
#endif
        error("vmm::set error");
        return vmm_set(va, value);
    }

    void cvm::vmm_setstr(uint32_t va, const string_t & str) {
        auto len = str.length();
        for (uint32_t i = 0; i < len; i++) {
            vmm_set(va + i, str[i]);
        }
        vmm_set(va + len, '\0');
    }

    uint32_t vmm_pa2va(uint32_t base, uint32_t pa) {
        return base + (pa & (SEGMENT_MASK));
    }

    uint32_t cvm::vmm_malloc(uint32_t size) {
        return vmm_pa2va(ctx->heap, ctx->pool->alloc(size));
    }

    uint32_t cvm::vmm_free(uint32_t addr) {
        if ((addr & 0xF0000000) != HEAP_BASE) {
            return 0;
        }
        return ctx->pool->free(K2U(addr));
    }

    uint32_t cvm::vmm_memset(uint32_t va, uint32_t value, uint32_t count) {
#if 0
        uint32_t pa;
        if (vmm_ismap(va, &pa))
        {
            pa |= OFFSET_INDEX(va);
            ATLTRACE("MEMSET> V=%08X P=%08X S=%08X\n", va, pa, count);
        }
        else
        {
            ATLTRACE("MEMSET> V=%08X P=ERROR S=%08X\n", va, count);
        }
#endif
        for (uint32_t i = 0; i < count; i++) {
#if 0
            ATLTRACE("MEMSET> V=%08X\n", va + i);
#endif
            vmm_set<byte>(va + i, value);
        }
        return 0;
    }

    uint32_t cvm::vmm_memcmp(uint32_t src, uint32_t dst, uint32_t count) {
        for (uint32_t i = 0; i < count; i++) {
#if 0
            ATLTRACE("MEMCMP> '%c':'%c'\n", vmm_get<byte>(src + i), vmm_get<byte>(dst + i));
#endif
            if (vmm_get<byte>(src + i) > vmm_get<byte>(dst + i))
                return 1;
            if (vmm_get<byte>(src + i) < vmm_get<byte>(dst + i))
                return 2;
        }
        return 0;
    }

    template<class T>
    void cvm::vmm_pushstack(uint32_t & sp, T value) {
        sp -= sizeof(T);
        vmm_set<T>(sp, value);
    }

    template<class T>
    T cvm::vmm_popstack(uint32_t & sp) {
        T t = vmm_get<T>(sp);
        sp += sizeof(T);
        return t;
    }

    //-----------------------------------------

    cvm::cvm() {
        vmm_init();
    }

    cvm::~cvm() {
        free(pgd_kern);
        free(pte_kern);
    }

    bool cvm::run(int cycle, int& cycles) {
        for (int i = 0; i < TASK_NUM; ++i) {
            if (tasks[i].flag & CTX_VALID) {
                if (tasks[i].state == CTS_RUNNING) {
                    ctx = &tasks[i];
                    exec(cycle, cycles);
                }
            }
        }
        if (global_state.interrupt) {
            global_state.interrupt = false;
            std::vector<int> foreground_pids;
            for (int i = 1; i < TASK_NUM; ++i) {
                if ((tasks[i].flag & CTX_VALID) && (tasks[i].flag & CTX_FOREGROUND) &&
                    tasks[i].parent != 0)
                    foreground_pids.push_back(i);
            }
#if LOG_SYSTEM
            ATLTRACE("[SYSTEM] SIG  | Received Ctrl-C!\n");
#endif
            for (auto& pid : foreground_pids) {
                destroy(pid);
            }
        }
        ctx = nullptr;
        return available_tasks > 0;
    }

    void cvm::exec(int cycle, int& cycles) {
        if (!ctx)
            error("no process!");
        for (auto i = 0; i < cycle; ++i) {
            i++;
            cycles++;
            if (global_state.interrupt) break;
            if ((ctx->pc & 0xF0000000) != USER_BASE) {
                if (ctx->pc != 0xE0000FF4 && ctx->pc != 0xE0000FFC) {
#if LOG_SYSTEM
                    ATLTRACE("[SYSTEM] ERR  | Invalid PC: %p\n", (void*)ctx->pc);
#endif
                    error("only code segment can execute");
                }
            }
            auto op = vmm_get(ctx->pc); // get next operation code
            ctx->pc += INC_PTR;

#if LOG_INS
            assert(op <= EXIT);
            // print debug info
            if (ctx->debug) {
                CStringA a, b;
                a.Format("%04d> [%08X] %02d %.4s", i, ctx->pc, op, INS_STRING((ins_t)op).c_str());
                if ((op >= PUSH && op <= LNT) || op == LOAD || op == SAVE)
                    b.Format(" %d\n", vmm_get(ctx->pc));
                else if (op == IMX)
                    b.Format(" %08X(%d) %08X(%d)\n", vmm_get(ctx->pc), vmm_get(ctx->pc),
                        vmm_get(ctx->pc + INC_PTR), vmm_get(ctx->pc + INC_PTR));
                else if (op <= ADJ)
                    b.Format(" %08X(%d)\n", vmm_get(ctx->pc), vmm_get(ctx->pc));
                else
                    b.Format("\n");
                ATLTRACE((a + b).GetBuffer(0));
            }
#endif
            switch (op) {
            case NOP:
                break;
            case IMM: {
                ctx->ax._i = vmm_get(ctx->pc);
                ctx->pc += INC_PTR;
            } /* load immediate value to ctx->ax._i */
                      break;
            case IMX: {
                ctx->ax._u._1 = vmm_get(ctx->pc);
                ctx->pc += INC_PTR;
                ctx->ax._u._2 = vmm_get(ctx->pc);
                ctx->pc += INC_PTR;
            } /* load immediate value to ctx->ax._i */
                      break;
            case LOAD: {
                auto n = vmm_get(ctx->pc);
                if (n <= 8) {
                    switch (n) {
                    case 1:
                        ctx->ax._i = vmm_get<byte>((uint32_t)ctx->ax._i);
                        break;
                    case 2:
                    case 3:
                    case 4:
                        ctx->ax._i = vmm_get((uint32_t)ctx->ax._i);
                        break;
                    case 8:
                        ctx->ax._uq = vmm_get<uint64>((uint32_t)ctx->ax._i);
                        break;
                    default:
                        error("load: not supported");
                        break;
                    }
                }
                else if (n <= BIG_DATA_NUM) {
                    auto addr = (uint32_t)ctx->ax._i;
                    for (auto j = 0; j < n / 4; ++j) {
                        *((int*)& ctx->ax.big_data[j * 4]) = vmm_get((addr)+j * 4);
                    }
                    if (n % 4 != 0) {
                        *((int*)& ctx->ax.big_data[n & ~3]) = vmm_get((addr)+(n & ~3));
                        memset(&ctx->ax.big_data[n], 0, (size_t)(4 - (n % 3)));
                    }
                }
                else {
                    error("load: not supported big data");
                }
                ctx->pc += INC_PTR;
            } /* load integer to ctx->ax._i, address in ctx->ax._i */
                       break;
            case SAVE: {
                auto n = vmm_get(ctx->pc);
                if (n <= 8) {
                    switch (n) {
                    case 1:
                        vmm_set<byte>((uint32_t)vmm_popstack(ctx->sp), (byte)ctx->ax._i);
                        break;
                    case 2:
                    case 3:
                    case 4:
                        vmm_set((uint32_t)vmm_popstack(ctx->sp), ctx->ax._i);
                        break;
                    case 8:
                        vmm_set((uint32_t)vmm_popstack(ctx->sp), ctx->ax._uq);
                        break;
                    default:
                        error("save: not supported");
                        break;
                    }
                }
                else if (n <= BIG_DATA_NUM) {
                    auto addr = (uint32_t)vmm_popstack(ctx->sp);
                    for (auto j = 0; j < n / 4; ++j) {
                        vmm_set(addr + j * 4, *((uint32_t*)& ctx->ax.big_data[j * 4]));
                    }
                    if (n % 4 != 0) {
                        memset(&ctx->ax.big_data[n], 0, (size_t)(4 - (n % 3)));
                        vmm_set(addr + (n & ~3), *((uint32_t*)& ctx->ax.big_data[n & ~3]));
                    }
                }
                else {
                    error("save: not supported big data");
                }
                ctx->pc += INC_PTR;
            } /* save integer to address, value in ctx->ax._i, address on stack */
                       break;
            case PUSH: {
                auto n = vmm_get(ctx->pc);
                if (n <= 8) {
                    switch (n) {
                    case 4:
                        vmm_pushstack(ctx->sp, ctx->ax._i);
                        break;
                    case 8:
                        vmm_pushstack(ctx->sp, ctx->ax._u._2);
                        vmm_pushstack(ctx->sp, ctx->ax._u._1);
                        break;
                    default:
                        error("push: not supported");
                        break;
                    }
                }
                else if (n <= BIG_DATA_NUM) {
                    if (n % 4 != 0) {
                        memset(&ctx->ax.big_data[n], 0, (size_t)(4 - (n % 3)));
                        vmm_pushstack(ctx->sp, *((uint32_t*)& ctx->ax.big_data[n & ~3]));
                    }
                    for (auto j = n / 4 - 1; j >= 0; --j) {
                        vmm_pushstack(ctx->sp, *((uint32_t*)& ctx->ax.big_data[j * 4]));
                    }
                }
                else {
                    error("push: not supported big data");
                }
                ctx->pc += INC_PTR;
            } /* push the value of ctx->ax._i onto the stack */
                       break;
            case POP: {
                auto n = vmm_get(ctx->pc);
                if (n <= 8) {
                    switch (n) {
                    case 4:
                        ctx->ax._i = vmm_popstack(ctx->sp);
                        break;
                    case 8:
                        ctx->ax._q = vmm_popstack<int64>(ctx->sp);
                        break;
                    default:
                        error("pop: not supported");
                        break;
                    }
                }
                else if (n <= BIG_DATA_NUM) {
                    for (auto j = 0; j < n / 4; ++j) {
                        *((uint32_t*)& ctx->ax.big_data[j * 4]) = vmm_popstack<uint32_t>(ctx->sp);
                    }
                    if (n % 4 != 0) {
                        *((uint32_t*)& ctx->ax.big_data[n & ~3]) = vmm_popstack<uint32_t>(ctx->sp);
                        memset(&ctx->ax.big_data[n], 0, (size_t)(4 - (n % 3)));
                    }
                }
                else {
                    error("pop: not supported big data");
                }
                ctx->pc += INC_PTR;
            } /* pop the value of ctx->ax._i from the stack */
                      break;
            case JMP: {
                ctx->pc = ctx->base + vmm_get(ctx->pc) * INC_PTR;
            } /* jump to the address */
                      break;
            case JZ: {
                ctx->pc = ctx->ax._i ? ctx->pc + INC_PTR : (ctx->base + vmm_get(ctx->pc) * INC_PTR);
            } /* jump if ctx->ax._i is zero */
                     break;
            case JNZ: {
                ctx->pc = ctx->ax._i ? (ctx->base + vmm_get(ctx->pc) * INC_PTR) : ctx->pc + INC_PTR;
            } /* jump if ctx->ax._i is zero */
                      break;
            case CALL: {
                vmm_pushstack(ctx->sp, ctx->pc);
                ctx->pc = ctx->base + (ctx->ax._ui) * INC_PTR;
            } /* call subroutine */
                /* break;case RET: {pc = (int *)*sp++;} // return from subroutine; */
                       break;
            case ENT: {
                vmm_pushstack(ctx->sp, ctx->bp);
                ctx->bp = ctx->sp;
                ctx->sp = ctx->sp - vmm_get(ctx->pc);
                ctx->pc += INC_PTR;
            } /* make new stack frame */
                      break;
            case ADJ: {
                ctx->sp = ctx->sp + vmm_get(ctx->pc) * INC_PTR;
                ctx->pc += INC_PTR;
            } /* add esp, <size> */
                      break;
            case LEV: {
                ctx->sp = ctx->bp;
                ctx->bp = (uint32_t)vmm_popstack(ctx->sp);
                ctx->pc = (uint32_t)vmm_popstack(ctx->sp);
            } /* restore call frame and PC */
                      break;
            case LEA: {
                ctx->ax._i = ctx->bp + vmm_get(ctx->pc);
                ctx->pc += INC_PTR;
            } /* load address for arguments. */
                      break;
            case CASE:
                if (vmm_get(ctx->sp) == ctx->ax._i) {
                    ctx->sp += INC_PTR;
                    ctx->ax._i = 0; // 0 for same
                }
                else {
                    ctx->ax._i = 1;
                }
                break;
                // OPERATOR
            case OR:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) | ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) | ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._q = vmm_popstack<int64>(ctx->sp) | ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._uq = vmm_popstack<uint64>(ctx->sp) | ctx->ax._uq;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case XOR:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) ^ ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) ^ ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._q = vmm_popstack<int64>(ctx->sp) ^ ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._uq = vmm_popstack<uint64>(ctx->sp) ^ ctx->ax._uq;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case AND:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) & ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) & ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._q = vmm_popstack<int64>(ctx->sp) & ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._uq = vmm_popstack<uint64>(ctx->sp) & ctx->ax._uq;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case EQ:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) == ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                case t_ptr:
                    ctx->ax._i = vmm_popstack<uint>(ctx->sp) == ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._i = vmm_popstack<int64>(ctx->sp) == ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._i = vmm_popstack<uint64>(ctx->sp) == ctx->ax._uq;
                    break;
                case t_float:
                    ctx->ax._i = vmm_popstack<float>(ctx->sp) == ctx->ax._f;
                    break;
                case t_double:
                    ctx->ax._i = vmm_popstack<double>(ctx->sp) == ctx->ax._d;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case NE:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) != ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                case t_ptr:
                    ctx->ax._i = vmm_popstack<uint>(ctx->sp) != ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._i = vmm_popstack<int64>(ctx->sp) != ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._i = vmm_popstack<uint64>(ctx->sp) != ctx->ax._uq;
                    break;
                case t_float:
                    ctx->ax._i = vmm_popstack<float>(ctx->sp) != ctx->ax._f;
                    break;
                case t_double:
                    ctx->ax._i = vmm_popstack<double>(ctx->sp) != ctx->ax._d;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case LT:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) < ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                case t_ptr:
                    ctx->ax._i = vmm_popstack<uint>(ctx->sp) < ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._i = vmm_popstack<int64>(ctx->sp) < ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._i = vmm_popstack<uint64>(ctx->sp) < ctx->ax._uq;
                    break;
                case t_float:
                    ctx->ax._i = vmm_popstack<float>(ctx->sp) < ctx->ax._f;
                    break;
                case t_double:
                    ctx->ax._i = vmm_popstack<double>(ctx->sp) < ctx->ax._d;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case LE:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) <= ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                case t_ptr:
                    ctx->ax._i = vmm_popstack<uint>(ctx->sp) <= ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._i = vmm_popstack<int64>(ctx->sp) <= ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._i = vmm_popstack<uint64>(ctx->sp) <= ctx->ax._uq;
                    break;
                case t_float:
                    ctx->ax._i = vmm_popstack<float>(ctx->sp) <= ctx->ax._f;
                    break;
                case t_double:
                    ctx->ax._i = vmm_popstack<double>(ctx->sp) <= ctx->ax._d;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case GT:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) > ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                case t_ptr:
                    ctx->ax._i = vmm_popstack<uint>(ctx->sp) > ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._i = vmm_popstack<int64>(ctx->sp) > ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._i = vmm_popstack<uint64>(ctx->sp) > ctx->ax._uq;
                    break;
                case t_float:
                    ctx->ax._i = vmm_popstack<float>(ctx->sp) > ctx->ax._f;
                    break;
                case t_double:
                    ctx->ax._i = vmm_popstack<double>(ctx->sp) > ctx->ax._d;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case GE:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) >= ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                case t_ptr:
                    ctx->ax._i = vmm_popstack<uint>(ctx->sp) >= ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._i = vmm_popstack<int64>(ctx->sp) >= ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._i = vmm_popstack<uint64>(ctx->sp) >= ctx->ax._uq;
                    break;
                case t_float:
                    ctx->ax._i = vmm_popstack<float>(ctx->sp) >= ctx->ax._f;
                    break;
                case t_double:
                    ctx->ax._i = vmm_popstack<double>(ctx->sp) >= ctx->ax._d;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case SHL:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) << ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) << ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._q = vmm_popstack<int64>(ctx->sp) << ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._uq = vmm_popstack<uint64>(ctx->sp) << ctx->ax._uq;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case SHR:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) >> ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) >> ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._q = vmm_popstack<int64>(ctx->sp) >> ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._uq = vmm_popstack<uint64>(ctx->sp) >> ctx->ax._uq;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case ADD:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) + ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) + ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._q = vmm_popstack<int64>(ctx->sp) + ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._uq = vmm_popstack<uint64>(ctx->sp) + ctx->ax._uq;
                    break;
                case t_float:
                    ctx->ax._f = vmm_popstack<float>(ctx->sp) + ctx->ax._f;
                    break;
                case t_double:
                    ctx->ax._d = vmm_popstack<double>(ctx->sp) + ctx->ax._d;
                    break;
                case t_ptr:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) + (uint)ctx->ax._i;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case SUB:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) - ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) - ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._q = vmm_popstack<int64>(ctx->sp) - ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._uq = vmm_popstack<uint64>(ctx->sp) - ctx->ax._uq;
                    break;
                case t_float:
                    ctx->ax._f = vmm_popstack<float>(ctx->sp) - ctx->ax._f;
                    break;
                case t_double:
                    ctx->ax._d = vmm_popstack<double>(ctx->sp) - ctx->ax._d;
                    break;
                case t_ptr:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) - (uint)ctx->ax._i;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case MUL:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) * ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) * ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._q = vmm_popstack<int64>(ctx->sp) * ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._uq = vmm_popstack<uint64>(ctx->sp) * ctx->ax._uq;
                    break;
                case t_float:
                    ctx->ax._f = vmm_popstack<float>(ctx->sp) * ctx->ax._f;
                    break;
                case t_double:
                    ctx->ax._d = vmm_popstack<double>(ctx->sp) * ctx->ax._d;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case DIV:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    if (ctx->ax._i == 0)
                        error("divide zero exception");
                    ctx->ax._i = vmm_popstack(ctx->sp) / ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    if (ctx->ax._ui == 0)
                        error("divide zero exception");
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) / ctx->ax._ui;
                    break;
                case t_long:
                    if (ctx->ax._q == 0)
                        error("divide zero exception");
                    ctx->ax._q = vmm_popstack<int64>(ctx->sp) / ctx->ax._q;
                    break;
                case t_ulong:
                    if (ctx->ax._uq == 0)
                        error("divide zero exception");
                    ctx->ax._uq = vmm_popstack<uint64>(ctx->sp) / ctx->ax._uq;
                    break;
                case t_float:
                    if (ctx->ax._f == 0)
                        error("divide zero exception");
                    ctx->ax._f = vmm_popstack<float>(ctx->sp) / ctx->ax._f;
                    break;
                case t_double:
                    if (ctx->ax._d == 0)
                        error("divide zero exception");
                    ctx->ax._d = vmm_popstack<double>(ctx->sp) / ctx->ax._d;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case MOD:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = vmm_popstack(ctx->sp) % ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._ui = vmm_popstack<uint>(ctx->sp) % ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._q = vmm_popstack<int64>(ctx->sp) % ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._uq = vmm_popstack<uint64>(ctx->sp) % ctx->ax._uq;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case NEG:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = -ctx->ax._i;
                    break;
                case t_long:
                    ctx->ax._q = -ctx->ax._q;
                    break;
                case t_float:
                    ctx->ax._f = -ctx->ax._f;
                    break;
                case t_double:
                    ctx->ax._d = -ctx->ax._d;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case NOT:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = ~ctx->ax._i;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._ui = ~ctx->ax._ui;
                    break;
                case t_long:
                    ctx->ax._q = ~ctx->ax._q;
                    break;
                case t_ulong:
                    ctx->ax._uq = ~ctx->ax._uq;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case LNT:
                switch ((cast_t)vmm_get(ctx->pc)) {
                case t_char:
                case t_short:
                case t_int:
                    ctx->ax._i = ctx->ax._i ? 0 : 1;
                    break;
                case t_uchar:
                case t_ushort:
                case t_uint:
                    ctx->ax._i = ctx->ax._ui ? 0 : 1;
                    break;
                case t_long:
                    ctx->ax._i = ctx->ax._q ? 0 : 1;
                    break;
                case t_ulong:
                    ctx->ax._i = ctx->ax._uq ? 0 : 1;
                    break;
                case t_float:
                    ctx->ax._i = ctx->ax._f == 0.0f ? 0 : 1;
                    break;
                case t_double:
                    ctx->ax._i = ctx->ax._d == 0 ? 0 : 1;
                    break;
                case t_ptr:
                    ctx->ax._i = ctx->ax._ui ? 0 : 1;
                    break;
                default:
                    error("unsupport operator: " + INS_STRING((ins_t)op));
                    break;
                }
                ctx->pc += INC_PTR;
                break;
            case EXIT: {
#if LOG_SYSTEM
                ATLTRACE("[SYSTEM] PROC | Exit: PID= #%d, CODE= %d\n", ctx->id, ctx->ax._i);
#endif
                destroy(ctx->id);
                return;
            }
            case INTR: // 中断调用，以寄存器ax传参
                if (interrupt())
                    return;
                break;
            case CAST: // 类型转换，以寄存器ax传参
                cast();
                break;
            default: {
#if LOG_SYSTEM
                ATLTRACE("[SYSTEM] ERR  | AX: %08X BP: %08X SP: %08X PC: %08X\n", ctx->ax._i, ctx->bp, ctx->sp, ctx->pc);
                for (uint32_t j = ctx->sp; j < STACK_BASE + PAGE_SIZE; j += 4) {
                    ATLTRACE("[SYSTEM] ERR  | [%08X]> %08X\n", j, vmm_get<uint32_t>(j));
                }
                ATLTRACE("[SYSTEM] ERR  | unknown instruction: %d\n", op);
#endif
                error("unknown instruction");
            }
            }

#if LOG_STACK
            if (ctx->debug) {
                ATLTRACE("\n---------------- STACK BEGIN <<<< \n");
                ATLTRACE("AX: %08X BX: %08X BP: %08X SP: %08X PC: %08X\n", ctx->ax._u._1, ctx->ax._u._2, ctx->bp, ctx->sp, ctx->pc);
                auto k = 0;
                CStringA a;
                CStringA b;
                for (uint32_t j = ctx->sp; j < STACK_BASE + PAGE_SIZE; j += 4, ++k) {
                    b.Format("[%08X]> %08X", j, vmm_get<uint32_t>(j)); a += b;
                    if (k % 4 == 3) {
                        a += "\n";
                        ATLTRACE(a.GetBuffer(0));
                        a = "";
                    }
                    else
                        a += "  |  ";
                }
                if (k % 4 != 0)
                    ATLTRACE("\n");
                ATLTRACE("---------------- STACK END >>>>\n\n");
            }
#endif
        }
    }

    void cvm::error(const string_t & str) const {
        throw cexception(ex_vm, str);
    }

    int cvm::load(const string_t & path, const std::vector<byte> & file, const std::vector<string_t> & args) {
        auto old_ctx = ctx;
        new_pid();
        ctx->file = file;
#if LOG_SYSTEM
        ATLTRACE("[SYSTEM] PROC | Create: PID= #%d\n", ctx->id);
#endif
        PE* pe = (PE*)file.data();
        // TODO: VALID PE FILE
        uint32_t pa;
        ctx->poolsize = PAGE_SIZE;
        ctx->mask = U2K(ctx->id);
        ctx->entry = pe->entry;
        ctx->stack = STACK_BASE | ctx->mask;
        ctx->data = DATA_BASE | ctx->mask;
        ctx->base = USER_BASE | ctx->mask;
        ctx->heap = HEAP_BASE | ctx->mask;
        ctx->pool = std::make_unique<cmem>(this);
        ctx->flag |= CTX_KERNEL;
        ctx->state = CTS_RUNNING;
        ctx->path = path;
        /* 映射4KB的代码空间 */
        {
            auto size = PAGE_SIZE / sizeof(int);
            auto text_size = pe->text_len / sizeof(int);
            auto text_start = (uint32_t*)(&pe->data + pe->data_len);
            for (uint32_t i = 0, start = 0; start < text_size; ++i, start += size) {
                auto new_page = (uint32_t)pmm_alloc();
                ctx->text_mem.push_back(new_page);
                vmm_map(ctx->base + PAGE_SIZE * i, new_page, PTE_U | PTE_P); // 用户代码空间
                if (vmm_ismap(ctx->base + PAGE_SIZE * i, &pa)) {
                    auto s = start + size > text_size ? (text_size & (size - 1)) : size;
                    for (uint32_t j = 0; j < s; ++j) {
                        *((uint32_t*)pa + j) = (uint)text_start[start + j];
#if 0
                        ATLTRACE("[%p]> [%08X] %08X\n", (void*)((int*)pa + j), ctx->base + PAGE_SIZE * i + j * 4, vmm_get<uint32_t>(ctx->base + PAGE_SIZE * i + j * 4));
#endif
                    }
                }
            }
        }
        /* 映射4KB的数据空间 */
        {
            auto size = PAGE_SIZE;
            auto data_size = pe->data_len;
            auto data_start = (char*)& pe->data;
            for (uint32_t i = 0, start = 0; start < data_size; ++i, start += size) {
                auto new_page = (uint32_t)pmm_alloc();
                ctx->data_mem.push_back(new_page);
                vmm_map(ctx->data + PAGE_SIZE * i, new_page, PTE_U | PTE_P | PTE_R); // 用户数据空间
                if (vmm_ismap(ctx->data + PAGE_SIZE * i, &pa)) {
                    auto s = start + size > data_size ? ((sint)data_size & (size - 1)) : size;
                    for (auto j = 0; j < s; ++j) {
                        *((char*)pa + j) = data_start[start + j];
#if 0
                        ATLTRACE("[%p]> [%08X] %d\n", (void*)((char*)pa + j), ctx->data + PAGE_SIZE * i + j, vmm_get<byte>(ctx->data + PAGE_SIZE * i + j));
#endif
                    }
                }
            }
        }
        /* 映射4KB的栈空间 */
        {
            auto new_page = (uint32_t)pmm_alloc();
            ctx->stack_mem.push_back(new_page);
            vmm_map(ctx->stack, new_page, PTE_U | PTE_P | PTE_R); // 用户栈空间
        }
        ctx->flag &= ~CTX_KERNEL;
        {
            ctx->stack = STACK_BASE;
            ctx->data = DATA_BASE;
            ctx->base = USER_BASE;
            ctx->heap = HEAP_BASE;
            ctx->sp = ctx->stack + ctx->poolsize; // 4KB / sizeof(int) = 1024
            ctx->pc = ctx->base | (ctx->entry * INC_PTR);
            ctx->ax._i = 0;
            ctx->bp = 0;

            auto _argc = args.size();
            vmm_pushstack(ctx->sp, EXIT);
            vmm_pushstack(ctx->sp, 4);
            vmm_pushstack(ctx->sp, PUSH);
            auto tmp = ctx->sp;
            auto argvs = vmm_malloc(_argc * INC_PTR);
            vmm_pushstack(ctx->sp, _argc);
            vmm_pushstack(ctx->sp, argvs);
            for (size_t i = 0; i < _argc; i++) {
                auto str = vmm_malloc(args[i].length() + 1);
                vmm_setstr(str, args[i]);
                vmm_set(argvs + INC_PTR * i, str);
            }
            vmm_pushstack(ctx->sp, tmp);
        }
        ctx->flag |= CTX_USER_MODE | CTX_FOREGROUND;
        ctx->debug = false;
        ctx->waiting_ms = 0;
        ctx->input_redirect = -1;
        ctx->output_redirect = -1;
        ctx->input_queue.clear();
        ctx->input_stop = false;
        available_tasks++;
        auto pid = ctx->id;
        ctx = old_ctx;
        return pid;
    }

    void cvm::destroy(int id) {
        auto old_ctx = ctx;
        ctx = &tasks[id];
        {
            if (global_state.input_lock == ctx->id) {
                global_state.input_lock = -1;
                global_state.input_read_ptr = -1;
                global_state.input_content.clear();
                global_state.input_success = false;
                cgui::singleton().reset_cmd();
            }
            if (set_cycle_id == ctx->id) {
                cgui::singleton().set_cycle(0);
                set_cycle_id = -1;
            }
            if (set_resize_id == ctx->id) {
                cgui::singleton().resize(0, 0);
                set_resize_id = -1;
            }
            if (ctx->output_redirect != -1 && tasks[ctx->output_redirect].flag & CTX_VALID) {
                if (!ctx->input_queue.empty()) {
                    std::copy(ctx->input_queue.begin(), ctx->input_queue.end(),
                        std::back_inserter(tasks[ctx->output_redirect].input_queue));
                    ctx->input_redirect = -1;
                }
                tasks[ctx->output_redirect].input_stop = true;
                ctx->output_redirect = -1;
            }
        }
        if (!ctx->child.empty()) {
            ctx->state = CTS_ZOMBIE;
            ctx = old_ctx;
            return;
        }
#if LOG_SYSTEM
        ATLTRACE("[SYSTEM] PROC | Destroy: PID= #%d, Code= %d\n", ctx->id, ctx->ax._i);
#endif
        ctx->flag = 0;
        {
            PE* pe = (PE*)ctx->file.data();
            ctx->poolsize = PAGE_SIZE;
            ctx->mask = U2K(ctx->id);
            ctx->entry = pe->entry;
            ctx->stack = STACK_BASE | ctx->mask;
            ctx->data = DATA_BASE | ctx->mask;
            ctx->base = USER_BASE | ctx->mask;
            ctx->heap = HEAP_BASE | ctx->mask;
            ctx->flag |= CTX_KERNEL;
            /* 映射4KB的代码空间 */
            {
                auto size = PAGE_SIZE / sizeof(int);
                auto text_size = pe->text_len / sizeof(int);
                for (uint32_t i = 0, start = 0; start < text_size; ++i, start += size) {
                    vmm_unmap(ctx->base + PAGE_SIZE * i); // 用户代码空间
                }
            }
            /* 映射4KB的数据空间 */
            {
                auto size = PAGE_SIZE;
                auto data_size = pe->data_len;
                for (uint32_t i = 0, start = 0; start < data_size; ++i, start += size) {
                    vmm_unmap(ctx->data + PAGE_SIZE * i); // 用户数据空间
                }
            }
            /* 映射4KB的栈空间 */
            vmm_unmap(ctx->stack); // 用户栈空间
            /* 映射16KB的堆空间 */
            {
                for (int i = 0; i < ctx->pool->page_size(); ++i) {
                    vmm_unmap(ctx->heap + PAGE_SIZE * i);
                }
            }
            {
                for (auto& a : ctx->allocation) {
                    memory.free_array((byte*)a);
                }
            }
            ctx->child.clear();
            ctx->state = CTS_DEAD;
            ctx->file.clear();
            ctx->allocation.clear();
            ctx->pool.reset();
            ctx->flag = 0;
            auto handles = ctx->handles;
            for (auto& h : handles) {
                destroy_handle(h);
            }
            ctx->handles.clear();
            if (ctx->parent != -1) {
                auto& parent = tasks[ctx->parent];
                parent.child.erase(ctx->id);
                if (parent.state == CTS_ZOMBIE)
                    destroy(ctx->parent);
                else if (parent.state == CTS_WAIT)
                    parent.state = CTS_RUNNING;
                ctx->parent = -1;
            }
            ctx->data_mem.clear();
            ctx->text_mem.clear();
            ctx->stack_mem.clear();
            ctx->input_queue.clear();
            {
                std::stringstream ss;
                ss << "/proc/" << ctx->id;
                fs.rm(ss.str());
            }
        }
        ctx = old_ctx;
        available_tasks--;
    }

    string_t get_args(const string_t & path, std::vector<string_t> & args) {
        std::stringstream ss(path);
        string_t temp;
        while (std::getline(ss, temp, ' ')) {
            args.push_back(temp);
        }
        return args.front();
    }

    string_t trim(string_t str) {
        string_t::size_type pos = str.find_last_not_of(' ');
        if (pos != string_t::npos) {
            str.erase(pos + 1);
            pos = str.find_first_not_of(' ');
            if (pos != string_t::npos) str.erase(0, pos);
        }
        else str.erase(str.begin(), str.end());
        return str;
    }

    int cvm::exec_file(const string_t & path) {
        if (path.empty())
            return -1;
        auto new_path = trim(path);
#if LOG_SYSTEM
        ATLTRACE("[SYSTEM] PROC | Exec: Command= %s\n", new_path.data());
#endif
        std::vector<string_t> args;
        auto file = get_args(new_path, args);
        auto pid = cgui::singleton().compile(file, args);
        if (pid >= 0) { // SUCCESS
            ctx->child.insert(pid);
            tasks[pid].parent = ctx->id;
#if LOG_SYSTEM
            ATLTRACE("[SYSTEM] PROC | Exec: Parent= #%d, Child= #%d\n", ctx->id, pid);
#endif
        }
        return pid;
    }

    int cvm::fork() {
        auto old_ctx = ctx;
        new_pid();
        ctx->file = old_ctx->file;
#if LOG_SYSTEM
        ATLTRACE("[SYSTEM] PROC | Fork: Parent= #%d, Child= #%d\n", old_ctx->id, ctx->id);
#endif
        PE* pe = (PE*)ctx->file.data();
        // TODO: VALID PE FILE
        uint32_t pa;
        ctx->poolsize = PAGE_SIZE;
        ctx->mask = ((uint)(ctx->id << 16) & 0x00ff0000);
        ctx->entry = old_ctx->entry;
        ctx->stack = old_ctx->stack | ctx->mask;
        ctx->data = old_ctx->data | ctx->mask;
        ctx->base = old_ctx->base | ctx->mask;
        ctx->heap = old_ctx->heap | ctx->mask;
        ctx->pool = std::make_unique<cmem>(this);
        ctx->flag |= CTX_KERNEL;
        ctx->state = CTS_RUNNING;
        ctx->path = old_ctx->path;
        old_ctx->child.insert(ctx->id);
        ctx->parent = old_ctx->id;
        /* 映射4KB的代码空间 */
        {
            auto size = PAGE_SIZE / sizeof(int);
            auto text_size = pe->text_len / sizeof(int);
            for (uint32_t i = 0, start = 0; start < text_size; ++i, start += size) {
                auto new_page = (uint32_t)pmm_alloc();
                std::copy((byte*)(old_ctx->text_mem[i]),
                    (byte*)(old_ctx->text_mem[i]) + PAGE_SIZE,
                    (byte*)new_page);
                ctx->text_mem.push_back(new_page);
                vmm_map(ctx->base + PAGE_SIZE * i, new_page, PTE_U | PTE_P | PTE_R); // 用户代码空间
                if (!vmm_ismap(ctx->base + PAGE_SIZE * i, &pa)) {
                    destroy(ctx->id);
                    error("fork: text segment copy failed");
                }
            }
        }
        /* 映射4KB的数据空间 */
        {
            auto size = PAGE_SIZE;
            auto data_size = pe->data_len;
            for (uint32_t i = 0, start = 0; start < data_size; ++i, start += size) {
                auto new_page = (uint32_t)pmm_alloc();
                ctx->data_mem.push_back(new_page);
                std::copy((byte*)(old_ctx->data_mem[i]),
                    (byte*)(old_ctx->data_mem[i]) + PAGE_SIZE,
                    (byte*)new_page);
                vmm_map(ctx->data + PAGE_SIZE * i, new_page, PTE_U | PTE_P | PTE_R); // 用户数据空间
                if (!vmm_ismap(ctx->data + PAGE_SIZE * i, &pa)) {
                    destroy(ctx->id);
                    error("fork: data segment copy failed");
                }
            }
        }
        /* 映射4KB的栈空间 */
        {
            auto new_page = (uint32_t)pmm_alloc();
            ctx->stack_mem.push_back(new_page);
            std::copy((byte*)(old_ctx->stack_mem[0]),
                (byte*)(old_ctx->stack_mem[0]) + PAGE_SIZE,
                (byte*)new_page);
            vmm_map(ctx->stack, new_page, PTE_U | PTE_P | PTE_R); // 用户栈空间
            if (!vmm_ismap(ctx->stack, &pa)) {
                destroy(ctx->id);
                error("fork: stack segment copy failed");
            }
        }
        /* 映射堆空间 */
        ctx->pool->copy_from(*old_ctx->pool);
        ctx->flag = old_ctx->flag;
        ctx->sp = old_ctx->sp;
        ctx->stack = old_ctx->stack;
        ctx->data = old_ctx->data;
        ctx->base = old_ctx->base;
        ctx->heap = old_ctx->heap;
        ctx->pc = old_ctx->pc;
        ctx->ax._i = -1;
        ctx->bp = old_ctx->bp;
        ctx->debug = old_ctx->debug;
        ctx->waiting_ms = 0;
        ctx->input_redirect = old_ctx->input_redirect;
        ctx->output_redirect = old_ctx->output_redirect;
        ctx->input_stop = old_ctx->input_stop;
        ctx->handles = old_ctx->handles;
        available_tasks++;
        auto pid = ctx->id;
        ctx = old_ctx;
        return pid;
    }

    void cvm::map_page(uint32_t addr, uint32_t id) {
        uint32_t pa;
        auto va = (ctx->heap | ctx->mask) | (PAGE_SIZE * id);
        vmm_map(va, addr, PTE_U | PTE_P | PTE_R);
#if LOG_SYSTEM
        ATLTRACE("[SYSTEM] MEM  | Map: PA= %p, VA= %p\n", (void*)addr, (void*)va);
#endif
        if (!vmm_ismap(va, &pa)) {
            destroy(ctx->id);
            error("heap alloc: alloc page failed");
        }
    }

    void cvm::as_root(bool flag) {
        fs.as_root(flag);
    }

    bool cvm::read_vfs(const string_t & path, std::vector<byte> & data) const {
        return fs.read_vfs(path, data);
    }

    bool cvm::write_vfs(const string_t & path, const std::vector<byte> & data) {
        return fs.write_vfs(path, data);
    }

    vfs_stream_t cvm::stream_type(const string_t & path) const {
        if (path.substr(0, 4) == "/dev") {
            static string_t pat{ R"(/dev/([a-z_]+))" };
            static std::regex re(pat);
            std::smatch res;
            if (std::regex_match(path, res, re)) {
                auto op = res[1].str();
                if (op == "random") {
                    return fss_random;
                }
                if (op == "null") {
                    return fss_null;
                }
                if (op == "console") {
                    return fss_console;
                }
            }
        }
        else if (path.substr(0, 5) == "/http") {
            return fss_net;
        }
        return fss_none;
    }

    string_t limit_string(const string_t & s, uint len) {
        if (s.length() <= len) {
            return s;
        }
        else {
            return s.substr(0, len);
        }
    }

    string_t cvm::stream_callback(const string_t & path) {
        static char sz[256];
        if (path.substr(0, 5) == "/proc") {
            static string_t pat{ R"(/proc/(\d+)/([a-z_]+))" };
            static std::regex re(pat);
            std::smatch res;
            if (std::regex_match(path, res, re)) {
                auto id = std::stoi(res[1].str());
                if (!(tasks[id].flag & CTX_VALID)) {
                    return "\033FFF0000F0\033[ERROR] Invalid pid\033S4\033";
                }
                const auto& op = res[2].str();
                if (op == "exe") {
                    return tasks[id].path;
                }
                else if (op == "parent") {
                    sprintf(sz, "%d", tasks[id].parent);
                    return sz;
                }
                else if (op == "heap_size") {
                    sprintf(sz, "%d", tasks[id].pool->page_size());
                    return sz;
                }
            }
        }
        else if (path.substr(0, 4) == "/sys") {
            static string_t pat{ R"(/sys/([a-z_]+))" };
            static std::regex re(pat);
            std::smatch res;
            if (std::regex_match(path, res, re)) {
                const auto& op = res[1].str();
                if (op == "ps") {
                    std::stringstream ss;
                    ss << "\033FFFA0A0A0\033[STATE] \033S4\033[PID] [PPID]\033FFFB3B920\033 [COMMAND LINE] \033FFF51C2A8\033[PAGE]\033S4\033" << std::endl;
                    for (auto i = 0; i < TASK_NUM; ++i) {
                        if (tasks[i].flag & CTX_VALID) {
                            sprintf(sz, "\033FFFA0A0A0\033%7s \033S4\033 %4d   %4d \033FFFB3B920\033%-14s \033FFF51C2A8\033  %4d\033S4\033",
                                state_string(tasks[i].state),
                                i,
                                tasks[i].parent,
                                limit_string(tasks[i].path, 14).c_str(),
                                ctx->allocation.size());
                            ss << sz << std::endl;
                        }
                    }
                    return ss.str();
                }
                else if (op == "mem") {
                    std::stringstream ss;
                    sprintf(sz, "%-18s %d", "Memory Total:", memory.DEFAULT_ALLOC_MEMORY_SIZE); ss << sz << std::endl;
                    sprintf(sz, "%-18s %d", "Memory Using:", (memory.DEFAULT_ALLOC_BLOCK_SIZE - memory.available()) * memory.BLOCK_SIZE); ss << sz << std::endl;
                    sprintf(sz, "%-18s %d", "Memory Free:", memory.available() * memory.BLOCK_SIZE); ss << sz << std::endl;
                    int pages = 0, heaps = 0, heaps_a = 0;
                    for (auto i = 0; i < TASK_NUM; ++i) {
                        if (tasks[i].flag & CTX_VALID) {
                            pages += tasks[i].allocation.size();
                            heaps += tasks[i].pool->page_size();
                            heaps_a += tasks[i].pool->available();
                        }
                    }
                    sprintf(sz, "%-18s %d", "Heap Total:", heaps* PAGE_SIZE); ss << sz << std::endl;
                    sprintf(sz, "%-18s %d", "Heap Using:", heaps * PAGE_SIZE - heaps_a); ss << sz << std::endl;
                    sprintf(sz, "%-18s %d", "Heap Free:", heaps_a); ss << sz << std::endl;
                    sprintf(sz, "%-18s %d", "Page Using:", pages); ss << sz << std::endl;
                    return ss.str();
                }
            }
        }
        else if (path.substr(0, 5) == "/http") {
            return "346";
        }
        return "\033FFF0000F0\033[ERROR] File not exists.\033S4\033";
    }

    vfs_node_dec* cvm::stream_create(const vfs_mod_query * mod, vfs_stream_t type, const string_t & path) {
        if (type == fss_net) {
            return new vfs_node_stream_net(mod, type, this, path);
        }
        if (type == fss_console) {
            return new vfs_node_stream_write(mod, type, this);
        }
        if (type != fss_none) {
            return new vfs_node_stream(mod, type, this);
        }
        error("invalid vfs stream");
        return nullptr;
    }

    int cvm::stream_index(vfs_stream_t type) {
        switch (type) {
        case fss_random: {
            static std::default_random_engine e(((uint32_t)time(nullptr)) % (UINT32_MAX));
            static std::uniform_int_distribution<int> u(0, 255);
            return u(e);
        }
        case fss_null: {
            return 0;
        }
        default:
            break;
        }
        error("invalid stream type");
        return -1;
    }

    string_t cvm::stream_net(vfs_stream_t type, const string_t & path) {
        switch (type) {
        case fss_net: {
            return net.http_get(path);
        }
        default:
            break;
        }
        error("invalid stream type");
        return "";
    }

    int cvm::stream_write(vfs_stream_t type, byte c)
    {
        if (global_state.input_lock == -1) {
            cgui::singleton().put_char((char) c);
            return 0;
        }
        return -1;
    }

    const char* cvm::state_string(cvm::ctx_state_t type) {
        assert(type >= CTS_RUNNING && type < CTS_DEAD);
        switch (type) {
        case CTS_RUNNING:
            return "RUNNING";
        case CTS_WAIT:
            return "WAITING";
        case CTS_ZOMBIE:
            return "ZOMBIE";
        case CTS_DEAD:
            return "DEAD";
        default:
            return "[ERROR] Invalid state string";
        }
    }

    int cvm::new_pid() {
        if (available_tasks >= TASK_NUM) {
            error("max process num!");
        }
        auto end = TASK_NUM + pids;
        for (int i = pids; i < end; ++i) {
            auto j = i % TASK_NUM;
            if (!(tasks[j].flag & CTX_VALID)) {
                tasks[j].flag |= CTX_VALID;
                ctx = &tasks[j];
                pids = (j + 1) % TASK_NUM;
                ctx->id = j;
                {
                    std::stringstream ss;
                    ss << "/proc/" << j;
                    auto dir = ss.str();
                    fs.as_root(true);
                    if (fs.mkdir(dir) == 0) { // '/proc/[pid]'
                        static std::vector<string_t> ps =
                        { "exe", "parent", "heap_size" };
                        dir += "/";
                        for (auto& _ps : ps) {
                            ss.str("");
                            ss << dir << _ps;
                            fs.func(ss.str(), this);
                        }
                    }
                    fs.as_root(false);
                }
                return j;
            }
        }
        error("max process num!");
        return -1;
    }

    int cvm::new_handle(cvm::handle_type type) {
        if (available_handles >= HANDLE_NUM)
            error("max handle num!");
        auto end = HANDLE_NUM + handle_ids;
        for (int i = handle_ids; i < end; ++i) {
            auto j = i % HANDLE_NUM;
            if (handles[j].type == h_none) {
                handles[j].type = type;
                handle_ids = (j + 1) % HANDLE_NUM;
                available_handles++;
                ctx->handles.insert(j);
                return j;
            }
        }
        error("max handle num!");
        return -1;
    }

    void cvm::destroy_handle(int handle) {
        if (handle < 0 || handle >= HANDLE_NUM)
            error("invalid handle");
        if (handles[handle].type != h_none) {
            auto h = &handles[handle];
            if (h->type == h_file) {
                delete h->data.file;
            }
            h->type = h_none;
            ctx->handles.erase(handle);
            available_handles--;
        }
        else {
            error("destroy handle failed!");
        }
    }

    char* cvm::output_fmt(int id) const {
        static char str[256];
        switch (id) {
        case 1:
            sprintf(str, "%d", ctx->ax._i);
            break;
        case 2:
            sprintf(str, "%p", ctx->ax._p);
            break;
        case 4:
            sprintf(str, "%f", ctx->ax._f);
            break;
        case 6:
            sprintf(str, "%f", ctx->ax._d);
            break;
        case 7:
            sprintf(str, "%lld", ctx->ax._q);
            break;
        case 9:
            sprintf(str, "%llu", ctx->ax._uq);
            break;
        default:
            sprintf(str, "[Invalid format]");
            break;
        }
        return str;
    }

    int cvm::output(int id) {
        if (ctx->output_redirect != -1) {
            if (id == 0) {
                tasks[ctx->output_redirect].input_queue.push_back(ctx->ax._c);
            }
            else {
                auto s = output_fmt(id);
                while (*s) tasks[ctx->output_redirect].input_queue.push_back(*s++);
            }
        }
        else if (global_state.input_lock == -1) {
            if (id == 0) {
                cgui::singleton().put_char(ctx->ax._c);
            }
            else {
                auto s = output_fmt(id);
                while (*s) cgui::singleton().put_char(*s++);
            }
        }
        else {
            if (global_state.input_lock != ctx->id)
                global_state.input_waiting_list.push_back(ctx->id);
            ctx->state = CTS_WAIT;
            ctx->pc -= INC_PTR;
            return 1;
        }
        return 0;
    }

    void cvm::cast() {
        switch (vmm_get(ctx->pc)) {
        case 1:
            ctx->ax._ui = (uint)ctx->ax._i;
            break;
        case 2:
            ctx->ax._i = (int)ctx->ax._ui;
            break;
        case 3:
            ctx->ax._uq = (uint64)ctx->ax._i;
            break;
        case 4:
            ctx->ax._q = (int64)ctx->ax._ui;
            break;
        case 5:
            ctx->ax._ui = (uint)ctx->ax._q;
            break;
        case 6:
            ctx->ax._i = (int)ctx->ax._uq;
            break;
        case 7:
            ctx->ax._uq = (uint64)ctx->ax._q;
            break;
        case 8:
            ctx->ax._q = (int64)ctx->ax._uq;
            break;
        case 9:
            ctx->ax._q = (int64)ctx->ax._i;
            break;
        case 10:
            ctx->ax._uq = (uint64)ctx->ax._ui;
            break;
        case 11:
            ctx->ax._i = (int)ctx->ax._q;
            break;
        case 12:
            ctx->ax._ui = (uint)ctx->ax._uq;
            break;
        case 13:
            ctx->ax._q = (int64)ctx->ax._c;
            break;
        case 14:
            ctx->ax._q = (int64)ctx->ax._s;
            break;
        case 20:
            ctx->ax._f = (float)ctx->ax._ui;
            break;
        case 21:
            ctx->ax._f = (float)ctx->ax._i;
            break;
        case 22:
            ctx->ax._f = (float)ctx->ax._uq;
            break;
        case 23:
            ctx->ax._f = (float)ctx->ax._q;
            break;
        case 24:
            ctx->ax._ui = (uint)ctx->ax._f;
            break;
        case 25:
            ctx->ax._i = (int)ctx->ax._f;
            break;
        case 26:
            ctx->ax._uq = (uint64)ctx->ax._f;
            break;
        case 27:
            ctx->ax._q = (int64)ctx->ax._f;
            break;
        case 28:
            ctx->ax._d = (double)ctx->ax._f;
            break;
        case 30:
            ctx->ax._d = (double)ctx->ax._ui;
            break;
        case 31:
            ctx->ax._d = (double)ctx->ax._i;
            break;
        case 32:
            ctx->ax._d = (double)ctx->ax._uq;
            break;
        case 33:
            ctx->ax._d = (double)ctx->ax._q;
            break;
        case 34:
            ctx->ax._ui = (uint)ctx->ax._d;
            break;
        case 35:
            ctx->ax._i = (int)ctx->ax._d;
            break;
        case 36:
            ctx->ax._uq = (uint64)ctx->ax._d;
            break;
        case 37:
            ctx->ax._q = (int64)ctx->ax._d;
            break;
        case 38:
            ctx->ax._f = (float)ctx->ax._d;
            break;
        default:
            error("unsupported cast");
            break;
        }
        ctx->pc += INC_PTR;
    }

    bool cvm::math(int id) {
        switch (id) {
        case 201:
            ctx->ax._d = std::sqrt(std::abs(ctx->ax._d));
            break;
        default:
#if LOG_SYSTEM
            ATLTRACE("[SYSTEM] ERR  | unknown interrupt: %d\n", ctx->ax._i);
#endif
            error("unknown interrupt");
            break;
        }
        ctx->pc += INC_PTR;
        return false;
    }

    bool cvm::gui(int id) {
        switch (id) {
        case 301:
        {
            if (ctx->ax._i == 1 || ctx->ax._i == 0)
            {
                global_state.gui = ctx->ax._i != 0;
                if (ctx->ax._i == 1) {
                    if (!global_state.ui->ready()) {
                        ctx->pc -= INC_PTR;
                        return true;
                    }
                }
                else {
                    if (global_state.ui->ready()) {
                        ctx->pc -= INC_PTR;
                        return true;
                    }
                }
            }
            else if (ctx->ax._i == 2 || ctx->ax._i == 3)
            {
                if (ctx->ax._i == 2)
                    ctx->ax._i = global_state.ui->get_width();
                else
                    ctx->ax._i = global_state.ui->get_height();
            }
            else if (ctx->ax._i >= 4 && ctx->ax._i <= 7)
            {
                if (ctx->ax._i == 7) {
                    if (global_state.ui->set_fresh(-1) != 0) {
                        ctx->pc -= INC_PTR;
                        return true;
                    }
                }
                else {
                    global_state.ui->set_fresh(ctx->ax._i - 4);
                }
            }
            else if (ctx->ax._i == 8)
            {
                global_state.ui->reset();
            }
        }
            break;
        case 302:
        {
            auto x = ctx->ax._ui >> 16;
            auto y = ctx->ax._ui & 0xFFFF;
            global_state.ui->move_to(x, y);
        }
            break;
        case 303:
        {
            auto x = ctx->ax._ui >> 16;
            auto y = ctx->ax._ui & 0xFFFF;
            global_state.ui->line_to(x, y);
        }
            break;
        case 304:
        {
            auto x = ctx->ax._ui >> 16;
            auto y = ctx->ax._ui & 0xFFFF;
            global_state.ui->draw_point(x, y);
        }
            break;
        case 305:
        {
            global_state.ui->set_color(ctx->ax._ui);
        }
            break;
        case 306:
        {
            global_state.ui->clear(ctx->ax._ui);
        }
            break;
        case 307:
        {
            auto x = ctx->ax._ui >> 16;
            auto y = ctx->ax._ui & 0xFFFF;
            global_state.ui->fill_rect(x, y);
        }
            break;
        default:
#if LOG_SYSTEM
            ATLTRACE("[SYSTEM] ERR  | unknown interrupt: %d\n", ctx->ax._i);
#endif
            error("unknown interrupt");
            break;
        }
        ctx->pc += INC_PTR;
        return false;
    }

    bool cvm::web(int id) {
        switch (id) {
        case 401:
        {
            auto json = vmm_getstr(ctx->ax._ui);
            std::stringstream ss;
            try {
                clib::cparser_json p(json);
                auto root = p.parse();
                clib::cast_json::print(root, 0, ss);
            }
            catch (const std::exception&) {
#if LOG_SYSTEM
                ATLTRACE("[SYSTEM] ERR  | JSON PARSE ERROR: %s\n", json.c_str());
#endif
                ss << "{\"Error\": \"clibparser::error\"}";
            }
            auto s = ss.str();
            ctx->ax._ui = vmm_malloc(s.length() + 1);
            vmm_setstr(ctx->ax._ui, s);
            break;
        }
        default:
#if LOG_SYSTEM
            ATLTRACE("[SYSTEM] ERR  | unknown interrupt: %d\n", ctx->ax._i);
#endif
            error("unknown interrupt");
            break;
        }
        ctx->pc += INC_PTR;
        return false;
    }

    bool cvm::interrupt() {
        auto id = vmm_get(ctx->pc);
        if (id > 200 && id < 300)
            return math(id);
        if (id >= 300 && id < 400)
            return gui(id);
        if (id >= 400 && id < 500)
            return web(id);
        switch (id) {
        case 0:
        case 1:
        case 2:
        case 4:
        case 6:
        case 7:
        case 9:
            if (output(id)) return true;
            break;
        case 3:
            ctx->debug = !ctx->debug;
            break;
        case 5:
            vmm_setstr((uint32_t)ctx->ax._i, global_state.hostname);
            break;
        case 8:
            if (global_state.input_lock == ctx->id) {
                cgui::singleton().input_char(ctx->ax._c);
            }
            break;
        case 10: {
            if (ctx->input_redirect != -1) {
                ctx->ax._i = ctx->input_stop ? 0 : 1;
                ctx->pc += INC_PTR;
            }
            else {
                if (global_state.input_lock == -1) {
                    global_state.input_lock = ctx->id;
                    ctx->pc += INC_PTR;
                    cgui::singleton().input_set(true);
                }
                else {
                    global_state.input_waiting_list.push_back(ctx->id);
                    ctx->state = CTS_WAIT;
                    ctx->pc -= INC_PTR;
                }
            }
            return true;
        }
        case 11: {
            if (ctx->input_redirect != -1) {
                if (!ctx->input_queue.empty()) {
                    ctx->ax._i = ctx->input_queue.front();
                    ctx->input_queue.pop_front();
                    break;
                }
                else if (!ctx->input_stop) {
                    ctx->pc -= INC_PTR;
                    return true;
                }
                else {
                    ctx->input_redirect = -1;
                }
            }
            else if (global_state.input_lock == ctx->id) {
                if (global_state.input_success) {
                    if (global_state.input_read_ptr >= (int)global_state.input_content.length()) {
                        ctx->ax._i = -1;
                        ctx->pc += INC_PTR;
                        // INPUT COMPLETE
                        for (auto& _id : global_state.input_waiting_list) {
                            if (tasks[_id].flag & CTX_VALID) {
                                assert(tasks[_id].state == CTS_WAIT);
                                tasks[_id].state = CTS_RUNNING;
                            }
                        }
                        global_state.input_lock = -1;
                        global_state.input_waiting_list.clear();
                        global_state.input_read_ptr = -1;
                        global_state.input_content.clear();
                        global_state.input_success = false;
                        cgui::singleton().input_set(false);
                        return true;
                    }
                    else {
                        ctx->ax._i = global_state.input_content[global_state.input_read_ptr++];
                        break;
                    }
                }
                else {
                    ctx->pc -= INC_PTR;
                    return true;
                }
            }
            ctx->ax._i = -1;
            ctx->pc += INC_PTR;
            return true;
        }
        case 12: {
            if (ctx->input_redirect == -1 && global_state.input_lock == ctx->id) {
                if (global_state.input_success) {
                    // INPUT INTERRUPT
                    for (auto& _id : global_state.input_waiting_list) {
                        if (tasks[_id].flag & CTX_VALID) {
                            assert(tasks[_id].state == CTS_WAIT);
                            tasks[_id].state = CTS_RUNNING;
                        }
                    }
                    global_state.input_lock = -1;
                    global_state.input_waiting_list.clear();
                    global_state.input_read_ptr = -1;
                    global_state.input_content.clear();
                    global_state.input_success = false;
                    cgui::singleton().input_set(false);
                }
            }
            else {
                ctx->input_stop = false;
            }
        }
                 break;
        case 13: {
            ctx->ax._i = ctx->input_redirect != -1 ? 0 : 1;
        }
                 break;
        case 14: {
            if (ctx->input_redirect != -1) {
                if (!ctx->input_queue.empty()) {
                    ctx->ax._i = 0;
                    break;
                }
                else if (!ctx->input_stop) {
                    ctx->pc -= INC_PTR;
                    return true;
                }
                else {
                    ctx->input_redirect = -1;
                }
            }
            else if (global_state.input_lock == ctx->id) {
                if (global_state.input_success) {
                    if (global_state.input_read_ptr >= (int)global_state.input_content.length()) {
                        ctx->ax._i = -1;
                        ctx->pc += INC_PTR;
                        // INPUT COMPLETE
                        for (auto& _id : global_state.input_waiting_list) {
                            if (tasks[_id].flag & CTX_VALID) {
                                assert(tasks[_id].state == CTS_WAIT);
                                tasks[_id].state = CTS_RUNNING;
                            }
                        }
                        global_state.input_lock = -1;
                        global_state.input_waiting_list.clear();
                        global_state.input_read_ptr = -1;
                        global_state.input_content.clear();
                        global_state.input_success = false;
                        cgui::singleton().input_set(false);
                        return true;
                    }
                    else {
                        ctx->ax._i = global_state.input_content[global_state.input_read_ptr];
                        if (ctx->ax._i > 0) {
                            ctx->ax._i = 0;
                        }
                        break;
                    }
                }
                else {
                    ctx->pc -= INC_PTR;
                    return true;
                }
            }
            ctx->ax._i = -1;
            ctx->pc += INC_PTR;
            return true;
        }
        case 20: {
            if (global_state.input_lock == -1) {
                set_resize_id = ctx->id;
                cgui::singleton().resize(ctx->ax._i >> 16, ctx->ax._i & 0xFFFF);
            }
            else {
                if (global_state.input_lock != ctx->id)
                    global_state.input_waiting_list.push_back(ctx->id);
                ctx->state = CTS_WAIT;
                ctx->pc -= INC_PTR;
                return true;
            }
            break;
        }
        case 30:
            if (ctx->ax._i != 0)
                ctx->ax._i = vmm_malloc(ctx->ax._ui);
            break;
        case 31:
            ctx->ax._i = vmm_free(ctx->ax._ui);
            break;
        case 40:
            destroy(ctx->id);
            return true;
        case 51:
            ctx->ax._i = exec_file(vmm_getstr(ctx->ax._ui));
            ctx->pc += INC_PTR;
            return true;
        case 50:
            ctx->ax._i = ctx->id;
            break;
        case 52: {
            if (!ctx->child.empty()) {
                ctx->state = CTS_WAIT;
                ctx->pc += INC_PTR;
                return true;
            }
            else {
                ctx->ax._i = -1;
            }
        }
                 break;
        case 53: {
            ctx->ax._i = exec_file(vmm_getstr(ctx->ax._ui));
            if (ctx->ax._i >= 0 && ctx->ax._i < TASK_NUM)
                tasks[ctx->ax._i].state = CTS_WAIT;
            break;
        }
        case 54: {
            if (ctx->ax._i >= 0 && ctx->ax._i < TASK_NUM) {
                if (ctx->child.find(ctx->ax._i) != ctx->child.end())
                    tasks[ctx->ax._i].state = CTS_RUNNING;
            }
            break;
        }
        case 55: {
            ctx->pc += INC_PTR;
            ctx->ax._i = fork();
            return true;
        }
        case 56: {
            auto left = ctx->ax._ui >> 16;
            auto right = ctx->ax._ui & 0xFFFF;
            if ((left == ctx->id || ctx->child.find(left) != ctx->child.end()) &&
                (right == ctx->id || ctx->child.find(right) != ctx->child.end())) {
                tasks[right].input_redirect = left;
                tasks[left].output_redirect = right;
            }
            break;
        }
        case 57: {
            std::vector<int> ids(ctx->child.begin(), ctx->child.end());
            for (auto& id : ids) {
                destroy(id);
            }
            break;
        }
        case 58: {
            if ((ctx->flag & CTX_FOREGROUND) != 0) {
                ctx->flag &= ~CTX_FOREGROUND;
            }
            else {
                ctx->flag |= CTX_FOREGROUND;
            }
            break;
        }
        case 59: {
            if (ctx->ax._i) {
                set_cycle_id = ctx->id;
            }
            else {
                set_cycle_id = -1;
            }
            cgui::singleton().set_cycle(ctx->ax._i);
            break;
        }
        case 60:
            vmm_setstr(ctx->ax._ui, fs.get_pwd());
            break;
        case 61:
            vmm_setstr(ctx->ax._ui, fs.get_user());
            break;
        case 62:
            ctx->ax._i = fs.cd(trim(vmm_getstr(ctx->ax._ui)));
            break;
        case 63:
            ctx->ax._i = fs.mkdir(trim(vmm_getstr(ctx->ax._ui)));
            break;
        case 64:
            ctx->ax._i = fs.touch(trim(vmm_getstr(ctx->ax._ui)));
            break;
        case 65: {
            auto path = trim(vmm_getstr(ctx->ax._ui));
            vfs_node_dec* dec;
            auto s = fs.get(path, &dec, this);
            if (s != 0) {
                ctx->ax._i = s;
                break;
            }
            auto h = new_handle(h_file);
            handles[h].name = path;
            handles[h].data.file = dec;
            ctx->ax._i = h;
        }
                 break;
        case 66: {
            auto h = ctx->ax._i;
            if (ctx->handles.find(h) != ctx->handles.end()) {
                auto dec = handles[h].data.file;
                ctx->ax._i = dec->index();
                if (ctx->ax._i == WAIT_CHAR) {
                    ctx->pc -= INC_PTR;
                    ctx->ax._i = h;
                    return true;
                }
                if (ctx->ax._i < READ_EOF) {
                    dec->advance();
                }
            }
            else {
                ctx->ax._i = READ_ERROR;
            }
        }
                 break;
        case 67: {
            auto h = ctx->ax._i;
            if (ctx->handles.find(h) != ctx->handles.end()) {
                destroy_handle(h);
            }
            else {
                ctx->ax._i = -1;
            }
        }
                 break;
        case 68: {
            ctx->ax._i = fs.rm_safe(trim(vmm_getstr(ctx->ax._ui)));
        }
        case 69: {
            auto h = ctx->ax._ui >> 16;
            auto c = (ctx->ax._ui & 0xFFFF) - 0x1000;
            if (ctx->handles.find(h) != ctx->handles.end()) {
                auto dec = handles[h].data.file;
                ctx->ax._i = dec->write((byte)c);
            }
            else {
                ctx->ax._i = -3;
            }
        }
                 break;
        case 70: {
            auto h = ctx->ax._i;
            if (ctx->handles.find(h) != ctx->handles.end()) {
                auto dec = handles[h].data.file;
                ctx->ax._i = dec->truncate();
            }
            else {
                ctx->ax._i = -3;
            }
        }
                 break;
        case 100: {
            if (ctx->ax._i < 0) {
                ctx->waiting_ms += (-ctx->ax._i) * 0.001;
            }
            else {
                ctx->record_now = std::chrono::system_clock::now();
                ctx->waiting_ms = ctx->ax._i * 0.001;
            }
        }
                  break;
        case 101: {
            auto now = std::chrono::system_clock::now();
            if (std::chrono::duration_cast<std::chrono::duration<decimal>>(
                now - ctx->record_now).count() <= ctx->waiting_ms) {
                ctx->pc -= INC_PTR;
                return true;
            }
        }
                  break;
        case 102:
            // 单位为微秒
            ctx->ax._q = std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000;
            break;
        default:
#if LOG_SYSTEM
            ATLTRACE("[SYSTEM] ERR  | unknown interrupt: %d\n", ctx->ax._i);
#endif
            error("unknown interrupt");
            break;
        }
        ctx->pc += INC_PTR;
        return false;
    }
}
