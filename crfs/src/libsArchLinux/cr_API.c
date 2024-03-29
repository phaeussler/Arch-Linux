#include "./cr_API.h"
#include "./utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
 #include <sys/types.h>


/* VARIABLES GLOBALES */
char* DISKNAME;


/* # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # */

/* FUNCIONES GENERALES */
void cr_mount(char* diskname){
    /* Funcion para montar el disco. Establece como variable global laruta local donde se encuentra el archivo.bincorrespondiente al disco */
	DISKNAME = diskname;
	/* TODO-> Atender CASOS BORDE:
	 * archivo diskname no existe
	 */
}

void cr_bitmap(unsigned block, bool hex){
    /* Función  para  imprimir  elbitmap.  Cada  vez  quese llama esta función, imprime enstderrel estado actual del bloque debitmapcorrespondiente ablock(bitmapblock∈{1, ...,129}), ya sea en binario (sihexesfalse) o en hexadecimal (sihexestrue). Sise ingresablock= 0, se debe imprimirel bitmap completo, imprimiendo ademas una linea con la cantidad de bloques ocupados, y una segunda linea con la cantidad de bloques libres.*/
    FILE* disk = fopen(DISKNAME, "rb");
    fseek(disk, 1024, SEEK_SET);
    uint32_t free = 0;
    uint32_t ocupied = 0;
    if (hex){
        uint16_t bitmap;
        if((130 > block) && (block > 0))
        {
            fprintf(stderr,"%d \n", block);
            fseek(disk, 1024*(block-1), SEEK_CUR);
            for(int i = 0; i < 1024; i++){
                fread(&bitmap, 1, 1, disk);
                for(int j = 1; j > -1; j--){
                    uint16_t bitmap2 = bitmap;
                    fprintf(stderr, "%x", (bitmap2>>j*4)&0x0F);
                }
                if(i%2){
                    fprintf(stderr, "\n");
                }
            }
        }
        else if(block == 0)
        {
            fprintf(stderr, "pass");
            for(uint32_t i = 0; i < 1024*128; i++){
                fread(&bitmap, 1, 1, disk);
                for(int j = 1; j > -1; j--){
                    uint16_t bitmap2 = bitmap;
                    fprintf(stderr, "%x", (bitmap2>>j*4)&0x0F);
                    ocupied += (bitmap2>>j*4)&0x0F;
                    free = 128*1024 - ocupied;
                }

                if(i%2){
                    fprintf(stderr, "\n");
                }
            }
            fprintf(stderr, "ocupied: %u\n", ocupied);
            fprintf(stderr, "free: %u\n", free);
        }
        else{
            printf("El bloque bitmap ingresado es invalido");
        }

    }
    else{
        uint16_t bitmap;
        if((130 > block) && (block > 0))
        {
            fseek(disk, 1024*(block - 1), SEEK_CUR);
            for(int i = 0; i < 1024; i++){
                fread(&bitmap, 1, 1, disk);

                for(int j = 7; j > -1; j--){
                    uint16_t bitmap2 = bitmap;
                    fprintf(stderr, "%d", (bitmap2>>j)&1);
                }

                if(i%2){
                    fprintf(stderr, "\n");
                }
            }
        }
        /* Bloque directorio */
        else if(block == 0)
        {
            for(uint32_t i = 0; i < 1024*128; i++){
                fread(&bitmap, 1, 1, disk);

                for(int j = 7; j > -1; j--){
                    uint16_t bitmap2 = bitmap;
                    fprintf(stderr, "%d", (bitmap2>>j)&1);
                    if((bitmap2>>j)&1) ocupied++;
                    else free++;
                }
                if(i%2){
                    fprintf(stderr, "   %u \n", i/2);
                }
            }
            fprintf(stderr, "ocupied: %u\n", ocupied);
            fprintf(stderr, "free: %u\n", free);
        }
        else{
            printf("El bloque bitmap ingresado es invalido");
        }
    }
    fclose(disk);
}


uint32_t get_pointer(FILE* disk){
	uint32_t pointer;
	uint32_t result = 0;
	for(int i = 0; i < 4; i++)
	{
		fread(&pointer, sizeof(uint8_t), 1, disk);
		result += ((pointer>>4)&(0xF)) * pow(16, 2*(3-i)+1) + ((pointer)&(0xF)) * pow(16, 2*(3-i));
	}
	return result;
}

