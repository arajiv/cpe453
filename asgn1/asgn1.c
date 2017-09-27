#include "malloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#pragma pack(1)

struct dummy
{
	int x;
	int y;
	int z;
};

int main()
{
	/* testing malloc with dynamic library */
	malloc(16);
	
	/*
	int i;

	// testing realloc
	// malloc case
	void *first_block = realloc(NULL, 16);
	printf("block 1: %p\n", first_block);
	//memset(first_block, 3, 16);

	void *second_block = realloc(NULL, 16);
	memset(second_block, 3, 16);
	printf("block 2: %p\n", second_block);

	void *third_block = realloc(NULL, 16);
	//memset(second_block, 3, 16);
	printf("block 3: %p\n", third_block);
	
	// case 2a: combining with previous block
	//free(first_block);
	//void *third_block = realloc(second_block, 64);
	//printf("block 3: %p\n", third_block);

	// case 2b: combining with next block
	//free(second_block);
	//void *third_block = realloc(first_block, 64);
	//printf("block 3: %p\n", third_block);

	// case 2c: combining with prev and next block
	//free(first_block);
	//free(third_block);
	//void *fourth_block = realloc(second_block, 112);

	// case 2d: extending the heap at the end
	free(first_block);
	free(third_block);
	void *fourth_block = realloc(second_block, 113);

	printf("block 4: %p\n", fourth_block);

	Block *block = (Block*) (fourth_block - META_DATA_SIZE);
	char *byte = (char*) fourth_block;
	for (i = 0; i < block->size; i++, byte++)
		printf("byte %d: %d\n", i, *byte);
	*/

	/*
	testing splitting blocks function
	tmp = (int*) malloc(16);
	printf("block 1 data: %p\n", tmp);
	free_block = (int*) malloc(64);
	printf("block 2 data: %p\n", free_block);
	tmp = (int*) malloc(16);
	printf("block 3 data: %p\n", tmp);

	free(free_block);	
	tmp = (int*) malloc(16);
	printf("malloc after free: %p\n", tmp);
	tmp = (int*) malloc(16);
	printf("malloc(16) again: %p\n", tmp);
	*/

	/*
	 * testing calloc
	char *byte;
	void *ptr = calloc(2, sizeof(struct dummy));
	Block* b = (Block*) (ptr - META_DATA_SIZE);

	printf("size of chunck: %lu\n", b->size);
	
	byte = (char*) ptr;
	for (i = 0; i < b->size; i++, byte++)
	{
		printf("byte %d: %d\n", i, *byte);
	}
	 */

	/*
	testing combining blocks function
	int *tmp;
	int *free_block1, *free_block2;
	char *b;
	Block *block;	
	free_block1 = (int*) malloc(16);
	printf("block 1: %p\n", free_block1);
	free_block2 = (int*) malloc(16);
	printf("block 2: %p\n", free_block2);
	tmp = (int*) malloc(16);
	printf("block 3: %p\n", tmp);

	free(free_block1);
	free(free_block2);

	b = (char*) malloc(16);
	printf("data address of block after frees: %p\n", b);
	b -= META_DATA_SIZE;
	block = (Block*) b;
	printf("block1->next: %p\n", block->next);
	*/
	
	//tmp = malloc(10);
	//free(tmp-1);
	/*
	for (i = 0; i < 5; i++)
	{
		tmp = (int*) malloc(1);

		if (i == 2)
			free_block = tmp;

		printf("block %d: %p\n\n", i+1, tmp);
	}

	free(free_block);

	tmp = (int*) malloc(32);
	printf("malloc(32): %p\n", tmp);

	tmp = (int*) malloc(10);
	printf("malloc(10): %p\n", tmp);
	*/

	return 0;
}
