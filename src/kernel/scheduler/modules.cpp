#include <modules.h>
#include <scheduler.h>
#include <fs/fs.h>
#include <elf.h>
#include <symbols.h>
#include <lib/hashMap.h>

namespace Modules
{
    StringHashMap<LoadedModule> map;

    void init()
    {
        new (&map) StringHashMap<LoadedModule>(10);
    }

    bool loadModule(fs::FileNode *file)
    {
        assert(file);
        Elf64_Header *header = (Elf64_Header *)kmalloc(sizeof(Elf64_Header));
        file->read(0, sizeof(Elf64_Header), header);

        if (header->e_ident[EI_MAG0] != ELFMAG0 || header->e_ident[EI_MAG1] != ELFMAG1 || header->e_ident[EI_MAG2] != ELFMAG2 || header->e_ident[EI_MAG3] != ELFMAG3)
        {
            Log::warn("Unable to load module: invalid magic");
            return false;
        }

        if (header->e_ident[EI_CLASS] != ELFCLASS64)
        {
            Log::warn("Unable to load module: no-64 bits not supported");
            return false;
        }

        if (header->e_ident[EI_DATA] != ELFDATA2LSB)
        {
            Log::warn("Unable to load module: no-LSB not supported");
            return false;
        }

        if (header->e_type != ET_REL)
        {
            Log::warn("Unable to load module: Type not supported");
        }

        uint size = header->e_shnum * header->e_shentsize;
        Elf64_Shdr *sections = (Elf64_Shdr *)kmalloc(size);
        file->read(header->e_shoff, size, sections);

        uint symbolsTableIndex;
        Elf64_Sym *symbols = nullptr;
        char *sectionNames = nullptr;

        struct StrTable
        {
            uint index;
            char *data;
        };

        Vector<StrTable> strTables(1);

#define findSection(type, code)                \
    for (uint i = 0; i < header->e_shnum; i++) \
    {                                          \
        Elf64_Shdr *sheader = &sections[i];    \
        if (sheader->sh_type != type)          \
            continue;                          \
        code                                   \
    }

        findSection(SHT_STRTAB, {
            char *data = (char *)kmalloc(sheader->sh_size);
            file->read(sheader->sh_offset, sheader->sh_size, data);
            if (i == header->e_shstrndx)
            {
                assert(!sectionNames);
                sectionNames = data;
            }
            else
            {
                strTables.push({.index = i, .data = data});
            }
        });

        assert(sectionNames);

        findSection(SHT_SYMTAB, {
            symbols = (Elf64_Sym *)kmalloc(sheader->sh_size);
            file->read(sheader->sh_offset, sheader->sh_size, symbols);
            symbolsTableIndex = i;
            break;
        });

        assert(symbols);

        size = 0;
        for (uint i = 0; i < header->e_shnum; i++)
        {
            Elf64_Shdr *sheader = &sections[i];
            if (sheader->sh_flags & SHF_ALLOC)
                size += sheader->sh_size;
        }

        void *loadAddress = Memory::Virtual::allocModuleSpace(size);
        void *loadAddressCurrent = loadAddress;

        for (uint i = 0; i < header->e_shnum; i++)
        {
            Elf64_Shdr *sheader = &sections[i];
            switch (sheader->sh_type)
            {
            case SHT_PROGBITS:
                file->read(sheader->sh_offset, sheader->sh_size, loadAddressCurrent);
                sheader->sh_addr = (uint64)loadAddressCurrent;
                loadAddressCurrent = (void *)((uint64)loadAddressCurrent + sheader->sh_size);
                break;

            case SHT_NOBITS:
                memset(loadAddressCurrent, 0, sheader->sh_size);
                sheader->sh_addr = (uint64)loadAddressCurrent;
                loadAddressCurrent = (void *)((uint64)loadAddressCurrent + sheader->sh_size);
                break;

            default:
                break;
            }
        }

        Module *module = nullptr;

        {
            uint strTabIndex = sections[symbolsTableIndex].sh_link;
            StrTable *table = strTables.find([strTabIndex](const StrTable &t)
                                             { return t.index == strTabIndex; });
            assert(table);

            for (uint i = 0; i < (sections[symbolsTableIndex].sh_size / sections[symbolsTableIndex].sh_entsize); i++)
            {
                if (symbols[i].st_shndx == SHN_UNDEF)
                {
                    if (symbols[i].st_name != 0)
                        symbols[i].st_value = KernelSymbols::get(table->data + symbols[i].st_name);
                }
                else if (symbols[i].st_shndx < SHN_LOPROC)
                {
                    symbols[i].st_value = sections[symbols[i].st_shndx].sh_addr + symbols[i].st_value;
                }

                if (String(table->data + symbols[i].st_name) == "__module")
                {
                    module = (Module *)symbols[i].st_value;
                }
            }

            assert(module);
        }

        findSection(SHT_RELA, {
            Elf64_Rela *rela = (Elf64_Rela *)kmalloc(sheader->sh_size);
            file->read(sheader->sh_offset, sheader->sh_size, rela);

            Elf64_Shdr *section = &sections[sheader->sh_info];

            for (uint i = 0; i < sheader->sh_size / sheader->sh_entsize; i++)
            {
                Elf64_Rela *rel = &rela[i];
                Elf64_Sym *sym = &symbols[ELF64_R_SYM(rel->r_info)];

                uint64 target = rel->r_offset + section->sh_addr;
                switch (ELF64_R_TYPE(rel->r_info))
                {
                case R_X86_64_64:
                    *(uint64 *)target = sym->st_value + rel->r_addend;
                    break;

                default:
                    assert(!"Invalid relocation type");
                    break;
                }
            }

            kfree(rela);
        });

#undef findSection

        kfree(header);
        kfree(sections);
        for (uint i = 0; i < strTables.size(); i++)
            kfree(strTables[i].data);
        kfree(symbols);

        if (module->init())
            return false;

        map.set(String(module->name), {.data = module, .address = (uint64)loadAddress, .size = size});

        Log::info("Module %s loaded", module->name);

        return true;
    }
} // namespace Modules
