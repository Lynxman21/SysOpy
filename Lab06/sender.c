#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>

int main() {
    unlink("./tmp/pipe");
    unlink("./tmp/result");

    double start, end, res;

    printf("Podaj start\n");
    scanf("%lf",&start);
    printf("Podaj koniec\n");
    scanf("%lf",&end);

    mkfifo("./tmp/pipe",0777);
    mkfifo("./tmp/result",0777);

    FILE *fd = fopen("./tmp/pipe","w");

    fwrite(&start,sizeof(double),1,fd);
    fwrite(&end,sizeof(double),1,fd);
    fclose(fd);

    FILE *res_pipe = fopen("./tmp/result","r");
    fread(&res,sizeof(double),1,res_pipe);

    fclose(res_pipe);
    printf("wynik: %lf",res);
    return 0;
}