#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 2
#endif

#define NUM_CYCLES 2
#define LEFT_NEIGHBOR(i)	(i+NUM_PHILOSOPHERS-1)%NUM_PHILOSOPHERS
#define RIGHT_NEIGHBOR(i)	(i+1)%NUM_PHILOSOPHERS

enum state {IDLE, EATING, THINKING, FINISHED, GRABBED_FIRST_FORK, GRABBED_SECOND_FORK};

sem_t mutex;
sem_t s[NUM_PHILOSOPHERS];
sem_t sections[NUM_PHILOSOPHERS];
int forks[NUM_PHILOSOPHERS];
int num_forks[NUM_PHILOSOPHERS];
int cycles[NUM_PHILOSOPHERS];
char sectionA[20];
char sectionB[20];

void eat(int id);
void think(int id);
void nap_time();
int take_fork(int id, int num);
void put_fork_down(int i);
int test(int i, int num);
void print_state_change(int id, int state, int fork_grabbed);

void *philosopher_cycle(void *i)
{
	int id = *(int*) i;
	int fork_grabbed;
	enum state current_state = IDLE;
	
	while (cycles[id] < NUM_CYCLES)
	{
		//print_state_change(id, current_state, -1);

		fork_grabbed = take_fork(id, num_forks[id]);
		current_state = GRABBED_FIRST_FORK;
		print_state_change(id, current_state, fork_grabbed);
		num_forks[id]++;

		fork_grabbed = take_fork(id, num_forks[id]);
		current_state = GRABBED_SECOND_FORK;
		print_state_change(id, current_state, fork_grabbed);
		num_forks[id]++;
		
		current_state = EATING;
		print_state_change(id, current_state, -1);
		eat(id);

		current_state = FINISHED;
		print_state_change(id, current_state, -1);

		put_fork_down(id);
		
		current_state = THINKING;
		print_state_change(id, current_state, -1);
		think(id);
		current_state = IDLE;
		print_state_change(id, current_state, -1);

		cycles[id]++;
	}

	return NULL;
}

void print_state_change(int id, int state, int fork_grabbed)
{
	
	sem_wait(&sections[0]);

	if (id == 0)
	{
		if (strlen(sectionB) == 0)
			sprintf(sectionB,"|B-----\t\t|\n");
		
		if (state == IDLE)
			sprintf(sectionA,"|A-----\t\t");
		else if (state == EATING)
			sprintf(sectionA,"|A01--- Eat\t");
		else if (state == THINKING)
			sprintf(sectionA,"|A----- Think\t");
		else if (state == GRABBED_FIRST_FORK)
		{
			if (fork_grabbed == 0)
				sprintf(sectionA,"|A0----\t\t");
			else
				sprintf(sectionA,"|A-1---\t\t");
		}
		else if (state == GRABBED_SECOND_FORK || state == FINISHED)
		{
			sprintf(sectionA,"|A01---\t\t");
		}
		else
			sprintf(sectionA,"|A-----\t\t");
	}
	else if (id == 1)
	{
		if (strlen(sectionA) == 0)
			sprintf(sectionA,"|A-----\t\t|\n");

		if (state == IDLE)
			sprintf(sectionB,"|B-----\t\t|\n");
		else if (state == EATING)
			sprintf(sectionB,"|B01--- Eat\t|\n");
		else if (state == THINKING)
			sprintf(sectionB,"|B----- Think\t|\n");
		else if (state == GRABBED_FIRST_FORK)
		{
			if (fork_grabbed == 0)
				sprintf(sectionB,"|B0----\t\t|\n");
			else
				sprintf(sectionB,"|B-1---\t\t|\n");
		}
		else if (state == GRABBED_SECOND_FORK || state == FINISHED)
			sprintf(sectionB,"|B01---\t\t|\n");
	}

	printf("%s%s", sectionA, sectionB);

	sem_post(&sections[0]);
	/*
	else if (id == 2)
	{
		int val, val2;
		sem_getvalue(&sections[2], &val);
		sem_getvalue(&sections[3], &val2);
		printf("c %d d %d\n", val, val2);
		sem_wait(&sections[2]);

		
		if (state == IDLE)
			printf("|C-----\t\t");
		else if (state == EATING)
			printf("|C----- Eat\t");
		else if (state == THINKING)
			printf("|C----- Think\t");
		
		sem_post(&sections[3]);
	}
	else if (id == 3)
	{
		int val, val2;
		sem_getvalue(&sections[3], &val);
		sem_getvalue(&sections[4], &val2);
		printf("d %d e %d\n", val, val2);
		sem_wait(&sections[3]);
		
		
		if (state == IDLE)
			printf("|D-----\t\t");
		else if (state == EATING)
			printf("|D----- Eat\t");
		else if (state == THINKING)
			printf("|D----- Think\t");
		

		sem_post(&sections[4]);
	}
	else if (id == 4)
	{
		int val, val2;
		sem_getvalue(&sections[4], &val);
		sem_getvalue(&sections[0], &val2);
		printf("e %d a %d\n", val, val2);
		sem_wait(&sections[4]);

		
		if (state == IDLE)
			printf("|E-----\t\t|\n");
		else if (state == EATING)
			printf("|E----- Eat\t|\n");
		else if (state == THINKING)
			printf("|E----- Think\t|\n");
		

		sem_post(&sections[0]);
	}
	
	else
	{
		if (state == IDLE)
			printf("|-----\t\t");
		else if (state == EATING)
			printf("|----- Eat\t");
		else if (state == THINKING)
			printf("|----- Think\t");

		sem_post(&sections[1]);
		sem_wait(&sections[0]);
	}
	*/
}

