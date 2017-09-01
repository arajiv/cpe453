#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 12
#endif

#define LEFT_NEIGHBOR(i)	(i+NUM_PHILOSOPHERS-1)%NUM_PHILOSOPHERS
#define RIGHT_NEIGHBOR(i)	(i+1)%NUM_PHILOSOPHERS

int num_cycles = 1;
enum state {IDLE, GRABBED_FIRST_FORK, GRABBED_SECOND_FORK, EATING, FINISHED, THINKING};

sem_t mutex;
sem_t s[NUM_PHILOSOPHERS];
sem_t sections[NUM_PHILOSOPHERS];
int forks[NUM_PHILOSOPHERS];
int num_forks[NUM_PHILOSOPHERS];
int cycles[NUM_PHILOSOPHERS];
char sec[NUM_PHILOSOPHERS][63+6];
enum state philosopher_state[NUM_PHILOSOPHERS];

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
	philosopher_state[id] = IDLE;
	
	while (cycles[id] < num_cycles)
	{
		fork_grabbed = take_fork(id, num_forks[id]);
		philosopher_state[id] = GRABBED_FIRST_FORK;
		print_state_change(id, philosopher_state[id], fork_grabbed);
		num_forks[id]++;

		fork_grabbed = take_fork(id, num_forks[id]);
		philosopher_state[id] = GRABBED_SECOND_FORK;
		print_state_change(id, philosopher_state[id], fork_grabbed);
		num_forks[id]++;
		
		philosopher_state[id] = EATING;
		print_state_change(id, philosopher_state[id], -1);
		eat(id);

		philosopher_state[id] = FINISHED;
		print_state_change(id, philosopher_state[id], -1);

		put_fork_down(id);
		
		philosopher_state[id] = THINKING;
		print_state_change(id, philosopher_state[id], -1);
		think(id);
		philosopher_state[id] = IDLE;
		print_state_change(id, philosopher_state[id], -1);

		cycles[id]++;
	}

	return NULL;
}

void print_state_change(int id, int state, int fork_grabbed)
{
	
	sem_wait(&sections[0]);

	int i;
	int right_fork = id;
	int left_fork = 0;
	char dashes[20] = "";

	if (id < (NUM_PHILOSOPHERS-1))
		left_fork = id + 1;
	
	for (i = 0; i < NUM_PHILOSOPHERS; i++)
		strcat(dashes, "-");

	for (i = 0; i < NUM_PHILOSOPHERS; i++)
	{
		if (i != id && strlen(sec[i]) == 0)
		{
			sprintf(sec[i],"|%s      ", dashes);
			if (i == (NUM_PHILOSOPHERS -1))
				strcat(sec[i], "|\n");
		}
	}

	if (state == IDLE)
		sprintf(sec[id],"|%s      ", dashes);
	else if (state == THINKING)
	{
		strcat(dashes, " Think");
		sprintf(sec[id],"|%s", dashes);
	}
	else if (state == EATING)
	{
		char temp[63];
		char middle[NUM_PHILOSOPHERS] = "";
		sprintf(sec[id],"|");
		for (i = 0; i < NUM_PHILOSOPHERS; i++)
		{
			if (i == left_fork || i == right_fork)
				sprintf(temp, "%d", i);
			else 
				sprintf(temp, "%c", '-');
			strcat(middle, temp);
		}
		strcat(middle, " Eat");
		sprintf(temp, "%s  ", middle);
		strcat(sec[id], temp);
	}
	else if (state == GRABBED_FIRST_FORK)
	{
		char temp[63];
		char middle[NUM_PHILOSOPHERS] = "";

		sprintf(sec[id],"|");
		if (forks[right_fork] == 1)
			fork_grabbed = right_fork;
		else if (forks[left_fork] == 1)
			fork_grabbed = left_fork;

		for (i = 0; i < NUM_PHILOSOPHERS; i++)
		{
			if (i == fork_grabbed)
				sprintf(temp, "%d", i);
			else 
				sprintf(temp, "%c", '-');
			strcat(middle, temp);
		}

		sprintf(temp, "%s      ", middle);
		strcat(sec[id], temp);
	}
	else if (state == GRABBED_SECOND_FORK || state == FINISHED)
	{
		char temp[63];
		char middle[NUM_PHILOSOPHERS] = "";
		sprintf(sec[id],"|");
		for (i = 0; i < NUM_PHILOSOPHERS; i++)
		{
			if (i == left_fork || i == right_fork)
				sprintf(temp, "%d", i);
			else 
				sprintf(temp, "%c", '-');
			strcat(middle, temp);
		}

		sprintf(temp, "%s      ", middle);
		strcat(sec[id], temp);
	}

	if (id == (NUM_PHILOSOPHERS-1))
		strcat(sec[id], "|\n");
	
	for (i = 0; i < NUM_PHILOSOPHERS; i++)
		printf("%s", sec[i]);
	
	sem_post(&sections[0]);
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
	int ln = LEFT_NEIGHBOR(i);
	int rn = RIGHT_NEIGHBOR(i);
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
	
	if (cycles[ln] < num_cycles && philosopher_state[ln] < EATING)
	{
		//printf("id: %d, left_neighbor: %d, cycles_ln: %d\n", i, LEFT_NEIGHBOR(i), cycles[LEFT_NEIGHBOR(i)]);
		test(LEFT_NEIGHBOR(i), num_forks[LEFT_NEIGHBOR(i)]);
	}

	if (cycles[rn] < num_cycles && philosopher_state[rn] < EATING)
	{
		//printf("id: %d, right_neighbor: %d, cycles_rn: %d\n", i, RIGHT_NEIGHBOR(i), cycles[RIGHT_NEIGHBOR(i)]);
		test(RIGHT_NEIGHBOR(i), num_forks[RIGHT_NEIGHBOR(i)]);
	}
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
				//printf("philosopher %d is grabbing fork %d\n", i, right_fork);
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
				//printf("philosopher %d is grabbing fork %d\n", i, left_fork);
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
				//printf("philosopher %d is grabbing fork %d\n", i, left_fork);
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
				//printf("philosopher %d is grabbing fork %d\n", i, right_fork);
				forks[right_fork] = 1;
				sem_post(&s[i]);
				return right_fork;
			}
		}
	}

	return -1;
}

