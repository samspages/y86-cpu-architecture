/*
 * CS 261 PA1: Mini-ELF header verifier
 *
 * Name: Samuel Page
 */

#include "p1-check.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

#define MAGIC_NUM 4607045

bool read_header (FILE *file, elf_hdr_t *hdr)
{
    int size = sizeof(elf_hdr_t);
    // check header validity
    if (hdr == NULL) {
        return false;
    }
    // read into header and check # of bytes read
    if (fread(hdr, 1, size, file)  < 16) {
        return false;
    }

    if (hdr->magic != MAGIC_NUM) {
        return false;
    }

    return true;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p1 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
}

bool parse_command_line_p1 (int argc, char **argv, bool *print_header, char **filename)
{
    int c;
    bool big_booling = false;
    bool lil_booling = false;

    while ((c = getopt(argc, argv, "hH")) != -1) {
        switch(c) {
        case 'h':
            lil_booling = true;
            big_booling = false;
            *print_header = false;
            break;
        case 'H':
            big_booling = true;
            if (argc != 3) {
                *print_header = false;
                lil_booling = true;
            } else {
                if (!lil_booling) {
                    *print_header = true;
                }
            }
            break;
        default:
            *print_header = false;
            lil_booling = true;
            break;
        }
    }

    *filename = argv[argc - 1];

    if (lil_booling) {
        usage_p1(argv);
    }

    if (lil_booling || big_booling) {
        return true;
    }

    return false;
}

void dump_header (elf_hdr_t hdr)
{
    printf("%02x ", (hdr.e_version & 0xff));
    printf("%02x ", (hdr.e_version >> 8) & 0xff);

    printf("%02x ", (hdr.e_entry & 0xff));
    printf("%02x ", (hdr.e_entry >> 8) && 0xff);

    printf("%02x ", (hdr.e_phdr_start & 0xff));
    printf("%02x ", (hdr.e_phdr_start >> 8) & 0xff);

    printf("%02x ", (hdr.e_num_phdr & 0xff));
    printf("%02x  ", (hdr.e_num_phdr >> 8) & 0xff);

    printf("%02x ", (hdr.e_symtab & 0xff));
    printf("%02x ", (hdr.e_symtab >> 8) & 0xff);

    printf("%02x ", (hdr.e_strtab & 0xff));
    printf("%02x ", (hdr.e_strtab >> 8) & 0xff);
    // output elf header structure hex

    printf("%02x ", (hdr.magic & 0xff));
    printf("%02x ", (hdr.magic >> 8) & 0xff);
    printf("%02x ", (hdr.magic >> 16) & 0xff);
    printf("%02x", (hdr.magic >> 24) & 0xff);
    printf("\n");
    // output magic number hex

    printf("Mini-ELF version %x\n", hdr.e_version & 0xff);
    printf("%s%#03x", "Entry point ", hdr.e_entry);
    printf("\n");
    // entry point output

    printf("%s%d%s%d%s%#2x%s", "There are ", hdr.e_num_phdr, " program headers, starting at offset ",
           hdr.e_phdr_start, " (", hdr.e_phdr_start, ")\n");

    if (hdr.e_symtab > 0) {
        printf("%s%d%s%#2x%s","There is a symbol table starting at offset ", hdr.e_symtab, " (",
               hdr.e_symtab, ")\n");
    } else {
        printf("There is no symbol table present\n");
    }

    if (hdr.e_strtab > 0) {
        printf("%s%d%s%#2x%s","There is a string table starting at offset ", hdr.e_strtab, " (",
               hdr.e_strtab, ")\n");
    } else {
        printf("There is no string table present\n");
    }
    // symbol table and string table outputs
}

