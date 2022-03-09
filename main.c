/*
 * CS 261: Main driver
 *
 * Name: Samuel Page
 */

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"

int main (int argc, char **argv)
{
    bool header = false;
    bool segments = false;
    bool membrief = false;
    bool memfull = false;
    bool disas_code = false;
    bool disas_data = false;
    bool exec_normal = false;
    bool exec_trace = false;
    char *filename = NULL;

    // parse command line options and exit program if option select is invalid
    if (!(parse_command_line_p4(argc, argv, &header, &segments, &membrief,
                                &memfull, &disas_code, &disas_data, &exec_normal, &exec_trace, &filename))) {
        return EXIT_FAILURE;
    }

    elf_hdr_t hdr;
    FILE *f = fopen(filename, "r");

    if (!read_header(f, &hdr)) {
        return EXIT_FAILURE;
    }

    elf_phdr_t phdrs[hdr.e_num_phdr];
    int offset;

    //read phdr data into phdrs array and check if null or invalid data
    for (int i = 0; i < hdr.e_num_phdr; i++) {
        offset = hdr.e_phdr_start + (i * sizeof(elf_phdr_t));
        if (!(read_phdr(f, offset, &phdrs[i]))) {
            printf("Failed to read file\n");
            return EXIT_FAILURE;
        }
    }

    // allocate memory for 4kb
    byte_t *memory = (byte_t*) calloc(MEMSIZE, 1);
    for (int n = 0; n < hdr.e_num_phdr; n++) {
        if (!(load_segment(f, memory, phdrs[n]))) {
            free(memory);
            return EXIT_FAILURE;
        }
    }

    if (header) {
        dump_header(hdr);
    }

    if (segments) {
        dump_phdrs(hdr.e_num_phdr, phdrs);
    }

    if (membrief) {
        for (int i = 0; i < hdr.e_num_phdr; i++) {
            dump_memory(memory, phdrs[i].p_vaddr, phdrs[i].p_vaddr + phdrs[i].p_filesz);
        }
    }

    if (memfull) {
        dump_memory(memory, 0, MEMSIZE);
    }

    if (disas_code) {
        printf("Disassembly of executable contents:\n");
        for (int i = 0; i < hdr.e_num_phdr; i++) {
            disassemble_code(memory, &phdrs[i], &hdr);
        }
    }

    if (disas_data) {

    }

    // instance variables for decode, exec, mem, write back
    int count;
    bool cnd = false;
    y86_t cpu;
    // allocate bytes for cpu
    memset(&cpu, 0x00, sizeof(cpu));
    cpu.stat = AOK;
    cpu.pc = hdr.e_entry;
    y86_reg_t valA = 0;
    y86_reg_t valE = 0;

    count = 0;
    if (exec_normal) {
        printf("Beginning execution at 0x%04x\n", hdr.e_entry);

        // loop while cpu is ok
        while (cpu.stat == AOK) {
            // get current instruction
            y86_inst_t ins = fetch(&cpu, memory);
            // decode and set
            valE = decode_execute(&cpu, ins, &cnd, &valA);
            memory_wb_pc(&cpu, ins, memory, cnd, valA, valE);
            count++;
        }

        dump_cpu_state(cpu);
        printf("Total execution count: %d\n", count);
    }

    count = 0;
    if (exec_trace) {
        printf("Beginning execution at 0x%04x\n", hdr.e_entry);
        dump_cpu_state(cpu);

        // loop while cpu = ok
        while (cpu.stat == AOK) {
            // get current instruction
            y86_inst_t ins = fetch(&cpu, memory);
            printf("\nExecuting: ");
            disassemble(ins);
            printf("\n");

            cnd = true;
            valE = decode_execute(&cpu, ins, &cnd, &valA);
            memory_wb_pc(&cpu, ins, memory, cnd, valA, valE);
            dump_cpu_state(cpu);
            count++;
        }
        printf("Total execution count: %d\n", count);
        printf("\n");
        dump_memory(memory, 0, MEMSIZE);
    }

    fclose(f);
    free(memory);
    return EXIT_SUCCESS;
}

