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

#define NUM_CYCLES 1
#define LEFT_NEIGHBOR(i)	(i+NUM_PHILOSOPHERS-1)%NUM_PHILOSOPHERS
#define RIGHT_NEIGHBOR(i)	(i+1)%NUM_PHILOSOPHERS

enum state {IDLE, EATING, THINKING};

sem_t mutex;
sem_t s[NUM_PHILOSOPHERS];
sem_t sections[NUM_PHILOSOPHERS];
int forks[NUM_PHILOSOPHERS];
int num_forks[NUM_PHILOSOPHERS];
int cycles[NUM_PHILOSOPHERS];

void eat(int id);
void think(int id);
void nap_time();
void take_fork(int id, int num);
void put_fork_down(int i);
void test(int i, int num);
void print_state_change(int id, int state);

void *philosopher_cycle(void *i)
{
	int id = *(int*) i;
	enum state current_state = IDLE;
	
	while (cycles[id] < NUM_CYCLES)
	{
		print_state_change(id, current_state);
		take_fork(id, num_forks[id]);
		num_forks[id]++;

		take_fork(id, num_forks[id]);
		num_forks[id]++;
		
		current_state = EATING;
		print_state_change(id, current_state);
		eat(id);
		current_state = IDLE;
		put_fork_down(id);
		print_state_change(id, current_state);
		
		//put_fork_down(id);
		
		current_state = THINKING;
		print_state_change(id, current_state);
		think(id);
		current_state = IDLE;
		print_state_change(id, current_state);

		cycles[id]++;
	}

	return NULL;
}

void print_state_change(int id, int state)
{
	if (id == 0)
	{
		int val, val2;
		sem_getvalue(&sections[0], &val);
		sem_getvalue(&sections[1], &val2);
		//printf("before wait(a): a %d b %d\n", val, val2);
		sem_wait(&sections[0]);

		sem_getvalue(&sections[0], &val);
		sem_getvalue(&sections[1], &val2);
		//printf("after wait(a): a %d b %d\n", val, val2);

		if (state == IDLE)
			printf("|A-----\t\t");
		else if (state == EATING)
			printf("|A----- Eat\t");
		else if (state == THINKING)
			printf("|A----- Think\t");
		sem_post(&sections[1]);
		sem_getvalue(&sections[0], &val);
		sem_getvalue(&sections[1], &val2);
		//printf("after post(a): a %d b %d\n", val, val2);
	}
	else if (id == 1)
	{
		int v, v2;
		//printf("about to read sections(b)\n");
		sem_getvalue(&sections[1], &v);;
		sem_getvalue(&sections[0], &v2);
		//printf("read sections(b)\n");
		//printf("before wait(b): b %d a %d\n", v, v2);
		sem_wait(&sections[1]);
		
		sem_getvalue(&sections[1], &v);
		sem_getvalue(&sections[0], &v2);
		//printf("after wait(b): b %d a %d\n", v, v2);

		if (state == IDLE)
			printf("|B-----\t\t|\n");
		else if (state == EATING)
			printf("|B----- Eat\t|\n");
		else if (state == THINKING)
			printf("|B----- Think\t|\n");

		sem_post(&sections[0]);
		sem_getvalue(&sections[1], &v);
		sem_getvalue(&sections[0], &v2);
		//printf("after post(b): b %d a %d\n", v, v2);
	}
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
	//printf("philosopher %d let go of fork %d\n", i, left_fork);

	forks[right_fork] = 0;
	num_forks[i]--;
	//printf("philosopher %d let go of fork %d\n", i, right_fork);


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
				//printf("philosopher %d grabbed fork %d\n", i, right_fork);
				sem_post(&s[i]);
			}
		}
		// odd philosophers will attempt to grab the left fork first
		else
		{
			//printf("philosopher %d is checking fork %d\n", i, left_fork);
			if (forks[left_fork] == 0)
			{
				forks[left_fork] = 1;
				//printf("philosopher %d grabbed fork %d\n", i, left_fork);
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
				//printf("philosopher %d grabbed fork %d\n", i, left_fork);
				sem_post(&s[i]);
			}
		}
		else
		{
			// try to grab the right fork
			if (forks[right_fork] == 0)
			{
				forks[right_fork] = 1;
				//printf("philosopher %d grabbed fork %d\n", i, right_fork);
				sem_post(&s[i]);
			}
		}
	}
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
