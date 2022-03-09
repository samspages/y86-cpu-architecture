/*
 * CS 261 PA3: Mini-ELF disassembler
 *
 * Name: Samuel Page
 */

#include "p3-disas.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

y86_inst_t fetch (y86_t *cpu, byte_t *memory)
{
    y86_inst_t ins;
    if (cpu == NULL || cpu->pc < 0 || cpu->pc >= MEMSIZE || memory == NULL) {
        ins.icode = INVALID;
        return ins;
    }

    ins.icode = memory[cpu->pc] >> 4;
    ins.ifun.b = memory[cpu->pc] & 0xf;

    switch (ins.icode) {
    case HALT:
        ins.valP = cpu->pc + 1;
        if (ins.ifun.b != 0x00) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case NOP:
        ins.valP = cpu->pc + 1;
        if (ins.ifun.b != 0x00) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case CMOV:
        ins.ra = memory[cpu->pc + 1] >> 4;
        ins.rb = memory[cpu->pc + 1] & 0xf;
        ins.valP = cpu->pc + 2;
        if (ins.ifun.b > 0x6) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case IRMOVQ:
        ins.ra = memory[cpu->pc + 1] >> 4;
        ins.rb = memory[cpu->pc + 1] & 0xf;
        ins.valC.v = *(uint64_t*) (&memory[cpu->pc + 2]);
        ins.valP = cpu->pc + 10;
        if (ins.ifun.b != 0x00 || ins.ra != 15) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case RMMOVQ:
        ins.ra = memory[cpu->pc + 1] >> 4;
        ins.rb = memory[cpu->pc + 1] & 0xf;
        ins.valC.d = *(uint64_t*) (&memory[cpu->pc + 2]);
        ins.valP = cpu->pc + 10;
        if (ins.ifun.b != 0x00) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case MRMOVQ:
        ins.ra = memory[cpu->pc + 1] >> 4;
        ins.rb = memory[cpu->pc + 1] & 0xf;
        ins.valC.d = *(uint64_t*) (&memory[cpu->pc + 2]);
        ins.valP = cpu->pc + 10;
        if (ins.ifun.b != 0x00) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case OPQ:
        ins.ra = memory[cpu->pc + 1] >> 4;
        ins.rb = memory[cpu->pc + 1] & 0xf;
        ins.valP = cpu->pc + 2;
        if (ins.ifun.b > 0x3) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case JUMP:
        ins.valC.dest = *(uint64_t*) (&memory[cpu->pc + 1]);
        ins.valP = cpu->pc + 9;
        if (ins.ifun.b > 0x06) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case CALL:
        ins.valC.dest = *(uint64_t*) (&memory[cpu->pc + 1]);
        ins.valP = cpu->pc + 9;
        if (ins.ifun.b != 0) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case RET:
        ins.valP = cpu->pc + 1;
        if (ins.ifun.b != 0x00) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case PUSHQ:
        ins.ra = memory[cpu->pc + 1] >> 4;
        ins.rb = memory[cpu->pc + 1] & 0xf;
        ins.valP = cpu->pc + 2;
        if (ins.ifun.b != 0x00 || ins.rb != 15) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case POPQ:
        ins.ra = memory[cpu->pc + 1] >> 4;
        ins.rb = memory[cpu->pc + 1] & 0xf;
        ins.valP = cpu->pc + 2;
        if (ins.ifun.b != 0x00 || ins.rb != 15) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case IOTRAP:
        ins.valP = cpu->pc + 1;
        if (ins.ifun.b > 5) {
            ins.icode = INVALID;
            cpu->stat = INS;
        }
        break;
    case INVALID:
        break;
    default:
        ins.icode = INVALID;
        cpu->stat = INS;
    }
    return ins;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p3 (char **argv)
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
}

