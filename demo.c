#include <unistd.h>
#include "fdpopen.h"

int main()
{
    int ls = fdpopen("ls -l","r");
    int more = fdpopen("more","w");
    char data[128];
    ssize_t len;
    do
    {
        len = read(ls,data,128);
        write(more,data,len);
    }while(len == 128);
     
    fdpclose(ls);
    fdpclose(more);
     
    return 0;
     
}