void write_pointer(FILE* pos, uint32_t pointer)
{
    uint8_t final;
	for(int i = 3; i > -1; i--)
	{
        final = (pointer>>8*i)&0xFF;
		fwrite(&final, 1, 1, pos);
	}
}

void write_name(FILE* disk, char* name)
{
    int zero = 27 - strlen(name);
    int aux = 0;
    for(int i = 0; i < strlen(name); i++){
        fwrite(&name[i], 1, 1, disk);
    }
    for(int i = 0; i < zero; i++)
    {
        fwrite(&aux, 1, 1, disk);
    }
}

void write_zero(FILE* disk)
{
    int aux = 0;
    for(int i = 0; i < 27; i++)
    {
        fwrite(&aux, 1, 1, disk);
    }
}


int cr_exists_recur(FILE* disk, char** path, int len, uint32_t my_dir, int from){
    for(int i = 0; i < 31; i++){
        uint8_t valid;
	    uint8_t name[27];
	    uint32_t pointer;
        fread(&valid, sizeof(uint8_t), 1, disk);
        fread(&name, sizeof(uint8_t)*27, 1, disk);
        uint32_t direction = get_pointer(disk);

        if(!strcmp(name, path[from]))
        {
            // printf("len: %d\n", len);
            if(len == 1) return 1;
            if(valid == 2 || valid == 8 || valid == 16)
            {
                fseek(disk, direction*1024, SEEK_SET);
                if(cr_exists_recur(disk, path, len-1, direction, from + 1)) return 1;
                fseek(disk, my_dir*1024 + 32*(i+1), SEEK_SET);
            }
        }
    }
    uint8_t valid;
	uint8_t name[27];
	uint32_t pointer;
    fread(&valid, sizeof(uint8_t), 1, disk);
    fread(&name, sizeof(uint8_t)*27, 1, disk);
    uint32_t direction = get_pointer(disk);
    if(valid == 32){
        fseek(disk, direction*1024, SEEK_SET);
        cr_exists_recur(disk, path, len, direction, from);
        fseek(disk, (my_dir + 1)*1024, SEEK_SET);
    }
    return 0;
}

uint32_t cr_exists_direction_recur(FILE* disk, char** path, int len, uint32_t my_dir, int from){
    for(int i = 0; i < 31; i++){
        uint8_t valid;
	    uint8_t name[27];
	    uint32_t pointer;
        fread(&valid, sizeof(uint8_t), 1, disk);
        fread(&name, sizeof(uint8_t)*27, 1, disk);
        uint32_t direction = get_pointer(disk);

        if(!strcmp(name, path[from]))
        {
            if(len == 1) return direction;
            if(valid == 2 || valid == 8 || valid == 16)
            {
                fseek(disk, direction*1024, SEEK_SET);
                int pos_dir = cr_exists_direction_recur(disk, path, len-1, direction, from + 1);
                if(pos_dir) return pos_dir;
                fseek(disk, my_dir*1024 + 32*(i+1), SEEK_SET);
            }
        }
    }
    uint8_t valid;
	uint8_t name[27];
	uint32_t pointer;
    fread(&valid, sizeof(uint8_t), 1, disk);
    fread(&name, sizeof(uint8_t)*27, 1, disk);
    uint32_t direction = get_pointer(disk);
    if(valid == 32){
        fseek(disk, direction*1024, SEEK_SET);
        int pos_dir = cr_exists_direction_recur(disk, path, len, direction, from);
        if(pos_dir) return pos_dir;
        fseek(disk, (my_dir + 1)*1024, SEEK_SET);
    }
    return 0;
}

uint32_t cr_exists_direction(char* path){
    /*Funcion para ver si un archivo o carpeta existe en la ruta especificada porpath. Retorna 1 si el archivo o carpeta existe y 0 en caso contrario. */
    FILE *disk = fopen(DISKNAME, "rb");
    char **dir_arr = calloc(20, sizeof(char*));
    for(int i = 0; i < 20; i++){
        dir_arr[i] = calloc(20, sizeof(char));
    }
    char *dirname = malloc(strlen(path)+1);
    strcpy(dirname, path);
    int val = 0;
    char *stok = strtok(dirname, "/");
    while(stok != NULL && val < 20){
        strcpy(dir_arr[val], stok);
        stok = strtok(NULL, "/");
        val++;
    }
    uint32_t returnado = cr_exists_direction_recur(disk, dir_arr, val, 0, 0);
    free(dirname);
    for(int i = 0; i < 20; i++)
    {
        free(dir_arr[i]);
    }
    free(dir_arr);
    fclose(disk);

    return returnado;

}