void print_header(int n)
{
	int i, middle;
	char temp[63] = "";
	for (i = 0; i < n+6; i++)
		strcat(temp, "=");
	for (i = 0; i < n-1; i++)
		printf("|%s", temp);
	printf("|%s|\n", temp);
		
	printf("|");
	middle = (n+6)/2;
	char temp2[63] = "";
	for (i = 0; i < n+6; i++)
	{
		strcat(temp2, " ");
	}
	strcat(temp2,"|");
	for (i = 0; i < n; i++)
	{
		temp2[middle] = 'A' + i;	
		if (i == (n-1))
			printf("%s\n", temp2);
		else
			printf("%s", temp2);
	}

	char temp3[63] = "";
	for (i = 0; i < n+6; i++)
		strcat(temp3, "=");
	for (i = 0; i < n-1; i++)
		printf("|%s", temp3);
	printf("|%s|\n", temp3);
	
	int j;
	for (j = 0; j < n; j++)
	{
		printf("|");
		for (i = 0; i < n; i++)
			printf("%c", '-');           

		for (;i < n+6; i++)
			printf(" ");
	}
	printf("|\n");
}

void check_args(int argc, char *argv[])
{
	if (argc > 2)
	{
		printf("usage: ./dine -ncX, where X is some positive integer\n");
		exit(-1);
	}
	else if (argc == 2)
	{
		if (sscanf(argv[1], "-nc%d", &num_cycles) != 1)
		{
			printf("usage: ./dine -ncX, where X is some positive integer\n");
			exit(-1);
		}
	}
}

int main(int argc, char *argv[])
{
	int i;
	int id[NUM_PHILOSOPHERS];
	pthread_t childid[NUM_PHILOSOPHERS];
	
	srandom(time(NULL));
	check_args(argc, argv);

	sem_init(&mutex, 0, 1);

	for (i = 0; i < NUM_PHILOSOPHERS; i++)
	{
		id[i] = i;
		num_forks[i] = 0;
		cycles[i] = 0;
	}

	sem_post(&sections[0]);
	print_header(NUM_PHILOSOPHERS);
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
	}

	sem_destroy(&mutex);
	for (i = 0; i < NUM_PHILOSOPHERS; i++)
		sem_destroy(&s[i]);
	
	for (i = 0; i < NUM_PHILOSOPHERS; i++)
		sem_destroy(&sections[i]);

	return 0;
}
