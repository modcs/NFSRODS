#include <stdio.h>

#include "rodscapi.h"

int main(void)
{
    printf("Hello World!\n");

    rcComm_t* comm = rodsConnect();

    int i = rodsLogin(comm);

   //putFile("/home/emc/irodscapi.tar.gz", "/tempZone/home/rods/irodscapi.tar.gz", comm);

   // getFile("/home/emc/xaxaxa.tar.gz", "/tempZone/home/rods/irodscapi.tar.gz",FALSE, FALSE, comm);

    //removeObj("/tempZone/home/rods/irodscapi.tar.gz", comm);

    listCollection("/tempZone/home/rods", comm);

    //testRead(comm);

    return 0;
}


