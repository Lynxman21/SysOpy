#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

int is_txt(char *file_name) {
    char *dot = strrchr(file_name,'.');

    if (dot != NULL && strcmp(dot,".txt") == 0) {
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Zła ilość argumentów");
        return 1;
    }

    DIR* input = opendir(argv[1]);

    if (input == NULL) {
        printf("Folder nie istnieje");
        return 1;
    }

    if (mkdir(argv[2],0777) == -1) {
        printf("Folder już istnieje");
        closedir(input);
        return 1;
    }

    DIR* output = opendir(argv[2]);

    struct dirent* curr_f;
    char full_path_read[1024];
    char full_path_write[1024];
    FILE *file_in;
    FILE *file_out;

    while ((curr_f = readdir(input))!=NULL) {
        if (is_txt(curr_f->d_name) == 0 || strcmp(curr_f->d_name,".") == 0 || strcmp(curr_f->d_name,"..") == 0) {
            continue;
        }

        snprintf(full_path_read,sizeof(full_path_read),"%s/%s",argv[1],curr_f->d_name);
        file_in = fopen(full_path_read,"r");
        if (file_in == NULL) {
            printf("Nie można otworzyć pliku");
            curr_f = readdir(input);
            continue;
        }


        snprintf(full_path_write,sizeof(full_path_write),"%s/%s",argv[2],curr_f->d_name);
        file_out = fopen(full_path_write,"w");
        if (file_out == NULL) {
            printf("Nie można otworzyć pliku");
            fclose(file_in);
            curr_f = readdir(input);
            continue;
        }

        char *line = NULL;
        size_t size_line = 0;
        ssize_t len;

        while ((len = getline(&line,&size_line,file_in)) != -1) {

            if (line[len-1] == '\n') {
                len--;
            }

            for (size_t i=0;i<len/2;i++) {
                char temp = line[len-i-1];
                line[len-i-1] = line[i];
                line[i] = temp;
            }

            fwrite(line,1,len,file_out);
            fwrite("\n",1,1,file_out);
        }
        
        fclose(file_in);
        fclose(file_out);
    }

    closedir(input);
    closedir(output);
}