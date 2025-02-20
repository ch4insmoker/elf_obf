#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

// changing elf header info
void patch_hdr(FILE *fp) {
    Elf64_Ehdr hdr;

    fseek(fp, 0, SEEK_SET);
    fread(&hdr, sizeof(Elf64_Ehdr), 1, fp);
    hdr.e_shoff =  0x0;
    hdr.e_shentsize = 0x539;
    hdr.e_shnum = 0x539;
    hdr.e_shstrndx = 0x0;
    hdr.e_ehsize = 0x0;
    hdr.e_ident[5] = ELFDATA2MSB;
    fseek(fp, 0, SEEK_SET);
    fwrite(&hdr, sizeof(Elf64_Ehdr), 1, fp);
    
}

void patch_phdr(FILE *fp) {
    Elf64_Ehdr hdr;
    Elf64_Phdr phdr;

    int fd = fileno(fp);
    struct stat st;
    fstat(fd, &st);
    unsigned long long size = st.st_size + 0x10;

    fseek(fp, 0, SEEK_SET);
    fread(&hdr, sizeof(Elf64_Ehdr), 1, fp);

    for (int i = 0; i < hdr.e_phnum; i++) {

        fseek(fp, hdr.e_phoff + i * hdr.e_phentsize, SEEK_SET);
        fread(&phdr, sizeof(Elf64_Phdr), 1, fp);

        if (phdr.p_type == PT_GNU_EH_FRAME || phdr.p_type == PT_NOTE) {

            phdr.p_type = 0x70000001;
            phdr.p_offset = 0x00;
            fseek(fp, hdr.e_phoff + i * hdr.e_phentsize, SEEK_SET);
            fwrite(&phdr, sizeof(Elf64_Phdr), 1, fp);
        }
    }
}

void patch_sections(FILE *fp) {
    Elf64_Ehdr hdr;
    char what = 0x00;

    fseek(fp, 0, SEEK_SET);
    fread(&hdr, sizeof(Elf64_Ehdr), 1, fp);
    
    for (int i = 0; i < hdr.e_shnum; i++) {
        fseek(fp, hdr.e_shentsize * i + hdr.e_shoff, SEEK_SET);
        fwrite(&what, 1, sizeof(Elf64_Shdr), fp);
    }
}

int main(int argc, char **argv) {
    
    FILE *fp;
    if (argc < 2 || (fp = fopen(argv[1], "rb+")) == NULL) {
        fputs("./obf <file>\n", stderr); 
        exit(-1);
    }    

    patch_sections(fp);
    patch_phdr(fp);
    patch_hdr(fp);

    fclose(fp);
}