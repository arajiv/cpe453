#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5
#endif

#define NUM_CYCLES 1
#define LEFT_NEIGHBOR(i)	(i+NUM_PHILOSOPHERS-1)%NUM_PHILOSOPHERS
#define RIGHT_NEIGHBOR(i)	(i+1)%NUM_PHILOSOPHERS

sem_t mutex;
sem_t s[NUM_PHILOSOPHERS];
int forks[NUM_PHILOSOPHERS];
int num_forks[NUM_PHILOSOPHERS];
int cycles[NUM_PHILOSOPHERS];

void eat(int id);
void take_fork(int id, int num);
void put_fork_down(int i);
void test(int i, int num);

void *child(void *id)
{
	int whoami = *(int*) id;

	printf("Child %d (%d):		Hello.\n\n", whoami, (int) getpid());
	printf("Child %d (%d):		Goodbye.\n\n", whoami, (int) getpid());

	return NULL;
}

void *philosopher_cycle(void *i)
{
	int id = *(int*) i;
	
	while (cycles[id] < NUM_CYCLES)
	{
		printf("philosopher %d has %d forks\n", id, num_forks[id]);
		take_fork(id, num_forks[id]);
		num_forks[id]++;
		printf("philosopher %d has %d forks\n", id, num_forks[id]);
		take_fork(id, num_forks[id]);
		num_forks[id]++;
		printf("philosopher %d has %d forks\n", id, num_forks[id]);
		eat(id);
		put_fork_down(id);
		cycles[id]++;
	}

	return NULL;
}

void eat(int id)
{
	printf("philosopher %d is EATING\n", id);
	struct timespec tv;
	int msec = (int)(((double) random()/RAND_MAX)*1000);

	tv.tv_sec = 0;
	tv.tv_nsec = 1000000*msec;
	
	printf("philosopher %d is going to eat for %d ms\n", id, msec);

	if (nanosleep(&tv, NULL) == -1)
	{
		perror("nanosleep");
	}
}

void take_fork(int i, int num)
{
	sem_wait(&mutex);
	test(i, num);	
	sem_post(&mutex);
	sem_wait(&s[i]);		/* blocks if no forks are not acquired */
}

void put_fork_down(int i)
{
	int right_fork = i;
	int left_fork = 0;

	if (i < (NUM_PHILOSOPHERS-1))
		left_fork = i+1;

	sem_wait(&mutex);
	forks[left_fork] = 0;
	num_forks[i]--;
	printf("philosopher %d let go of fork %d\n", i, left_fork);

	forks[right_fork] = 0;
	num_forks[i]--;
	printf("philosopher %d let go of fork %d\n", i, right_fork);


	if (cycles[LEFT_NEIGHBOR(i)] < NUM_CYCLES)
		test(LEFT_NEIGHBOR(i), num_forks[LEFT_NEIGHBOR(i)]);

	if (cycles[RIGHT_NEIGHBOR(i)] < NUM_CYCLES)
		test(RIGHT_NEIGHBOR(i), num_forks[RIGHT_NEIGHBOR(i)]);

	sem_post(&mutex);
}

void test(int i, int num)
{
	int right_fork = i;
	int left_fork = 0;
	
	if (i < (NUM_PHILOSOPHERS-1))
		left_fork = i + 1;

	if (num == 0)
	{
		// if I am an even philosopher grabbing my first fork
		if ((i%2) == 0)
		{
			// try to grab the right fork
			if (forks[right_fork] == 0)
			{	
				forks[right_fork] = 1;
				printf("philosopher %d grabbed fork %d\n", i, right_fork);
				sem_post(&s[i]);
			}
		}
		// odd philosophers will attempt to grab the left fork first
		else
		{
			printf("philosopher %d is checking fork %d\n", i, left_fork);
			if (forks[left_fork] == 0)
			{
				forks[left_fork] = 1;
				printf("philosopher %d grabbed fork %d\n", i, left_fork);
				sem_post(&s[i]);
			}
		}
	}
	else
	{
		if ((i%2) == 0)
		{

			// try to grab the left fork
			if (forks[left_fork] == 0)
			{
				forks[left_fork] = 1;
				printf("philosopher %d grabbed fork %d\n", i, left_fork);
				sem_post(&s[i]);
			}
		}
		else
		{
			// try to grab the right fork
			if (forks[right_fork] == 0)
			{
				forks[right_fork] = 1;
				printf("philosopher %d grabbed fork %d\n", i, right_fork);
				sem_post(&s[i]);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	pid_t pid;
	int i;
	int id[NUM_PHILOSOPHERS];
	pthread_t childid[NUM_PHILOSOPHERS];

	sem_init(&mutex, 0, 1);
	pid = getpid();

	for (i = 0; i < NUM_PHILOSOPHERS; i++)
	{
		id[i] = i;
		num_forks[i] = 0;
		cycles[i] = 0;
	}

	for (i = 0; i < NUM_PHILOSOPHERS; i++)
	{
		int res;
		res = pthread_create(&childid[i], NULL, philosopher_cycle, (void*) (&id[i]));

		if (res != 0)
		{
			fprintf(stderr, "Child %i:	%s\n",i,strerror(errno));
			exit(-1);
		}
	}

	for (i = 0; i < NUM_PHILOSOPHERS; i++)
	{
		pthread_join(childid[i], NULL);
		printf("Parent (%d):		childid %d exited.\n\n", (int) pid, (int) childid[i]);
	}

	printf("Parent (%d):		Goodbye.\n\n", (int) pid);
	
	sem_destroy(&mutex);

	return 0;
}
