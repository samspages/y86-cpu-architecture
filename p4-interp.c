/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name: Samuel Page
 */

#include "p4-interp.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

y86_reg_t getOp(y86_reg_t valB, y86_reg_t *valA, y86_inst_t inst, y86_t *cpu)
{
    int64_t sValE = 0;
    y86_reg_t ValE = 0;
    int64_t sValB = valB;
    int64_t sValA = *valA;

    switch(inst.ifun.op) {
        case(ADD):
            sValE = sValB + sValA;
            cpu->of = (sValB < 0 && sValA < 0 && sValE > 0) || (sValB > 0 && sValA > 0 && sValE < 0);
            ValE = sValE;
            break;
        case(SUB):
            sValE = sValB - sValA;
            cpu->of = ((sValB < 0 && sValA > 0 && sValE > 0) || (sValB > 0 && sValA < 0 && sValE < 0 ));
            ValE = sValE;
            break;
        case(AND):
            ValE = valB & *valA;
            break;
        case(XOR):
            ValE = *valA ^ valB;
            break;
        case(BADOP):
            cpu->stat = INS;
            return ValE;
        default :
            cpu ->stat = INS;
            return ValE;
    }

    cpu->sf = (ValE >> 63 == 1);
    cpu->zf = (ValE == 0);
    return ValE;
}

bool cond(y86_cmov_t mov, y86_t *cpu)
{
    bool cnd = false;
    switch(mov) {
        case(RRMOVQ):
            cnd = true;
            break;
        case(CMOVLE):
            if(cpu->zf || (cpu->sf ^ cpu->of)) {
                cnd = true;
            }
            break;
        case(CMOVL):
            if(cpu->sf ^ cpu->of) {
                cnd =  true;
            }
            break;
        case(CMOVE):
            if(cpu->zf) {
                cnd =  true;
            }
            break;
        case(CMOVNE):
            if(!cpu->zf) {
                cnd =  true;
            }
            break;
        case(CMOVGE):
            if(cpu->sf == cpu->of) {
                cnd =  true;
            }
            break;
        case(CMOVG):
            if(!cpu->zf && cpu->sf == cpu->of) {
                cnd =  true;
            }
            break;
        case(BADCMOV):
            cpu->stat = INS;
    }

    return cnd;
}

y86_reg_t decode_execute (y86_t *cpu, y86_inst_t inst, bool *cnd, y86_reg_t *valA)
{
    //null checks
    if (cnd == NULL || valA == NULL) {
        cpu->stat = INS;
        return 0;
    }

    y86_reg_t valE = 0;
    y86_reg_t valB;

    switch (inst.icode) {
        case(HALT):
            cpu->stat = HLT;
            break;
        case(NOP):
            break;
        case(CMOV):
            *valA = inst.ra;
            valE = inst.ra;
            *cnd = cond(inst.ifun.cmov, cpu);
            break;
        case(IRMOVQ):
            valE = inst.valC.v;
            break;
        case(RMMOVQ):
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[inst.rb];
            valE = inst.valC.d + valB;
            break;
        case(MRMOVQ):
            valB = cpu->reg[inst.rb];
            valE = inst.valC.d + valB;
            break;
        case(OPQ):
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[inst.rb];
            valE = getOp(valB, valA, inst, cpu);
            break;
        case(JUMP):
            switch (inst.ifun.jump) {
                case(JMP):
                    *cnd = true;
                    break;
                case(JLE):
                    *cnd = false;
                    if (cpu->zf || (cpu->sf ^ cpu->of)) {
                        *cnd = true;
                    }
                    break;
                case(JL):
                    if(cpu->sf ^ cpu->of) {
                        *cnd =  true;
                    }
                    break;
                case(JE):
                    if(cpu->zf) {
                        *cnd = true;
                    }
                    break;
                case(JNE):
                    if(!cpu->zf) {
                        *cnd = true;
                    }
                    break;
                case(JGE):
                    if(cpu->zf == cpu->of) {
                        *cnd = true;
                    }
                    break;
                case(JG):
                    if(!cpu->zf && cpu->sf == cpu->of) {
                        *cnd = true;
                    }
                    break;
                case(BADJUMP):
                    cpu->stat = INS;
                    break;
            }
            break;
        case(CALL):
            valB = cpu->reg[RSP];
            valE = valB - 8;
            break;
        case(RET):
            *valA = cpu->reg[RSP];
            valB = cpu->reg[RSP];
            valE = valB + 8;
            break;
        case(PUSHQ):
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[RSP];
            valE = valB - 8;
            break;
        case(POPQ):
            *valA = cpu->reg[RSP];
            valB = cpu->reg[RSP];
            valE = valB + 8;
            break;
        case(INVALID):
            cpu->stat = INS;
            break;
        default:
            cpu->stat = INS;
            break;
    }
    return valE;
}

