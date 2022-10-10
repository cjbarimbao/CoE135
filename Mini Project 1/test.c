#include <string.h>
#include <stdio.h>

int main () {
   char buf[1024] = "123/12/1/3";
   char * token;
   puts(buf);
   /* get the first token */
   token = strtok(buf, "/");
   
   /* walk through other tokens */
   while( token != NULL ) {
      printf("%s\n", token);
      token = strtok(NULL, "/");
   }
   puts(buf);
   return(0);
}