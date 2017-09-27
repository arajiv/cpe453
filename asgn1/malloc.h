#ifndef MALLOC_H
#define MALLOC_H

#include <stdlib.h>

typedef struct block
{
	size_t size;
	int in_use;
	struct block *next;
	struct block *prev;
	char d[1];
} Block;

#define META_DATA_SIZE sizeof(int*)*4

Block* combine_blocks(Block* block);
void split_block(Block* block, size_t size);
Block *add_block(Block* prev_block, size_t size);
Block *find_unused_block(Block **prev, size_t size);
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
int valid_ptr(void *ptr);

#endif
