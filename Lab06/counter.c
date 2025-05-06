#include<stdio.h>

double function(double x) {
    return 4/(x*x + 1);
}

double integral(double (*func)(double),double start,double end,double dx) {
    double res = 0.0;
    double x = start;
    int steps = (int)((end-start)/dx);

    for (int i=0;i<steps;i++) {
        res += dx*func(x);
        x += dx;
    }

    double reminder = end-x;
    if (reminder > 0.0) {
        res += reminder*func(x);
    }

    return res;
}

int main() {
    double start, end, res;
    FILE *data = fopen("./tmp/pipe","r");
    fread(&start,sizeof(double),1,data);
    fread(&end,sizeof(double),1,data);

    res = integral(function,start,end,0.0001);

    FILE *res_pipe = fopen("./tmp/result","w");
    fwrite(&res,sizeof(double),1,res_pipe);
    fclose(res_pipe);

    return 0;
}