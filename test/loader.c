#include <stdio.h>
#include <stdlib.h>

#include "cap_so.h"

int main(int argc, char **argv)
{
    printf("loading OK\n");

    if(test_caps(1))
    {
        printf("capabilities OK\n");
    }
    else
    {
        return 1;
    }

    return 0;
}