void think(int id)
{
	//printf("philosopher %c is THINKING\n", 'A' + id);
	nap_time();
}

void eat(int id)
{
	//printf("philosopher %c is EATING\n", 'A' + id);
	nap_time();
}

void nap_time()
{
	struct timespec tv;
	int msec = (int)(((double) random()/RAND_MAX)*1000);

	tv.tv_sec = 0;
	tv.tv_nsec = 1000000*msec;

	if (nanosleep(&tv, NULL) == -1)
		perror("nano_sleep");
}

int take_fork(int i, int num)
{
	int fork_grabbed;
	sem_wait(&mutex);
	fork_grabbed = test(i, num);	
	sem_post(&mutex);
	sem_wait(&s[i]);		/* blocks if no forks are not acquired */
	return fork_grabbed;
}

void put_fork_down(int i)
{
	int right_fork = i;
	int left_fork = 0;
	int current_state;

	if (i < (NUM_PHILOSOPHERS-1))
		left_fork = i+1;

	sem_wait(&mutex);
	forks[left_fork] = 0;
	num_forks[i]--;
	current_state = GRABBED_FIRST_FORK;
	print_state_change(i, current_state, right_fork);


	forks[right_fork] = 0;
	num_forks[i]--;
	current_state = IDLE;
	print_state_change(i, current_state, -1);

	if (cycles[LEFT_NEIGHBOR(i)] < NUM_CYCLES)
		test(LEFT_NEIGHBOR(i), num_forks[LEFT_NEIGHBOR(i)]);

	if (cycles[RIGHT_NEIGHBOR(i)] < NUM_CYCLES)
		test(RIGHT_NEIGHBOR(i), num_forks[RIGHT_NEIGHBOR(i)]);

	sem_post(&mutex);
}

int test(int i, int num)
{
	int right_fork = i, left_fork = 0;
	
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
				sem_post(&s[i]);
				return right_fork;
			}
		}
		// odd philosophers will attempt to grab the left fork first
		else
		{
			//printf("philosopher %d is checking fork %d\n", i, left_fork);
			if (forks[left_fork] == 0)
			{
				forks[left_fork] = 1;
				sem_post(&s[i]);
				return left_fork;
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
				sem_post(&s[i]);
				return left_fork;
			}
		}
		else
		{
			// try to grab the right fork
			if (forks[right_fork] == 0)
			{
				forks[right_fork] = 1;
				sem_post(&s[i]);
				return right_fork;
			}
		}
	}

	return -1;
}

int main(int argc, char *argv[])
{
	//pid_t pid;
	int i;
	int id[NUM_PHILOSOPHERS];
	pthread_t childid[NUM_PHILOSOPHERS];

	sem_init(&mutex, 0, 1);
	//pid = getpid();

	for (i = 0; i < NUM_PHILOSOPHERS; i++)
	{
		id[i] = i;
		num_forks[i] = 0;
		cycles[i] = 0;
	}

	sem_post(&sections[0]);
	for (i = 0; i < NUM_PHILOSOPHERS; i++)
	{
		int res;
		res = pthread_create(&childid[i], NULL, philosopher_cycle, (void*) (&id[i]));

		if (res != 0)
		{
			//fprintf(stderr, "Child %i:	%s\n",i,strerror(errno));
			exit(-1);
		}
	}

	for (i = 0; i < NUM_PHILOSOPHERS; i++)
	{
		pthread_join(childid[i], NULL);
		//printf("Parent (%d):		childid %d exited.\n\n", (int) pid, (int) childid[i]);
	}

	//printf("Parent (%d):		Goodbye.\n\n", (int) pid);
	
	sem_destroy(&mutex);
	for (i = 0; i < NUM_PHILOSOPHERS; i++)
		sem_destroy(&s[i]);
	
	for (i = 0; i < NUM_PHILOSOPHERS; i++)
		sem_destroy(&sections[i]);

	return 0;
}