int cr_exists(char* path){
    /*Funcion para ver si un archivo o carpeta existe en la ruta especificada porpath. Retorna 1 si el archivo o carpeta existe y 0 en caso contrario. */
    FILE *disk = fopen(DISKNAME, "rb");
    char **dir_arr = calloc(20, sizeof(char*));
    for(int i = 0; i < 20; i++){
        dir_arr[i] = calloc(20, sizeof(char));
    }
    char *dirname = malloc(strlen(path)+1);
    strcpy(dirname, path);
    int val = 0;
    char *stok = strtok(dirname, "/");
    while(stok != NULL && val < 20){
        strcpy(dir_arr[val], stok);
        stok = strtok(NULL, "/");
        val++;
    }
    int returnado = cr_exists_recur(disk, dir_arr, val, 0, 0);
    free(dirname);
    for(int i = 0; i < 20; i++)
    {
        free(dir_arr[i]);
    }
    free(dir_arr);
    fclose(disk);

    return returnado;


}

void print_dir(FILE* disk, uint32_t dir)
{
    fseek(disk, dir*1024, SEEK_SET);
    for(int i = 0; i < 31; i++){
        uint8_t valid;
	    uint8_t name[27];
	    uint32_t pointer;
        fread(&valid, sizeof(uint8_t), 1, disk);
        fread(&name, sizeof(uint8_t)*27, 1, disk);
        uint32_t direction = get_pointer(disk);

        if(valid == 2 || valid == 4 || valid == 16 || valid == 8) printf("%s\n", name);
    }
    uint8_t valid;
	uint8_t name[27];
	uint32_t pointer;
    fread(&valid, sizeof(uint8_t), 1, disk);
    fread(&name, sizeof(uint8_t)*27, 1, disk);
    uint32_t direction = get_pointer(disk);
    // printf("value: %d, name: %s, dir: %u\n", valid, name, direction);
    if(valid == 32){
        print_dir(disk, direction);
    }
}

void cr_ls_recur(FILE* disk, char** path, int len, uint32_t my_dir, int from)
{
    // printf("len %d, from: %d\n", len, from);
    for(int i = 0; i < 31; i++){
        uint8_t valid;
	    uint8_t name[27];
	    uint32_t pointer;
        fread(&valid, sizeof(uint8_t), 1, disk);
        fread(&name, sizeof(uint8_t)*27, 1, disk);
        uint32_t direction = get_pointer(disk);

        // printf("name: %s\n", valid, name, direction);
        if(!strcmp(name, path[from]))
        {
            // printf("entre %d\n", valid);
            if(len == 1) print_dir(disk, direction);
            if(valid == 2 || valid == 8 || valid == 16)
            {
                fseek(disk, direction*1024, SEEK_SET);
                cr_ls_recur(disk, path, len-1, direction, from + 1);
                // fseek(disk, my_dir*1024 + 32*(i+1), SEEK_SET);
                // printf("he vuelto %u\n", my_dir);
            }
        }
    }
    uint8_t valid;
	uint8_t name[27];
	uint32_t pointer;
    fread(&valid, sizeof(uint8_t), 1, disk);
    fread(&name, sizeof(uint8_t)*27, 1, disk);
    uint32_t direction = get_pointer(disk);
    // printf("value: %d, name: %s, dir: %u\n", valid, name, direction);
    if(valid == 32){
        fseek(disk, direction*1024, SEEK_SET);
        cr_exists_recur(disk, path, len, direction, from);
        fseek(disk, (my_dir + 1)*1024, SEEK_SET);
    }
}


void cr_ls(char* path){
    /*Funcion para listar los elementos de un directorio del disco. Imprime en pan-talla los nombres de todos los archivos y directorios contenidos en el directorio indicado porpath. */
    if(cr_exists(path))
    {
        FILE *disk = fopen(DISKNAME, "rb");
        char **dir_arr = calloc(20, sizeof(char*));
        for(int i = 0; i < 20; i++){
            dir_arr[i] = calloc(20, sizeof(char));
        }
        char *dirname = malloc(strlen(path)+1);
        strcpy(dirname, path);
        int val = 0;
        char *stok = strtok(dirname, "/");
        while(stok != NULL && val < 20){
            strcpy(dir_arr[val], stok);
            stok = strtok(NULL, "/");
            val++;
        }
        cr_ls_recur(disk, dir_arr, val, 0, 0);
        free(dirname);
        for(int i = 0; i < 20; i++)
        {
            free(dir_arr[i]);
        }
        free(dir_arr);
        fclose(disk);
    }
}


