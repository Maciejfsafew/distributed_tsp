#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <algorithm>
#include <iostream>
#include<time.h>
#include <unistd.h>

using namespace std;

// proc nr
int world_rank;
int world_size;

MPI::Status status;

int graph[30][30];
int N;
int BEST = 500;

int **queue;
int queueBegin = 0;
int queueEnd = 0;

int degree = 4;
int maxQueue = 800000;

int TASK[6];
unsigned int taskCount = 0;
long long nodes = 0;

int used[30];
int cut = 0;
void printQueue()
{
  for(int i = 0; i < queueEnd; ++i)
    printf("%d %d %d %d %d\n", queue[i][0], queue[i][1], queue[i][2], queue[i][3], queue[i][4]);  
}

void sendTask(int proc)
{
  queue[queueBegin][degree + 1] = BEST;
  MPI::COMM_WORLD.Send(queue[queueBegin], degree + 2, MPI_INT, proc, 0);
  if(queueBegin < queueEnd)
    queueBegin++;
  if(queueBegin % 400 == 0)
    printf("%d\n", queueBegin);
}

int getResult(int cost, int last, int first, int remaining)
{
	int res = 500;
	if(cost >= BEST) {cut++; return res;}
	if(remaining <= 0)
	{
		//BEST = min(BEST, cost + abs(graph[first][last]));
		return cost + abs(graph[first][last]);
	} 
	
	for(int i = 0; i < N; ++i)
	{
	  if(used[i] == 0 && graph[last][i] > 0)
	  {
	    used[i] = 1;
	    nodes++;
	    res = min(res, getResult(cost + abs(graph[last][i]), i, first, remaining - 1));
	    used[i] = 0;
	  }
	}
	return res;
}

int computeResult()
{
  int cost = TASK[0];
  for(int i = 0; i < 30; ++i) used[i] = 0;
  for(int i = 1; i < degree + 1; ++i)
    used[TASK[i]] = 1;
  return getResult(cost, TASK[degree], TASK[1], N - degree);
}

void processTask()
{
  while(1)
  {
    MPI::COMM_WORLD.Recv(&TASK, degree + 2, MPI_INT, 0, 0, status);
    taskCount = taskCount + 1;
    if(TASK[0] == -1)
      return;
    BEST = min(TASK[degree + 1], BEST);
    
    int result = computeResult();
    MPI::COMM_WORLD.Send(&result, 1, MPI_INT, 0, 0); 
  }
}

void cleanQueue()
{
	for(int i = 1; i < world_size; ++i)
	  sendTask(i);
	  
	int result = 500;
	int responses = 0;
	while(responses < queueEnd)
	{
      MPI::COMM_WORLD.Probe(MPI_ANY_SOURCE, 0, status);
      
      int source = status.Get_source();
      MPI::COMM_WORLD.Recv(&result, 1, MPI_INT, source, 0, status);
      if(result < BEST)
      {
		  BEST = result;
		  printf("Better solution: %d\n", result);
	  }
      sendTask(source);	
      responses++;
	}
}
void init()
{
	queue = new int*[maxQueue];
	for(int i = 0; i < maxQueue; ++i)
	  queue[i] = new int[degree + 2];

	for(int a = 0; a < N; ++a)
	for(int b = 0; b < N; ++b)
	for(int c = 0; c < N; ++c)
	for(int d = 0; d < N; ++d)
	if(a!=b&&a!=c&&a!=d&&b!=c&&b!=d&&c!=d)
	{
		queue[queueEnd][0] = graph[a][b] + graph[b][c] + graph[c][d];
		queue[queueEnd][1] = a;
		queue[queueEnd][2] = b;
		queue[queueEnd][3] = c;
		queue[queueEnd++][4] = d;
	}
	nodes = queueEnd;
	for(int i = 0; i < 5; i++)
	  queue[queueEnd][i] = -1;
}

void debug(){
  for(int i = 0; i < N; ++i){
    for(int j = 0; j < N; ++j)
      printf("%d ",graph[i][j]);
    printf("\n");
  }	
}

void rootBestSolution()
{		
  for(int p = 1; p < world_size; ++p) 
  {
	int solution = 500;
    MPI::COMM_WORLD.Recv(&solution, 1, MPI_INT, p, 0, status); 
    BEST = min(solution, BEST);   
  }
  for(int p = 1; p < world_size; ++p) 
    MPI::COMM_WORLD.Send(&BEST, 1, MPI_INT, p, 0); 
}

void workerBestSolution()
{
  MPI::COMM_WORLD.Send(&BEST, 1, MPI_INT, 0, 0);
  int solution = 500;
  MPI::COMM_WORLD.Recv(&solution, 1, MPI_INT, 0, 0, status); 
  BEST = min(solution, BEST);   
}

int main(int argc, char** argv) {
  MPI::Init();
  world_rank = MPI::COMM_WORLD.Get_rank();
  world_size = MPI::COMM_WORLD.Get_size();
  
  double t0 = MPI_Wtime();
  double t1 = 0;
  if(world_rank == 0)
  {
	scanf("%d", &N);
	for(int i = 0; i < N; ++i)
	for(int j = 0; j < N; ++j)
      scanf("%d", &graph[i][j]);
    
    for(int p = 1; p < world_size; ++p) 
      MPI::COMM_WORLD.Send(&N, 1, MPI_INT, p, 0); 
      
    for(int p = 1; p < world_size; ++p)
    for(int i = 0; i < N; ++i)
      MPI::COMM_WORLD.Send(&graph[i], N, MPI_INT, p, 0);
      
    rootBestSolution();
  }
  else 
  {
    MPI::COMM_WORLD.Recv(&N, 1, MPI_INT, 0, 0, status);
    
    for(int i = 0; i < N; ++i)
      MPI::COMM_WORLD.Recv(&graph[i], N, MPI_INT, 0, 0, status);
    
    workerBestSolution();
  }
  
  if(world_rank == 0)
  {
    init();
    printf("Queue lenght %d\n", queueEnd - queueBegin);
    cleanQueue();
  }
  else
  {
    processTask();
    printf("Proc: %d TaskCount: %d Nodes: %lld Cut: %d\n", world_rank, taskCount, nodes, cut);
  }
  
  t1 = MPI_Wtime();
  printf("Proc: %d Time: %f\n", world_rank, t1 - t0);
  MPI_Finalize();
}
