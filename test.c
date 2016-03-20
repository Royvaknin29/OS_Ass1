#include "types.h"
#include "user.h"

int
main(int argc, char *argv[]){
    printf(1, "     Welcome to Testing File!!!\n");
    printf(1, "*************************************\n");
    char buffer[100] = "";
    history(buffer, 2);

    printf(1, buffer);

    exit();
}