#include <elf.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// const char *sh_type_tb[] = {SHT_NULL, SHT_PROGBITS,
//         SHT_SYMTAB, SHT_STRTAB, SHT_RELA, SHT_HASH,
//         SHT_DYNAMIC};

/**
 * @breif: 打印节头部表
*/
static void print_sh(const Elf64_Shdr *sh, uint32_t indx, uint32_t shstr_idx, const char *buf)
{
    printf("=========================%d:%d=======================\n", indx, shstr_idx);
    printf("[sh_name]: [%s]\n", (buf + sh[indx].sh_name));
    printf("[sh_type]: [%d]\n", sh[indx].sh_type);
    printf("[sh_flags]: [%lu]\n", sh[indx].sh_flags);
    printf("[sh_offset]: [%lx]\n", sh[indx].sh_offset);
    printf("[sh_size]: [%lu]\n", sh[indx].sh_size);
    printf("[sh_size]: [%lu]\n", sh[indx].sh_size);
    printf("[sh_size]: [%lu]\n", sh[indx].sh_size);   
}

static void print_sym(const Elf64_Sym *sh, uint32_t indx, uint32_t shstr_idx, const char *buf)
{
    printf("=========================%d:%d=======================\n", indx, shstr_idx);
    printf("[st_name]: [%d]:[%s]\n", (sh[indx].st_name), (buf+sh[indx].st_name+1));
    printf("[st_value]: [%lu]\n", sh[indx].st_value);
    printf("[st_size]: [%lu]\n", sh[indx].st_size);
    printf("[st_shndx]: [%u]\n", sh[indx].st_shndx);
}

int main()
{
    FILE *fp = fopen("./test.o", "r");
    if (fp)
    {
        Elf64_Ehdr hd;
        
        char buf[128] = {0};
        char str_buf[256] = {0};
        char sym_buf[256] = {0};
        uint32_t i = 0;
        fread(&hd, sizeof(Elf64_Ehdr), 1, fp);
        Elf64_Shdr *sh = (Elf64_Shdr *)calloc(hd.e_shnum, sizeof(Elf64_Shdr));
        
        if (!sh)
        {
            perror("malloc failed");
            fclose(fp);
            return -1;
        }
        fseek(fp, hd.e_shoff, SEEK_SET);
        fread(sh, sizeof(Elf64_Shdr), hd.e_shnum, fp);

        
        fseek(fp, sh[hd.e_shstrndx].sh_offset, SEEK_SET);
        fread(buf, sizeof(char), sh[hd.e_shstrndx].sh_size, fp);
        
        // 12 是用readelf -S 看输出得到的字符串表.strrtab的下标，可以使用段头部表比较每一个明称自动确定下标
        fseek(fp, sh[12].sh_offset, SEEK_SET);
        fread(str_buf, sizeof(char), sh[12].sh_size, fp);
        
        // 字符串表 都是全局生命周期的变量
        printf("\nstrtable.......................\n");
        
        for (i = 0; i < sh[12].sh_size; i++)
        {
            if (str_buf[i] == '\0')
                printf("\n");
            printf("%c", str_buf[i]);
        }
        
        //  暂时解析理解不足，解析太复杂，不解析符号表
        // printf("\nsymtable.......................\n");
        
        // uint8_t cnt = sh[11].sh_size/sizeof(Elf64_Sym);
        // printf("cnt of symtable is:[%lu]:[%d]\n", sh[11].sh_size, cnt);
        // Elf64_Sym *sym = (Elf64_Sym *)calloc(cnt, sizeof(Elf64_Sym));
        // fseek(fp, sh[11].sh_offset, SEEK_SET);
        // fread(sym, sizeof(Elf64_Sym), cnt, fp);
        
        // for (i = 0; i < cnt; i++)
        // {
        //     print_sym(sym, i, hd.e_shstrndx, buf);
        // }
        
        // 段表名字符串表
        printf("\nshstrtable.......................\n");
        
        for (i = 0; i < sh[hd.e_shstrndx].sh_size; i++)
        {
            if (buf[i] == '\0')
                printf("\n");
            else
                printf("%c", buf[i]);
            
        }
        
        // 段头部表
        printf("\nshtable.......................\n");
        
        for (i = 0; i < hd.e_shnum; i++)
        {
            print_sh(sh, i, hd.e_shstrndx, buf);
        }
        free(sh);
        // free(sym);
    }
    fclose(fp);

}