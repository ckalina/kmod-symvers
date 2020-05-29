#ifndef __ELF_H
#define __ELF_H

# include <libelf.h>
# include <gelf.h>

# ifndef symtab_for_each
#  define symtab_for_each(SYMTAB_DATA, STRTAB_DATA, SYMBOL, HASH)       \
        for (size_t slen = 0;                                           \
             SYMBOL = STRTAB_DATA->d_buf + ((Elf64_Sym *)symtab.data->d_buf)[slen].st_name, \
             HASH = &((Elf64_Sym *)symtab.data->d_buf)[slen].st_value, \
             slen < SYMTAB_DATA->d_size / sizeof(Elf64_Sym);            \
             slen++)
# endif

# ifndef __versions_for_each
#  define __versions_for_each(ELF_DATA, SYMBOL, HASH)                   \
        for (size_t __offset =  0;                                      \
             SYMBOL = ELF_DATA->d_buf + 8 + __offset,                   \
             HASH = ELF_DATA->d_buf + __offset,                         \
             __offset < ELF_DATA->d_size;                               \
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

struct elf_file {
        int fd;
        char *path;
        Elf *elf;
        GElf_Ehdr *ehdr;
        Elf_Scn *scn;
        size_t shstrndx;
        unsigned int endian;
};

struct elf_section {
        Elf_Scn *section;
        Elf_Data *data;
        uint8_t no_bits : 1;
};

static void elf_open(const char *path, struct elf_file *f)
{
        int class;
        Elf_Kind kind;

        if (elf_version(EV_CURRENT) == EV_NONE)
                fail("outdated ELF library.");

        if ((f->fd = open(path, O_RDWR)) == -1)
                fail("cannot open %s", path);

        if (asprintf(&f->path, "%s", path) == -1)
                fail("memory alloc failure");

        f->elf = elf_begin(f->fd, ELF_C_RDWR, NULL);

        if ((kind = elf_kind(f->elf)) != ELF_K_ELF)
                fail("file is not an ELF object");

        if ((f->ehdr = malloc(sizeof(*f->ehdr))) == NULL)
                fail("memory alloc failure\n");

        if (gelf_getehdr(f->elf, f->ehdr) == NULL)
                fail("getehdr: %s\n", elf_errmsg(-1));

        f->endian = f->ehdr->e_ident[EI_DATA];

        if ((class = gelf_getclass(f->elf)) != ELFCLASS64)
                fail("unsupported ELF class: %d\n", class);

        if (elf_getshdrstrndx(f->elf, &f->shstrndx) != 0)
                fail("elf_getshdrstrndx: %s\n", elf_errmsg(-1));
}

static void elf_close(struct elf_file *f)
{
        free(f->path);
        free(f->ehdr);
        elf_end(f->elf);
        close(f->fd);
}

static int elf_get_section(struct elf_file *f, const char *section,
                           struct elf_section *s)
{
        Elf_Scn *scn;
        GElf_Shdr shdr;
        char *name;
        Elf_Data *data;

        scn = elf_nextscn(f->elf, NULL);
        for (; scn != NULL; scn = elf_nextscn(f->elf, scn)) {
                if (gelf_getshdr(scn, &shdr) != &shdr)
                        fail("getshdr: %s\n", elf_errmsg(-1));

                name = elf_strptr(f->elf, f->shstrndx, shdr.sh_name);
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
