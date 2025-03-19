#include <stdlib.h>
#include <time.h>

int main(int argc, char * argv[]){
  srand(time(NULL));
  return rand() % 7 + 1;
}