int unload_file(char* orig, char* dest){
    printf("Downloading ... origin [%s] destiny [%s]\n", orig, dest);
	char mode = 'r';
	crFILE* file_desc = cr_open(orig, mode);
    FILE* destiny = fopen(dest, "wb");
    if (destiny){

        int nbytes = 512;
        unsigned char* buffer[nbytes];
        int bytes_readed = nbytes;
        while (bytes_readed == nbytes)
        {
            bytes_readed = cr_read(file_desc, buffer, nbytes);
            fwrite(buffer, 1, bytes_readed, destiny);
        }
        fclose(destiny);
    }
    else{
        printf("Error de path de destino %s", dest);
    }
    cr_close(file_desc);
    printf("End download\n");
    return 0;
}

int make_dir_recur(FILE* disk, uint32_t dir, char* origin ,char* dest)
{
    // printf("%s\n", dest);
    fseek(disk, dir*1024, SEEK_SET);
    for(int i = 0; i < 31; i++){
        uint8_t valid;
	    uint8_t name[27];
	    uint32_t pointer;
        fread(&valid, sizeof(uint8_t), 1, disk);
        fread(&name, sizeof(uint8_t)*27, 1, disk);
        uint32_t direction = get_pointer(disk);
        if(strcmp(name, "")){
            char* new_dest = calloc(strlen(dest) + strlen(name) + 2, sizeof(char));
            char* new_origin = calloc(strlen(origin) + strlen(name) + 2, sizeof(char));
            strcpy(new_dest, dest);
            strcat(new_dest, "/");
            strcat(new_dest, name);
            strcpy(new_origin, origin);
            strcat(new_origin, "/");
            strcat(new_origin, name);
            printf("%s\n", new_dest);
            if(valid == 2)
            {
                mkdir(new_dest, 0777);
                make_dir_recur(disk, direction, new_origin, new_dest);
                fseek(disk, dir*1024 + 32*(i + 1), SEEK_SET);
            }
            if(valid == 4)
            {
                // some_func();
                // printf("Downloading... origin: [%s] destiny [%s]\n", name, new_dest);
                unload_file(new_origin, new_dest);
            }
            free(new_dest);
            free(new_origin);
        }
    }
    uint8_t valid;
	uint8_t name[27];
	uint32_t pointer;
    fread(&valid, sizeof(uint8_t), 1, disk);
    fread(&name, sizeof(uint8_t)*27, 1, disk);
    uint32_t direction = get_pointer(disk);
    if(valid == 32){
        make_dir_recur(disk, direction, origin, dest);
    }
    // free(new_dest);
    return 1;
}


int parse_dir_recur(FILE* disk, char** path, int len, uint32_t my_dir, int from, char* origin,char* dest)
{
    // printf("len %d, from: %d\n", len, from);
    for(int i = 0; i < 31; i++){
        uint8_t valid;
	    uint8_t name[27];
	    uint32_t pointer;
        fread(&valid, sizeof(uint8_t), 1, disk);
        fread(&name, sizeof(uint8_t)*27, 1, disk);
        uint32_t direction = get_pointer(disk);

        // printf("name: %s\n", valid, name, direction);
        if(!strcmp(name, path[from]))
        {
            // printf("entre %d\n", valid);
            if(len == 1) {
                if(valid == 4)
                {
                    char* new_dest = calloc(strlen(dest) + strlen(name) + 2, sizeof(char));
                    strcpy(new_dest, dest);
                    strcat(new_dest, "/");
                    strcat(new_dest, path[len + from - 1]);
                    unload_file(origin, new_dest);
                    free(new_dest);
                    return 1;
                }

                int returnado = make_dir_recur(disk, direction, origin, dest);
                return returnado;
            }
            if(valid == 2 || valid == 8 || valid == 16)
            {
                fseek(disk, direction*1024, SEEK_SET);
                if(parse_dir_recur(disk, path, len-1, direction, from + 1, origin, dest)) {
                    return 1;
                }
                // fseek(disk, my_dir*1024 + 32*(i+1), SEEK_SET);
                // printf("he vuelto %u\n", my_dir);
            }
        }
    }
    uint8_t valid;
	uint8_t name[27];
	uint32_t pointer;
    fread(&valid, sizeof(uint8_t), 1, disk);
    fread(&name, sizeof(uint8_t)*27, 1, disk);
    uint32_t direction = get_pointer(disk);
    // printf("value: %d, name: %s, dir: %u\n", valid, name, direction);
    if(valid == 32){
        fseek(disk, direction*1024, SEEK_SET);
        parse_dir_recur(disk, path, len, direction, from, origin, dest);
    }
    return 0;
}


