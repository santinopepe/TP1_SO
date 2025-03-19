#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int toNum(char * str){
    int num = 0;
    for(int i = 0; str[i] != '\0'; i++){
        num = num * 10 + str[i] - '0';
    }
    return num;
}

int main(int argc, char * argv[]){

    if(argc != 3){
        printf("Uso: %s <ancho> <alto>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    int ancho = toNum(argv[1]);
    int alto = toNum(argv[2]);




    for(int i = 0; i < alto; i++){
        for(int j = 0; j < ancho; j++){
            printf("%d ", rand() % 9 + 1);
        }
        printf("\n");
    }



    return 0;


}