void memory_wb_pc (y86_t *cpu, y86_inst_t inst, byte_t *memory,
                   bool cnd, y86_reg_t valA, y86_reg_t valE)
{
    //null checks
    if (memory == NULL || valE < 0 || valA < 0 || cpu->pc >= MEMSIZE || cpu->pc < 0) {
        cpu->stat = ADR;
        return;
    }
    uint64_t valM;
    uint64_t *m;

    switch(inst.icode) {
        case(HALT):
            cpu->pc = inst.valP;
            break;
        case(NOP):
            cpu->pc = inst.valP;
            break;
        case(CMOV):
            cpu->pc = inst.valP;
            break;
        case(IRMOVQ):
            cpu->pc = inst.valP;
            cpu->reg[inst.rb] = valE;
            break;
        case(RMMOVQ):
            if (valE >= MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            cpu->pc = inst.valP;
            m = (uint64_t *) &memory[valE];
            *m = valA;
            break;
        case(MRMOVQ):
            if (valE >= MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            cpu->pc = inst.valP;
            m = (uint64_t *) &memory[valE];
            valM = *m;
            cpu->reg[inst.ra] = valM;
            break;
        case(OPQ):
            cpu->reg[inst.rb] = valE;
            cpu->pc = inst.valP;
            break;
        case(JUMP):
            if (cnd) {
                cpu->pc = inst.valC.dest;
            }
            break;
        case(CALL):
            if (valE >= MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            cpu->pc = inst.valC.dest;
            m = (uint64_t *) &memory[valE];
            *m = inst.valP;
            cpu->reg[RSP] = valE;
            break;
        case(RET):
            if (valA >= MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            m = (uint64_t *) &memory[valA];
            valM = *m;
            cpu->reg[RSP] = valE;
            cpu->pc = valM;
            break;
        case(PUSHQ):
            if (valE >= MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            cpu->pc = inst.valP;
            m = (uint64_t *) &memory[valE];
            *m = valA;
            cpu->reg[RSP] = valE;
            break;
        case(POPQ):
            if (valA >= MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            cpu->pc = inst.valP;
            m = (uint64_t *) &memory[valA];
            valM = *m;
            cpu->reg[RSP] = valE;
            cpu->reg[inst.ra] = valM;
            break;
        case(INVALID):
            cpu->stat = INS;
            break;
        default:
            cpu->stat = INS;
            break;
    }
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p4 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (trace mode)\n");
}

bool parse_command_line_p4 (int argc, char **argv,
                            bool *header, bool *segments, bool *membrief, bool *memfull,
                            bool *disas_code, bool *disas_data,
                            bool *exec_normal, bool *exec_trace, char **filename)
{
    // null checks
    if (argc <= 1 || argv == NULL || header == NULL || membrief == NULL
            || segments == NULL || memfull == NULL || exec_normal == NULL || exec_trace == NULL) {
        usage_p4(argv);
        return false;
    }

    int opt;
    while ((opt = getopt(argc, argv, "hHafsmMdDeE")) != - 1) {
        switch(opt) {
            case 'h':
                *header = false;
                usage_p4(argv);
                return true;
            case 'H':
                *header = true;
                break;
            case 'a':
                *header = true;
                *membrief = true;
                *segments = true;
                break;
            case 'f':
                *header = true;
                *memfull = true;
                *segments = true;
                break;
            case 's':
                *segments = true;
                break;
            case 'm':
                *membrief = true;
                break;
            case 'M':
                *memfull = true;
                break;
            case 'd':
                *disas_code = true;
                break;
            case 'D':
                *disas_data = true;
                break;
            case 'e':
                *exec_normal = true;
                break;
            case 'E':
                *exec_trace = true;
                break;
            default:
                usage_p4(argv);
                return false;
        }
    }

    *filename = argv[argc - 1];

    // invalid option check
    if (*membrief && *memfull) {
        usage_p4(argv);
        return false;
    }

    // invalid file name check
    if (*filename == NULL) {
        usage_p4(argv);
        return false;
    }

    return true;
}

void dump_cpu_state (y86_t cpu)
{
    //print cpu state
    printf("Y86 CPU state:\n");
    printf("  %%rip: %016lx   flags: Z%d S%d O%d     ",  cpu.pc, cpu.zf, cpu.sf, cpu.of);

    //switch & print cpu status
    switch(cpu.stat) {
        case(1):
            printf("AOK\n");
            break;
        case(2):
            printf("HLT\n");
            break;
        case(3):
            printf("ADR\n");
            break;
        case(4):
            printf("INS\n");
            break;
    }

    // print registers rax - r14
    printf("  %%rax: %016lx    %%rcx: %016lx\n",  cpu.reg[RAX],  cpu.reg[RCX]);
    printf("  %%rdx: %016lx    %%rbx: %016lx\n",  cpu.reg[RDX],  cpu.reg[RBX]);
    printf("  %%rsp: %016lx    %%rbp: %016lx\n",  cpu.reg[RSP],  cpu.reg[RBP]);
    printf("  %%rsi: %016lx    %%rdi: %016lx\n",  cpu.reg[RSI],  cpu.reg[RDI]);
    printf("   %%r8: %016lx     %%r9: %016lx\n",  cpu.reg[R8],  cpu.reg[R9]);
    printf("  %%r10: %016lx    %%r11: %016lx\n",  cpu.reg[R10],  cpu.reg[R11]);
    printf("  %%r12: %016lx    %%r13: %016lx\n",  cpu.reg[R12],  cpu.reg[R13]);
    printf("  %%r14: %016lx", cpu.reg[R14]);
    printf("\n");
}