uint32_t reserve_unused_block()
{
    FILE* disk = fopen(DISKNAME, "rb+");
    uint16_t bitmap;
    uint32_t dir = 0;
    uint32_t dir2 = 0;
    uint8_t to_write;
    int value;
    int ready = 0;
    fseek(disk, 1024, SEEK_SET);
    for(uint32_t i = 0; i < 1024*128; i++){
        if(ready) break;
        fread(&bitmap, 1, 1, disk);

        for(int j = 7; j > -1; j--){
            value = (bitmap>>j)&1;
            if(value == 0){
                dir = 8*(i) + 7-j;
                dir2 = i;
                to_write = bitmap | (int)(1*pow(2, j));
                // printf("%u %u, %d\n", to_write, bitmap, dir);
                ready = 1;
                break;
            }
        }
    }
    if(to_write)
    {
        fseek(disk, 1024 + dir2, SEEK_SET);
        fwrite(&to_write, 1, 1, disk);
    }
    fclose(disk);
    // printf("Direccion: %u\n", dir);
    return dir;
}

void free_used_block(uint32_t dir)
{
    FILE* disk = fopen(DISKNAME, "rb+");
    uint16_t bitmap = 0;
    uint32_t dir2 = 0;
    uint16_t to_write = 0;
    int value;
    int ready = 0;
    fseek(disk, 1024, SEEK_SET);
    for(uint32_t i = 0; i < 1024*128; i++){
        if(ready) break;
        fread(&bitmap, 1, 1, disk);

        for(int j = 7; j > -1; j--){
            value = (bitmap>>j)&1;
            // printf("%d, %u\n", value, (i*8)+7-j);
            if(value && (i*8)+7-j == dir){
                dir2 = i;
                to_write = bitmap & (255 - (int)pow(2, j));
                // printf("%u %u, %d\n", to_write, bitmap, dir2);
                ready = 1;
                break;
            }
            else
            {
                to_write = bitmap;
            }

        }
    }
    fseek(disk, 1024 + dir2, SEEK_SET);
    // printf("%u, %u, %u\n", to_write, dir2, dir);
    fwrite(&to_write, 1, 1, disk);
    fclose(disk);
    // printf("Direccion: %u\n", dir);
}

uint32_t create_new_dir_cont(FILE* disk, uint32_t dir, char* name){
    // printf("eso si wea\n");
    fseek(disk, -32, SEEK_CUR);
    int new_valid = 32;
    uint32_t new_block = reserve_unused_block();
    fwrite(&new_valid, 1, 1, disk);
    write_name(disk, name);
    write_pointer(disk, new_block);
    // printf("%d\n", new_block);
    return new_block;
}

void write_dir_father(FILE* disk, uint32_t dir, uint32_t father_dir)
{
    int father = 16;
    int me = 8;
    fseek(disk, dir*1024, SEEK_SET);
    fwrite(&father, 1, 1, disk);
    write_name(disk, "..");
    write_pointer(disk, father_dir);
    fwrite(&me, 1, 1, disk);
    write_name(disk, ".");
    write_pointer(disk, dir);

}

