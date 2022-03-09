/*
 * CS 261 PA2: Mini-ELF loader
 *
 * Name: Samuel Hayden Page
 */

#include "p2-load.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{
    int size = sizeof(elf_phdr_t);
    long int magic_num = 3735928559;

    if (phdr == NULL || file == NULL) {
        return false;
    }

    fseek(file, offset, SEEK_SET);
    fread(phdr, 1, size, file);

    if (phdr->magic != magic_num) {
        return false;
    }

    return true;
}

bool load_segment (FILE *file, byte_t *memory, elf_phdr_t phdr)
{
    int four_k = 4096;
    // null checks
    if (!file || !memory || phdr.p_offset < 0) {
        return false;
    }

    // bad address check
    if (phdr.p_vaddr < 0 || phdr.p_vaddr > four_k || phdr.p_vaddr + phdr.p_filesz > four_k) {
        return false;
    }

    // seek file to given offset
    if (fseek(file, phdr.p_offset, SEEK_SET) != 0) {
        return false;
    }

    // read to memory and check segment size
    //       virtual address of phdr   segment size count file stream   check segment size and # of bytes read
    if (fread(&memory[phdr.p_vaddr], phdr.p_filesz, 1, file) != 1 && phdr.p_filesz) {
        return false;
    }
    return true;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p2 (char **argv)
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
}

bool parse_command_line_p2 (int argc, char **argv,
                            bool *print_header, bool *print_segments,
                            bool *print_membrief, bool *print_memfull,
                            char **filename)
{
    // null checks
    if (argc <= 1 || argv == NULL || print_header == NULL || print_membrief == NULL
            || print_segments == NULL || print_memfull == NULL) {

        usage_p2(argv);
        return false;
    }

    int opt;
    while ((opt = getopt(argc, argv, "hHafsmM")) != - 1) {
        switch(opt) {
            case 'h':
                *print_header = false;
                usage_p2(argv);
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
            default:
                return false;
        }
    }
    *filename = argv[argc - 1];
    // invalid option check
    if (*print_membrief && *print_memfull) {
        usage_p2(argv);
        return false;
    }
    // invalid file name check
    if (*filename == NULL) {
        return false;
    }

    return true;
}

void dump_phdrs (uint16_t numphdrs, elf_phdr_t phdr[])
{
    char *type;
    char *flag;

    printf(" Segment   Offset    VirtAddr  FileSize  Type      Flag\n");

    for (int i = 0; i < numphdrs; i++) {
        // cases for given type val
        if (phdr[i].p_type == 0) {
            type = "DATA      ";
        } else if (phdr[i].p_type == 1) {
            type = "CODE      ";
        } else if (phdr[i].p_type == 2) {
            type = "STACK     ";
        } else if (phdr[i].p_type == 3) {
            type = "HEAP      ";
        } else {
            type = "UNKNOWN   ";
        }
        // get flag
        flag = flag_helper(phdr[i].p_flag);
        printf("  %02d       0x%04x    0x%04x    0x%04x    %s",
               i, phdr[i].p_offset, phdr[i].p_vaddr, phdr[i].p_filesz, type);
        printf("%s\n", flag);
    }
}

char* flag_helper(uint16_t flag_num)
{
    char *result = "   ";

    if (flag_num > 0 && flag_num <= 7) {
        if (flag_num == 1) {
            result = "  X";
        }
        if (flag_num == 2) {
            result = " W ";
        }
        if (flag_num == 4) {
            result = "R  ";
        }
        if (flag_num == 5) {
            result = "R X";
        }
        if (flag_num == 6) {
            result = "RW ";
        }
        if (flag_num == 7) {
            result = "RWX";
        }
        if (flag_num == 3) {
            result = " WX";
        }

    }
    return result;
}
void dump_memory (byte_t *memory, uint16_t start, uint16_t end)
{
    printf("Contents of memory from %04x to %04x:\n", start, end);
    for (int i = start; i <= end; i++) {
        if (i == end) {
            printf("\n");
            break;
        }
        // end file with new line
        if (i % 16 == 0) {
            if (i > 0) {
                printf("\n");
            }
            printf("  %04x", i);
        }
        // print new line and start address every 2 byte
        if (i % 8 == 0) {
            printf(" ");
        }
        // print new space every byte
        if (i < start) {
            printf("   ");
        } else {
            printf(" %02x", memory[i]);
        }
    }
}

