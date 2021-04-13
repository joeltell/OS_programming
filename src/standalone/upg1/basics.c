#include <stdio.h>
#include <string.h>
int main(int argc,char * argv[])
{

  int pad=0;
  int longest=20;
  float totsize = 0;
  for (int i = 0;i<argc;++i){
    pad = longest-strlen(argv[i]);

      printf("%-s %*ld  \n",argv[i],pad,strlen(argv[i]));
      totsize = totsize + strlen(argv[i]);
  }
  pad=longest-strlen("total length");
  printf("Total lenght %*.0f \n",pad,totsize);
  float average = totsize/argc;
  pad=longest-strlen("average length");
  printf("Average length %*.2f \n",pad,average);
  return 0;
}