bool parse_command_line_p3 (int argc, char **argv,
                            bool *print_header, bool *print_segments,
                            bool *print_membrief, bool *print_memfull,
                            bool *disas_code, bool *disas_data, char **filename)
{
    // null checks
    if (argc <= 1 || argv == NULL || print_header == NULL || print_membrief == NULL
            || print_segments == NULL || print_memfull == NULL) {
        usage_p3(argv);
        return false;
    }

    int opt;
    while ((opt = getopt(argc, argv, "hHafsmMdD")) != - 1) {
        switch(opt) {
        case 'h':
            *print_header = false;
            usage_p3(argv);
            return true;
        case 'H':
            *print_header = true;
            break;
        case 'a':
            *print_header = true;
            *print_membrief = true;
            *print_segments = true;
            break;
        case 'f':
            *print_header = true;
            *print_memfull = true;
            *print_segments = true;
            break;
        case 's':
            *print_segments = true;
            break;
        case 'm':
            *print_membrief = true;
            break;
        case 'M':
            *print_memfull = true;
            break;
        case 'd':
            *disas_code = true;
            break;
        case 'D':
            *disas_data = true;
            break;
        default:
            return false;
        }
    }

    *filename = argv[argc - 1];

    // invalid option check
    if (*print_membrief && *print_memfull) {
        usage_p3(argv);
        return false;
    }

    // invalid file name check
    if (*filename == NULL) {
        usage_p3(argv);
        return false;
    }

    return true;
}

void print_register(y86_regnum_t reg)
{
    switch(reg) {
    case RAX:
        printf("%%rax");
        break;
    case RCX:
        printf("%%rcx");
        break;
    case RDX:
        printf("%%rdx");
        break;
    case RBX:
        printf("%%rbx");
        break;
    case RSP:
        printf("%%rsp");
        break;
    case RBP:
        printf("%%rbp");
        break;
    case RSI:
        printf("%%rsi");
        break;
    case RDI:
        printf("%%rdi");
        break;
    case R8:
        printf("%%r8");
        break;
    case R9:
        printf("%%r9");
        break;
    case R10:
        printf("%%r10");
        break;
    case R11:
        printf("%%r11");
        break;
    case R12:
        printf("%%r12");
        break;
    case R13:
        printf("%%r13");
        break;
    case R14:
        printf("%%r14");
        break;
    case NOREG:
        break;
    }
}

void disassemble (y86_inst_t inst)
{
    switch (inst.icode) {
    case HALT:
        printf("halt");
        break;
    case NOP:
        printf("nop");
        break;
    case CMOV:
        switch (inst.ifun.cmov) {
        case RRMOVQ:
            printf("rrmovq ");
            break;
        case CMOVLE:
            printf("cmovle ");
            break;
        case CMOVL:
            printf("cmovl ");
            break;
        case CMOVE:
            printf("cmove ");
            break;
        case CMOVNE:
            printf("cmovne ");
            break;
        case CMOVGE:
            printf("cmovge ");
            break;
        case CMOVG:
            printf("cmovg ");
            break;
        case BADCMOV:
            return;
        }
        print_register(inst.ra);
        printf(", ");
        print_register(inst.rb);
        break;

    case IRMOVQ:
        printf("irmovq ");
        printf("%#lx, ", (uint64_t) inst.valC.v);
        print_register(inst.rb);
        break;
    case RMMOVQ:
        printf("rmmovq ");
        print_register(inst.ra);
        if (inst.rb == NOREG) {
            printf(", %#lx", (uint64_t) inst.valC.d);
        } else {
            printf(", %#lx(", (uint64_t) inst.valC.d);
            print_register(inst.rb);
            printf(")");
        }
        break;
    case MRMOVQ:
        printf("mrmovq ");
        if (inst.rb == NOREG) {
            printf("%#lx", (uint64_t) inst.valC.d);
            printf(", ");
        } else {
            printf("%#lx(", (uint64_t) inst.valC.d);
            print_register(inst.rb);
            printf("), ");
        }
        print_register(inst.ra);
        break;

    case OPQ:
        switch (inst.ifun.op) {
        case ADD:
            printf("addq ");
            break;
        case SUB:
            printf("subq ");
            break;
        case AND:
            printf("andq ");
            break;
        case XOR:
            printf("xorq ");
            break;
        case BADOP:
            return;
        }
        print_register(inst.ra);
        printf(", ");
        print_register(inst.rb);
        break;

    case JUMP:
        switch (inst.ifun.jump) {
        case JMP:
            printf("jmp ");
            break;
        case JLE:
            printf("jle ");
            break;
        case JL:
            printf("jl ");
            break;
        case JE:
            printf("je ");
            break;
        case JNE:
            printf("jne ");
            break;
        case JGE:
            printf("jge ");
            break;
        case JG:
            printf("jg ");
            break; //gap ggez
        case BADJUMP:
            return;
        }
        printf("%#lx", (uint64_t) inst.valC.dest);
        break;

    case CALL:
        printf("call ");
        printf("%#lx", (uint64_t) inst.valC.dest);
        break;
    case RET:
        printf("ret");
        break;
    case PUSHQ:
        printf("pushq ");
        print_register(inst.ra);
        break;
    case POPQ:
        printf("popq ");
        print_register(inst.ra);
        break;

    case IOTRAP:
        printf("iotrap ");
        switch (inst.ifun.trap) {
        case CHAROUT:
            printf("0");
            break;
        case CHARIN:
            printf("1");
            break;
        case DECOUT:
            printf("2");
            break;
        case DECIN:
            printf("3");
            break;
        case STROUT:
            printf("4");
            break;
        case FLUSH:
            printf("5");
            break;
        case BADTRAP:
            return;
        }
    case INVALID:
        return;
    }
}