int get_pos_on_directory(FILE* disk, uint32_t dir, char* dirname, char* my_name){
    fseek(disk, dir*1024, SEEK_SET);
    // printf("%u\n", dir);
    for(int i = 0; i < 31; i++){
        uint8_t valid;
	    uint8_t name[27];
	    uint32_t pointer;
        fread(&valid, sizeof(uint8_t), 1, disk);
        fread(&name, sizeof(uint8_t)*27, 1, disk);
        uint32_t direction = get_pointer(disk);

        // printf("value: %d, name: %s, dir: %u\n", valid, name, direction);
        if(valid != 2 && valid != 4 && valid != 8)
        {
            if(valid != 16 && valid != 32)
            {
                uint8_t new_valid = 2;
                uint32_t new_pointer = reserve_unused_block();
                fseek(disk, -32, SEEK_CUR);
                fwrite(&new_valid, 1, 1, disk);
                write_name(disk, dirname);
                write_pointer(disk, new_pointer);
                write_dir_father(disk, new_pointer, dir);
                return 1;

            }
        }
    }
    uint8_t valid;
	uint8_t name[27];
    uint32_t pointer;
    fread(&valid, sizeof(uint8_t), 1, disk);
    fread(&name, sizeof(uint8_t)*27, 1, disk);
    uint32_t direction = get_pointer(disk);
    if(valid == 32)
    {
        return get_pos_on_directory(disk, direction, dirname, my_name);
    }
    else
    {
        direction = create_new_dir_cont(disk, dir, my_name);
        return get_pos_on_directory(disk, direction, dirname, my_name);
    }

    return 0;
}

int cr_mkdir(char* foldername)
{
    if(cr_exists(foldername)) return 0;
    /* Funcion para crear directorios. Crea el directorio vacio referido por foldername. */
    FILE* disk = fopen(DISKNAME, "rb+");
    char *dirname = malloc(strlen(foldername)+1);
    char *aux = "";
    strcpy(dirname, aux);
    char *dirname2 = malloc(strlen(foldername) + 1);
    strcpy(dirname2, foldername);
    char *stok = strtok(dirname2, "/");
    char *copy = malloc(strlen(foldername) + 1);
    char *copy2 = malloc(strlen(foldername) + 1);
    strcpy(copy, "..");
    while(stok != NULL){
        strcpy(copy2, copy);
        strcpy(copy, stok);
        // // printf("%s\n", stok);
        stok = strtok(NULL, "/");
        if(stok != NULL)
        {
            strcat(dirname, "/");
            strcat(dirname, copy);
        }
    }
    if(cr_exists(dirname) || !strcmp(copy2, "..")){
        uint32_t direction = cr_exists_direction(dirname);
        int final = get_pos_on_directory(disk, direction, copy, copy2);
        free(dirname);
        free(dirname2);
        free(copy);
        free(copy2);
        fclose(disk);
        return final;

    }
    free(dirname);
    free(dirname2);
    free(copy2);
    free(copy);
    fclose(disk);
    return 0;
}


/* # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # */

/* FUNCIONES DE MANEJO DE ARCHIVOS */
crFILE* cr_open(char* path, char mode){
    /* Función para abrir un archivo. Si mode es‘r’, busca el archivo en la rutapathy retorna uncrFILE*que lo representa. Simodees‘w’, se verifica que el archivono exista en la ruta especificada y se retorna un nuevocrFILE*que lo representa.*/
    crFILE* crfile = malloc(sizeof(crFILE));
    int direction = cr_exists_direction(path);
    crfile -> pos_data = 0;
    crfile -> overright = 0;
    crfile -> block = direction;
    crfile ->bytes_readed = 0;
    crfile->mode = mode == 'w';
    crfile->path = malloc(strlen(path) + 1);
    strcpy(crfile->path, path);
    if (crfile->mode){
        FILE* local = fopen(path, "wb");
        fseek(local, 0L, SEEK_END);
        crfile->size = ftell(local);
        if (direction){
            crfile-> overright = 1;
        }
    }
    else{
        if (direction){
            FILE* disk = fopen(DISKNAME, "rb");
            fseek(disk, crfile->block*1024, SEEK_SET);
            crfile->size = get_pointer(disk);
            crfile -> pos_index = 4;
            fclose(disk);
            return crfile;
        }
        else
        {
            printf("cr_open: Path invalido\n");
            free(crfile->path);
            free(crfile);
            return NULL;
        }

    }
}

int read(crFILE* file_desc, void* buffer, uint32_t pointer, int byte){
    FILE* disk = fopen(DISKNAME, "rb");
    fseek(disk, pointer*BLOCK_SIZE + file_desc->pos_data, SEEK_SET);
    fread(buffer + byte, 1, 1, disk);
    file_desc->bytes_readed++;
    file_desc->pos_data++;
    fclose(disk);
    return 0;
}


