#include "malloc.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define align16(x) (((((x)-1) >> 4) << 4)+16)

void *start = NULL;
void *top_of_heap = NULL;

/*
 * Tries to change the size of the allocation pointed to
 * by ptr to size, and returns ptr. If there is not enough
 * room to enlarge the memory allocation pointed to by ptr,
 * realloc creates a new allocation, copies as much of the old
 * data pointed to by ptr as will fit to the new allocation,
 * frees the old allocation, and returns a pointer to the
 * allocated memory. 
 * 
 * realloc(NULL,size) = malloc(size)
 * realloc(ptr, 0) = free(ptr)
 * 
 * void* memcpy(void *restrict dst, const void *restrict src, size_t n)
 * copies n bytes from memory area src to memory area dst.
 * If dst and src overlap, behavior is undefined. Applications in which src
 * and dst might overlap should use memmove instead.
 * memcpy returns the original value of dst
 */
void *realloc(void *ptr, size_t size)
{
	Block* block;

	/* free checks for validity of ptr */
	if (size == 0)
	{
		free(ptr);
		return NULL;
	}

	/* malloc case */
	if (ptr == NULL)
		return malloc(size);	
	
	//puts("in realloc\n");

	/* check that the ptr is valid */
	if (valid_ptr(ptr-META_DATA_SIZE))
	{
		size_t aligned_size = align16(size);
		block = (Block*) (ptr-META_DATA_SIZE);
		
		// case 1: size requested is less than or equal to size of block
		if (aligned_size <= block->size)
		{
			int min_size_block = META_DATA_SIZE + 16;
			
			// check if we can split it the block
			if ((block->size - aligned_size) >= min_size_block)				
				split_block(block, aligned_size);	
			return block->d; 	
		}
		// case 2: size requested is greater than size of block
		else
		{
			Block *new_block = block;

			/* expansion in place: try to combine neighboring blocks */
			// try to combine previous block with current block
			if (block->prev != NULL && (block->prev)->in_use == 0 &&
				aligned_size <= ((block->prev)->size + 
				META_DATA_SIZE + block->size))
			{
				puts("combining with previous block\n");
				new_block = combine_blocks(block->prev);
				memcpy(new_block->d, block->d, block->size);
			}
			// try to combine next block with current block
			else if (block->next != NULL && (block->next)->in_use == 0 &&
				aligned_size <= ((block->next)->size + 
				META_DATA_SIZE + block->size))
			{
				puts("combining with next block\n");
				combine_blocks(new_block);
			}
			// try to combine the previous and next blocks with the current block
			else if (block->prev != NULL && block->next != NULL &&
				(block->prev)->in_use == 0 && (block->next)->in_use == 0 &&
				aligned_size <= ((block->prev)->size + 
					(block->next)->size + 2*META_DATA_SIZE + block->size))
			{
				puts("combining with prev and next blocks\n");
				new_block = combine_blocks(block->prev);
				memcpy(new_block->d, block->d, block->size);
				combine_blocks(new_block);
			}
			/* could not combine blocks, use malloc */
			else
			{
				void *new_unit;

				puts("adding another unit at the end\n");

				/* allocate memory */
				new_unit = malloc(aligned_size);

				/* copy data to new allocation */
				memcpy(new_unit, ptr, block->size);

				/* free old allocation */
				free(block->d);

				return new_unit;
			}

			/* return the new allocation */
			return new_block->d;
		}
	}
	else
		puts("pointer is invalid\n");
}

/*
 * contiguously allocates enough space for count objects that are
 * size bytes of memory each and returns a pointer to the allocated memory.
 * The allocated memory is filled with bytes of value zero.
 * 
 * memset(void* b, int c, size_t len) in string.h
 * writes len bytes of value c to the string b and returns its first argument
 */
void *calloc(size_t count, size_t size)
{
	void *ptr;

	/* 
	 * if every object is 0 bytes then return NULL 
	 * check what library function returns
	 */	
	if (size == 0)
		return NULL;

	/* allocate memory */	
	ptr = malloc(size*count);

	if (ptr == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}
	
	/* assign each byte to 0 and return the start of allocated memory */
	return memset(ptr, 15, align16(size*count));
}

/*
 * Allocates size bytes of memory and returns a pointer to the allocated memory.
 * If there is an error it returns NULL and sets errno to ENOMEM.
 */
