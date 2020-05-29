#define _GNU_SOURCE

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

#include "utils.h"
#include "elf.h"

int main(int argc, const char **argv)
{
    char *symbol;
    uint32_t *hash;
    struct elf_file file;
    struct elf_section strtab, symtab, __versions;

	if (argc < 2 || argc > 2 && argc % 2 != 0)
		fail("Usage: %s elf_file [SYMBOL NEW_CHECKSUM]...", argv[0]);

    elf_open(argv[1], &file);

    elf_get_section(&file, "__versions", &__versions);
    elf_get_section(&file, ".strtab", &strtab);
    elf_get_section(&file, ".symtab", &symtab);

    if (symtab.data->d_size % sizeof(Elf64_Sym) != 0)
        fail("unexpected size of symtab section");

    int i = argc;
    while (i-->1) {
        symtab_for_each(symtab.data, strtab.data, symbol, hash) {
            if (strncmp(symbol, "__crc", 5) != 0)
                continue;
            if (i == 1) {
                printf("%08p\t%-40s\tsymtab\n", *hash, symbol);
            }
            if (strcmp(symbol, argv[i]) == 0)
                *hash = strtol(argv[i+1], NULL, 16);
        }

        __versions_for_each(__versions.data, symbol, hash) {
            if (i == 1) {
                printf("%08p\t%-40s\t__versions\n", *hash, symbol);
                continue;
            }
            if (strcmp(symbol, argv[i]) == 0)
                *hash = strtol(argv[i+1], NULL, 16);
        }
    }

    elf_close(&file);

    return 0;
}