int cr_read(crFILE* file_desc, void* buffer, int nbytes){
    /* Función para leer archivos. Leelos siguientes n bytes desde el archivo descrito por file_desc y los guarda en la
    dirección apuntada por buffer. Debe retornar la cantidad de Byte efectivamente leidos desde el archivo.
    Esto es importante si nbytes es mayor a la cantidad de Byte restantes en el archivo.
    La lectura dereadse efectúa desde la posicióndel archivo inmediatamente posterior a la última
    posición leída por un llamado a read.*/
    FILE* disk = fopen(DISKNAME, "rb");
    fseek(disk, (file_desc->block)*BLOCK_SIZE + file_desc->pos_index, SEEK_SET);
    uint32_t pointer = get_pointer(disk);
    fseek(disk, pointer*BLOCK_SIZE + file_desc->pos_data, SEEK_SET);
    int byte = 0;
    while (file_desc->pos_index < IDX_PTRS+4)
    {
        if (file_desc->bytes_readed >= file_desc->size){
            fclose(disk);
            return byte;
        }
        read(file_desc, buffer, pointer, byte);
        byte ++;
        if (file_desc->bytes_readed % BLOCK_SIZE == 0){
            file_desc->pos_data = 0;
            file_desc->pos_index += 4;
            fseek(disk, (file_desc->block)*BLOCK_SIZE + file_desc->pos_index, SEEK_SET);
            pointer = get_pointer(disk);
            fseek(disk, pointer*BLOCK_SIZE, SEEK_SET);
        }
        if (byte == nbytes)
        {
            fclose(disk);
            return byte;
        }
    }
    // fseek(disk, (file_desc->block)*BLOCK_SIZE + 1012, SEEK_SET);
    // pointer = get_pointer(disk);
    // file_desc->block = pointer;
    // file_desc->pos_data = 0;
    // file_desc->pos_index = 0;
    // cr_read(file_desc, buffer, nbytes);
    fclose(disk);
    return byte;
}

int cr_write(crFILE* file_desc, void* buffer, int nbytes){
    /* Función para escribir archivos. Escribe en el archivo descrito por file_desc los nbytes que se encuentren en
    la dirección indicada por buffer. Retorna la cantidad de Byte escritos en el archivo. Si se produjo un error
    porque no pudo seguirescribiendo, ya sea porque el disco se llenó o porque el archivo no puede crecer m ́as,
    este número puede ser menor a nbytes(incluso 0).*/
    return 0;
}

int cr_close(crFILE* file_desc){
    /* Función para cerrar archivos.  Cierra el archivo  indicado  por filedesc. Debe garantizar que cuando esta función retorna, el archivo se encuentra actualizado en disco.*/
    free(file_desc->path);
    free(file_desc);
    return 0;
}

void rm_index(FILE* disk, uint32_t dir)
{
    fseek(disk, dir*1024, SEEK_SET);
    uint32_t file_size = get_pointer(disk);
    int size = 251;
    if((file_size-1)/1024 <= 251) size = (file_size-1)/1024;
    for(int i = 0; i <= size; i++)
    {
        uint32_t pointer = get_pointer(disk);
        free_used_block(pointer);
    }
    if((file_size-1)/1024 > 251)
    {
        uint32_t new_pointer = get_pointer(disk);
        fseek(disk, new_pointer*1024, SEEK_SET);
        size = 255;
        if((file_size-1)/1024 <= 507) size = ((file_size-1)/1024) - 252;
        for(int i = 0; i <= size; i++)
        {
            uint32_t pointer = get_pointer(disk);
            free_used_block(pointer);
        }
        free_used_block(new_pointer);
        //doble
        if((file_size-1)/1024 > 507)
        {
            fseek(disk, dir*1024 + 1016, SEEK_SET);
            uint32_t second_pointer = get_pointer(disk);
            int nivel = 0;
            uint32_t aux = file_size - 252 - 256;
            while(aux)
            {
                fseek(disk, second_pointer*1024 + nivel*4, SEEK_SET);
                new_pointer = get_pointer(disk);
                nivel++;
                fseek(disk, new_pointer, SEEK_SET);
                int nivel2 = 256;
                while(aux && nivel2)
                {
                    uint32_t second_level_pointer = get_pointer(disk);
                    nivel2--;
                    free_used_block(second_level_pointer);
                    aux--;
                }
                free_used_block(new_pointer);

            }
            free_used_block(second_pointer);

            //triple
            if((file_size-1)/1024 > 66043)
            {
                fseek(disk, dir*1024 + 1020, SEEK_SET);
                uint32_t thrid_pointer = get_pointer(disk);
                int nivel3 = 0;
                uint32_t aux = file_size - 252 - 256 - (256*256);
                while (aux)
                {
                    fseek(disk, thrid_pointer*1024 + nivel3*4, SEEK_SET);
                    uint32_t second_pointer = get_pointer(disk);
                    nivel3++;
                    int nivel = 0;
                    int aux_nivel = 256;
                    while(aux && aux_nivel)
                    {
                        fseek(disk, second_pointer*1024 + nivel*4, SEEK_SET);
                        new_pointer = get_pointer(disk);
                        aux_nivel--;
                        nivel++;
                        fseek(disk, new_pointer, SEEK_SET);
                        int nivel2 = 256;
                        while(aux && nivel2)
                        {
                            uint32_t second_level_pointer = get_pointer(disk);
                            nivel2--;
                            free_used_block(second_level_pointer);
                            aux--;
                        }
                        free_used_block(new_pointer);
                    }
                    free_used_block(second_pointer);
                }
                free_used_block(thrid_pointer);
            }

        }

    }
}