// prints 2 spaces for every empty byte slot
void print_space_helper(int num)
{
    for (int i = 0; i < 10 - num; i++) {
        printf("  ");
    }
}

// returns byte size of instruction
int instruction_size_helper(y86_inst_t ins)
{
    switch (ins.icode) {
    case INVALID:
        return 0;
    case HALT:
    case NOP:
    case RET:
    case IOTRAP:
        return 1;
    case CMOV:
    case PUSHQ:
    case POPQ:
    case OPQ:
        return 2;
    case JUMP:
    case CALL:
        return 9;
    default:
        return 10;
    }
}

void disassemble_code (byte_t *memory, elf_phdr_t *phdr, elf_hdr_t *hdr)
{
    if (memory == NULL || phdr == NULL || hdr == NULL) {
        return;
    }

    y86_t cpu; // struct to store program counter
    y86_inst_t ins; // struct to store fetched instruction
    cpu.pc = phdr->p_vaddr; // start at beginning of the segment


    printf("  0x%03lx:%31s%03lx code", (uint64_t) cpu.pc, "| .pos 0x", (uint64_t) cpu.pc);
    printf("\n");

    while (cpu.pc < phdr->p_vaddr + phdr->p_filesz) {
        ins = fetch(&cpu, memory); // stage 1 get instruction

        if (ins.icode == INVALID) {
            printf("Invalid opcode: %#02x\n\n", ins.icode);
            return;
        }

        if (cpu.pc == hdr->e_entry) {
            printf("  0x%03lx:%31s", (uint64_t) cpu.pc, "| _start:");
            printf("\n");
        }

        printf("  %#03lx: ", cpu.pc);
        for (int i = cpu.pc; i <  cpu.pc + instruction_size_helper(ins); i++) {
            printf("%02x", memory[i]);
        }

        print_space_helper(instruction_size_helper(ins));
        printf(" |   ");
        disassemble(ins); // stage 2 print disassembled instruction
        printf("\n");
        cpu.pc = ins.valP; // stage 3 update program counter
    }

    printf("\n");
}

void disassemble_data (byte_t *memory, elf_phdr_t *phdr)
{
}

void disassemble_rodata (byte_t *memory, elf_phdr_t *phdr)
{
}