void *malloc(size_t size)
{
	Block *b;
	size_t aligned_size = align16(size);

	puts("in malloc\n");
	
	if (size == 0)
		return NULL;
	
	/* allocating memory for the first time */
	if (start == NULL)
	{
		b = add_block(NULL, aligned_size);
		start = b;

		if (b == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}
	}
	else
	{
		Block *prev_block;				
		prev_block = start;

		b = find_unused_block(&prev_block, aligned_size);

		/* no unused block found of the right size, extend memory */
		if (b == NULL)
		{
			puts("malloc: adding a unit at the end\n");
			printf("prev_block: %p\n", prev_block);
			b = add_block(prev_block, aligned_size);
			printf("base of new block: %p\n", b);

			if (b == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}
		}
		else
		{
			int size_of_smallest_block = META_DATA_SIZE + 16;

			// check if we can split the block
			if (b->size >= (size_of_smallest_block + aligned_size))
				split_block(b, aligned_size);
			b->in_use = 1;
		}
	}

	return b->d;
}

void split_block(Block* block, size_t size)
{
	Block *second_block = (char*) block + (block->size + META_DATA_SIZE);
	int size_of_smallest_block = META_DATA_SIZE + 16;

	/* second block meta data */	
	second_block->size = block->size - size_of_smallest_block;
	second_block->in_use = 0;
	second_block->next = block->next;
	second_block->prev = block;

	/* first block meta data */
	block->size = size; 
	block->in_use = 1;
	block->next = second_block;
}

/* 
 * deallocates the memory pointed to by ptr.
 * If ptr is a NULL pointer, no operation is performed.
 */
void free(void *ptr)
{
	if (ptr == NULL)
		return;	

	ptr -= META_DATA_SIZE; 			/* move ptr to base of block */

	// check that pointer passed by user is valid
	if (valid_ptr(ptr))
	{
		Block *b = (Block*) ptr;	/* convert to Block ptr */
		b->in_use = 0;					/* free the block */
		
		/* try to combine freed block with previous block */
		if (b->prev != NULL && (b->prev)->in_use == 0)
		{
			b = combine_blocks(b->prev);
		}
						
		/* try to combine freed block with next block */
		if (b->next != NULL && (b->next)->in_use == 0)
			combine_blocks(b);
	}
	else
	{
		fprintf(stderr, "%p: pointer being freed was not allocated by malloc\n", 
			ptr);
		exit(EXIT_FAILURE);
	}
}

/* only works for the previous block */
Block* combine_blocks(Block* block)
{
	/* change previous block meta data */
	block->size = META_DATA_SIZE + (block->next)->size + block->size;
	block->next = (block->next)->next;
	//printf("block1->next: %p\n", block->next);

	/* change freed block meta data */	
	if ((block->next) != NULL)
		(block->next)->prev = block;
	
	if ((block->prev) != NULL)
		(block->prev)->next = NULL;

	return block;
}

int valid_ptr(void *ptr)
{
	Block *cursor = start;	
	int isValid = 0; 

	while (cursor != NULL)
	{
		if (cursor == ptr)
		{
			isValid = 1;
			break;		
		}			

		cursor = cursor->next;
	}
	
	return isValid;
}

Block* find_unused_block(Block **prev, size_t size)
{
	Block* cursor;

	/* stop when we are at the end of the list or we find a free block */
	for (cursor = *prev; cursor != NULL && (cursor->in_use == 1
		|| (cursor->size < size)); cursor = cursor->next)
		*prev = cursor;

	return cursor;
}

Block* add_block(Block* prev_block, size_t size)
{
	Block *bottom_of_block = NULL;
	int unused_space;
	void *top_of_last_block; 

	// calculate the unused space if the heap has already been created
	if (prev_block)
	{
		top_of_last_block = prev_block + 
			(META_DATA_SIZE + prev_block->size)/sizeof(Block);
		unused_space = top_of_heap - top_of_last_block;
	}

	// if we are allocating memory for the first time or
	// ran out of the memory, then extend the heap with sbrk
	if (prev_block == NULL || 
		(prev_block != NULL && unused_space < (size+META_DATA_SIZE)))
	{
		Block* prev_top_of_heap;

		/* get the bottom of the block allocated */
		prev_top_of_heap = (Block*) sbrk((size+META_DATA_SIZE)*5);

		if (unused_space != 0)
			bottom_of_block = (char*) prev_block + 
				(prev_block->size + META_DATA_SIZE);
		else
			bottom_of_block = prev_top_of_heap;

		/* calculate the new top of the heap */
		top_of_heap = bottom_of_block + ((size+META_DATA_SIZE)*5)/sizeof(Block);

		/* check if memory allocation failed */
		if (prev_top_of_heap == (void*) -1)
			return NULL;
	}
	else
	{
		puts("add_block: in else\n");
		bottom_of_block = (char*)prev_block + (META_DATA_SIZE + prev_block->size);
	}

	/* link the prev block to the new block */
	if (prev_block)
		prev_block->next = bottom_of_block;

	/* store the metadata of the current block */
	bottom_of_block->size = size;
	bottom_of_block->next = NULL;
	bottom_of_block->prev = prev_block;
	bottom_of_block->in_use = 1;

	return bottom_of_block;
}
