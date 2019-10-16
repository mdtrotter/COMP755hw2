#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <semaphore.h>
#include <unistd.h>
#include <queue>

using namespace std;

// Assuming only three stages
sem_t availCalc;
sem_t availCons;
sem_t qs1;
sem_t qs2;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
queue<int> queue1;
queue<int> queue2;
int widgetNum = 0;
int q1Max = 20;
int q2Max = 10;
int q1Count = 0;
int q2Count = 0;
int calcCount = 0;
int consCount = 0;

// Function used by each producer to place id on stack

void *prod(void *threadid)
{
	long tid;
	tid = (long)threadid;
	for(int i=0; i<100; i++){
		pthread_mutex_lock(&mutex1);
   		queue1.push(widgetNum);
		widgetNum++;
		pthread_mutex_unlock(&mutex1);
		sem_wait(&qs1);
		pthread_mutex_lock(&mutex2);
		cout << "thread " << tid << " is producing widget " << i << endl;
		pthread_mutex_unlock(&mutex2);
		sem_post(&availCalc);
	}
	pthread_exit(NULL);
}

void *calc(void *threadid)
{
	long tid;
	tid = (long)threadid;
	for(int i=0; i<300; i++){
		sem_wait(&availCalc);
		pthread_mutex_lock(&mutex1);
		queue1.pop();
		pthread_mutex_unlock(&mutex1);
		sem_post(&qs1);
		
		pthread_mutex_lock(&mutex2);
		queue2.push(calcCount);
		pthread_mutex_unlock(&mutex2);
		sem_wait(&qs2);
		pthread_mutex_lock(&mutex2);
		cout << "thread " << tid << " is performing calculations on widget " << i << endl;
		pthread_mutex_unlock(&mutex2);
		sem_post(&availCons);
	}
	pthread_exit(NULL);
}

void *cons(void *threadid)
{
	long tid;
	tid = (long)threadid;
	for(int i=0; i<75; i++){
		sem_wait(&availCons);
		pthread_mutex_lock(&mutex2);
		queue2.pop();
		cout << "thread " << tid << " is consuming widget " << i << endl;
		pthread_mutex_unlock(&mutex2);
		sem_post(&qs2);
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
   pthread_t producers[3];
   pthread_t consumers[4];
   pthread_t calculate;
   int rc;
   int n;
   int m;
   long t;
   
   n = atoi(argv[1]);
   m = atoi(argv[2]);

   sem_init(&availCalc, 0, 0);
   sem_init(&qs1, 0, n);
   sem_init(&availCons, 0, 0);
   sem_init(&qs2, 0, m);
   // Producers/Consumers created
   for(t=0;t<3;t++){
   	rc = pthread_create(&producers[t], NULL, prod, (void *)t);
   	if (rc){
       		printf("ERROR; return code from pthread_create() is %d\n", rc);
       		exit(-1);
     	}
   }
   rc = pthread_create(&calculate, NULL, calc, (void*)t);
   t++;
   long x = t;
   for(t=0; t<4; t++){
   	rc = pthread_create(&consumers[t], NULL, cons, (void *)x);
     	x++;
     	if (rc){
       		printf("ERROR; return code from pthread_create() is %d\n", rc);
       		exit(-1);
       	}
   }

   for( t = 0; t < 3; t++){
      	pthread_join(producers[t], 0);
   }
   pthread_join(calculate, 0);
   for( t = 0; t < 4; t++){
      	pthread_join(consumers[t], 0);
   }

   /* Last thing that main() should do */
   pthread_exit(NULL);
}
