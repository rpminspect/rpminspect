#include <stdio.h>
#include <string.h>
int main (int argc, char **argv)
{
    /* https://access.redhat.com/blogs/766093/posts/1976213 */
    char buffer[5];
    printf("Buffer Contains: %s , Size Of Buffer is %lu\n",
           buffer, sizeof(buffer));
    strcpy(buffer, argv[1]);
    printf("Buffer Contains: %s , Size of Buffer is %lu\n",
           buffer, sizeof(buffer));
    return 0;
}
