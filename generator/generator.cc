#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <ctime>
using namespace std;

int tab[100][100];
int main(int argc, char* argv[]){
  srand(time(NULL));
  int N = atoi(argv[1]);
    
  printf("%d\n",N);
  
  for(int i = 0; i < N; ++i)
  {
    for(int j = 0; j <= i; ++j)
    {
      int value = 2 + (rand() % 100);
      if(i == j) value = 0;
      tab[i][j] = tab[j][i] = value;
    }
  }	
  
  
  for(int i = 0; i < N; ++i)
  {
    for(int j = 0; j < N; ++j)
    {
      printf("%d ", tab[i][j]);
    }
    printf("\n");
  }	
  printf("\n");
  return 0;
}
