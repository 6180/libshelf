#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../include/elf.h"


Elf_Desc *Elf_Open(const char *path) {
    Elf_Desc *elf_desc;
    long path_length;
    struct stat file_stat;

    stat(path, &file_stat);

    /* Make sure this is a regular file. */
    if (!S_ISREG(file_stat.st_mode)) 
        return NULL;

    /* Make sure we can read the file. */
    if (access(path, R_OK) != 0) 
        return NULL;

    /* Allocate memory for the elf descriptor struct and set the filepath. */
    elf_desc = calloc(1, sizeof(Elf_Desc));
    path_length = strlen(path);
    elf_desc->filepath = malloc(path_length + 1);
    strncpy(elf_desc->filepath, path, path_length);

    /* Check if we can write to the file. */
    if (access(elf_desc->filepath, W_OK) == 0)
        elf_desc->e_writable = 1;

    /* Initialize fields we know at this point. */
    elf_desc->e_size = file_stat.st_size;
    elf_desc->e_readable = 1;
    elf_desc->e_magic = ELF_MAGIC;

    /* Before trying to open make sure the file is at least as large as an elf header. */
    if (elf_desc->e_size < 52) {
        printf("File is smaller than the smallest valid elf header.\n");
    }

    if (elf_desc->e_writable) 
        elf_desc->e_fd = open(elf_desc->filepath, O_RDWR);
    else 
        elf_desc->e_fd = open(elf_desc->filepath, O_RDONLY);

    /* We can't open with the permissions detected earlier for some reason. */
    if (elf_desc->e_fd == -1) {
        printf("Failed opening file\ni");
        elf_desc->e_fd = 0;
        goto error;
    }
    
    elf_desc->e_rawdata = mmap(
        NULL,
        elf_desc->e_size,
        PROT_READ,
        MAP_PRIVATE,
        elf_desc->e_fd,
        0
    );

    if (elf_desc->e_rawdata == MAP_FAILED) {
        printf("mmap() failed.\n");
        goto error;
    }

    elf_desc->e_mmapped = 1;
    elf_desc->e_ident = elf_desc->e_rawdata;

    return elf_desc;

error:
    fflush(stdout);

    if (elf_desc->e_mmapped)
        munmap(elf_desc->e_rawdata, elf_desc->e_size);

    if (elf_desc->e_fd)
        close(elf_desc->e_fd);

    free(elf_desc->filepath);
    free(elf_desc);

    return (Elf_Desc *) NULL;
}

// ssize_t Elf_Write(Elf_Desc *elf_desc, const char *path) {
//     return (ssize_t) 0;
// }

void Elf_Close(Elf_Desc *elf_desc) {
    (void) elf_desc;
}

void Elf_Dump_Ident(Elf_Desc *desc) {
    if (desc == NULL) {
        printf("Null pointer passed to Elf_Dump_Ident()\n");
        exit(-1);
    }

    printf("E_Ident:\n");
    printf("\tMagic: %02x %02x %02x %02x\n",
        desc-> e_ident[EI_MAG0],
        desc-> e_ident[EI_MAG1],
        desc-> e_ident[EI_MAG2],
        desc-> e_ident[EI_MAG3]        
    );

    printf("\tClass: %s\n", get_elf_class(desc->e_ident[EI_CLASS]));


    printf("\tData: ");
    switch (desc->e_ident[EI_DATA]) {
      case 1: {
        printf("2's compliment, little endian\n");
        break;
      }
      case 2: {
        printf("2's compliment, big endian\n");        
        break;
      }
      default: {
        printf("Invalid data format (0x%02x)\n", desc->e_ident[EI_DATA]);
        break;
      }
    }

    printf("\tVersion: ");
    if (desc->e_ident[EI_VERSION] == 1)
        printf("1 (current version)\n");
    else
        printf("%d (invalid version)\n", desc->e_ident[EI_VERSION]);

    printf("\tOS/ABI: %s\n");
}

static const char* get_elf_class(unsigned int elf_class) {
    static char buf[32];

    switch (elf_class) {
        case 0: return "none";
        case 1: return "ELF32";
        case 2: return "ELF64";
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", elf_class);
            return buf;
        }
    }
}

static const char *get_data_encoding(unsigned int encoding) {
    static char buf[32];

    switch (encoding) {
        case 0: return "none";
        case 1: return "2's compliment, little-endian";
        case 2: return "2's compliment, big-endian";
        default: {
            snprintf(buf, sizeof(buf), "<unknown: 0x%02x>", encoding);
            return buf;
        }
    }
}
