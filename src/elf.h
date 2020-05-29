#ifndef __ELF_H
#define __ELF_H

# include <libelf.h>
# include <gelf.h>

# ifndef __versions_for_each
#  define __versions_for_each(ELF_DATA, SYMBOL, HASH)                   \
        for (int __offset =  0;                                         \
             SYMBOL = ELF_DATA->d_buf + 8 + __offset,                   \
                     HASH = ELF_DATA->d_buf + __offset,                 \
                     __offset < ELF_DATA->d_size;                       \
             __offset += 0x40)
# endif

# ifndef get_uint
#  define get_uint(PTR, SIZE, ENDIANITY, RES)                           \
        do {                                                            \
                RES = 0;                                                \
                for (size_t i = 0; i < SIZE; i++)                       \
                        if (ENDIANITY == ELFDATA2MSB)                   \
                                RES = (RES << 8) | PTR[i];              \
                        else                                            \
                                RES = (RES << 8) | PTR[SIZE - i];       \
        } while ((void)(0))
# endif

struct elf_section {
    Elf_Scn *section;
	Elf_Data *data;
    uint8_t no_bits : 1;
};

static int elf_get_section(Elf *elf, size_t shstrndx, const char *section,
                           struct elf_section *s)
{
	Elf_Scn *scn;
	GElf_Shdr shdr;
	char *name;
	Elf_Data *data;

	scn = elf_nextscn(elf, NULL);
	for (; scn != NULL; scn = elf_nextscn(elf, scn)) {
		if (gelf_getshdr(scn, &shdr) != &shdr)
			fail("getshdr: %s\n", elf_errmsg(-1));

		name = elf_strptr(elf, shstrndx, shdr.sh_name);
		if (name == NULL)
			fail("elf_strptr: %s\n", elf_errmsg(-1));

		if (strcmp(name, section) == 0)
			break;
	}

    if (scn == NULL)
		return -1;

    s->section = scn;
    s->no_bits = shdr.sh_type == SHT_NOBITS;

	if (gelf_getshdr(scn, &shdr) != &shdr)
		fail("getshdr: %s\n", elf_errmsg(-1));

	s->data = elf_getdata(scn, NULL);
	if (s->data == NULL || s->data->d_size == 0)
		fail("Section %s empty!\n", section);

    return 0;
}
#endif