int change_dir_file(FILE* disk, uint32_t dir, char* filename)
{
    fseek(disk, dir*1024, SEEK_SET);
    printf("dir: %u, file_name: %s\n", dir, filename);
    for(int i = 0; i < 31; i++){
        uint8_t valid;
	    uint8_t name[27];
        fread(&valid, sizeof(uint8_t), 1, disk);
        fread(&name, sizeof(uint8_t)*27, 1, disk);
        uint32_t direction = get_pointer(disk);
        if(valid == 4 && !strcmp(name, filename)){
            int cero = 0;
            uint32_t new_dir = 0;
            fseek(disk, -32, SEEK_CUR);
            fwrite(&cero, 1, 1, disk);
            write_zero(disk);
            write_pointer(disk, new_dir);
            rm_index(disk, direction);
            free_used_block(direction);
            return 1;
        }
    }
    return 0;
}


int cr_rm(char* path){
    /* Función para borrar archivos. Elimina el archivo referenciado por la ruta path del directorio correspondiente. Los bloques que estaban siendo usados por el archivo deben quedar libres.*/
    if(!cr_exists(path)) return 0;
    FILE* disk = fopen(DISKNAME, "rb+");
    char *dirname = malloc(strlen(path)+1);
    char *aux = "";
    strcpy(dirname, aux);
    char *dirname2 = malloc(strlen(path) + 1);
    strcpy(dirname2, path);
    char *stok = strtok(dirname2, "/");
    char *copy = malloc(strlen(path) + 1);
    strcpy(copy, "..");
    while(stok != NULL){
        strcpy(copy, stok);
        // // printf("%s\n", stok);
        stok = strtok(NULL, "/");
        if(stok != NULL)
        {
            strcat(dirname, "/");
            strcat(dirname, copy);
        }
    }
    uint32_t direction = cr_exists_direction(dirname);
    int final = change_dir_file(disk, direction, copy);
    free(dirname);
    free(dirname2);
    free(copy);
    fclose(disk);
    return final;
}


int cr_unload(char* orig, char* dest){
    /*Funcion para listar los elementos de un directorio del disco. Imprime en pan-talla los nombres de
    todos los archivos y directorios contenidos en el directorio indicado por path. */
    if(cr_exists(orig))
    {
        FILE *disk = fopen(DISKNAME, "rb");
        char **dir_arr = calloc(20, sizeof(char*));
        for(int i = 0; i < 20; i++){
            dir_arr[i] = calloc(20, sizeof(char));
        }
        char *dirname = malloc(strlen(orig)+1);
        strcpy(dirname, orig);
        int val = 0;
        char *stok = strtok(dirname, "/");
        while(stok != NULL && val < 20){
            strcpy(dir_arr[val], stok);
            stok = strtok(NULL, "/");
            val++;
        }
        parse_dir_recur(disk, dir_arr, val, 0, 0, orig, dest);
        free(dirname);
        for(int i = 0; i < 20; i++)
        {
            free(dir_arr[i]);
        }
        free(dir_arr);
        fclose(disk);
        return 1;
    }
    return 0;
}

int cr_load(char* orig){
//     /* Función que se encarga de copiar un archivo o ́arbol de directorios,
//     referenciado por origal disco. En caso de que un archivo sea demasiado
//     pesado para el disco, se debe escribir todo lo posible hasta acabar el
//     espacio disponible.*/

    return 0;
}

