#include "malloc.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define align16(x) (((((x)-1) >> 4) << 4)+16)

void *start = NULL;
void *last = NULL;
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
         {
            //puts("split it\n");
            split_block(block, aligned_size);	
         }
         //printf("realloc case 1, returning: %p\n", block->d);
         return block->d; 	
      }
      else if (block->next == NULL)
      {
         //block->in_use = 0
         //block = add_block(last, aligned_size);
         //memcpy(block->d, );
         block->size = aligned_size;

         if (ptr+aligned_size > top_of_heap)
         {
            void *prev_top_of_heap;
            int size_to_allocate = (16+META_DATA_SIZE)*64000;

            if (aligned_size > size_to_allocate)
               size_to_allocate = aligned_size + META_DATA_SIZE;

            prev_top_of_heap = sbrk(size_to_allocate);
            top_of_heap=prev_top_of_heap + size_to_allocate;

            if (prev_top_of_heap == (void*) -1)
            {
               //printf("sbrk failed\n");
               return NULL;
            }
         }

         return block->d;
      }
      // case 3: size requested is greater than size of block
      else
      {
         Block *new_block = block;

         /* expansion in place: try to combine neighboring blocks */
         // try to combine previous block with current block
         if (block->prev != NULL && (block->prev)->in_use == 0 &&
            aligned_size <= ((block->prev)->size + 
            META_DATA_SIZE + block->size))
         {
            //puts("combining with previous block\n");
            new_block = combine_blocks(block->prev);
            memcpy(new_block->d, block->d, block->size);
         }
         // try to combine next block with current block
         else if (block->next != NULL && (block->next)->in_use == 0 &&
            aligned_size <= ((block->next)->size + 
            META_DATA_SIZE + block->size))
         {
            //puts("combining with next block\n");
            combine_blocks(new_block);
         }
         else if (block->prev != NULL && block->next != NULL &&
            (block->prev)->in_use == 0 && (block->next)->in_use == 0 &&
            aligned_size <= ((block->prev)->size + 
            (block->next)->size + 2*META_DATA_SIZE + block->size))
         {
            // try to combine previous and next block with current block
            //puts("combining with prev and next blocks\n");
            new_block = combine_blocks(block->prev);
            memcpy(new_block->d, block->d, block->size);
            combine_blocks(new_block);
         }
         /* could not combine blocks, use malloc */
         else
         {
            void *new_unit;

            //puts("could not combine blocks\n");

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
   //else
      //puts("pointer is invalid\n");
   return NULL;
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
   return memset(ptr, 0, align16(size*count));
}

/*
 * Allocates size bytes of memory and returns a pointer to the allocated memory.
 * If there is an error it returns NULL and sets errno to ENOMEM.
 */
void *malloc(size_t size)
{
   Block *b;
   size_t aligned_size = align16(size);

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
         //puts("malloc: adding a unit at the end\n");
         //printf("prev_block: %p\n", prev_block);
         b = add_block(prev_block, aligned_size);
         //printf("base of new block: %p\n", b);

         if (b == NULL)
         {
            exit(EXIT_FAILURE);
            errno = ENOMEM;
            return NULL;
         }
      }
      else
      {
         int size_of_smallest_block = META_DATA_SIZE + 16;
         //puts("malloc found unused block\n");

         // check if we can split the block
         if (b->size >= (size_of_smallest_block + aligned_size))
         {
            //printf("malloc is splitting block: %p\n",b->d);
            split_block(b, aligned_size);
         }
         b->in_use = 1;
      }
   }

   return b->d;
}

void split_block(Block* block, size_t size)
{
   Block *second_block = (char*) block + (size+META_DATA_SIZE);
   int size_of_smallest_block = META_DATA_SIZE + 16;

   /* second block meta data */	
   second_block->size = block->size - (size+META_DATA_SIZE);
   second_block->in_use = 0;
   second_block->next = block->next;
   second_block->prev = block;

   if (block->next)
      (block->next)->prev = second_block;

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
      return;
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
      //printf("block %d\n", i);

      if (cursor == ptr)
      {
         isValid = 1;
         break;		
      }			

      cursor = cursor->next;
      /* 
      if (cursor != cursor->next)
         i++;
      else
      {
         //printf("top of heap: %p\n", top_of_heap);
         exit(EXIT_FAILURE);
      }
      */
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
   int unused_space = 0;
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
      void* prev_top_of_heap;
      int size_to_allocate = (16 + META_DATA_SIZE)*64000;

      /* get the bottom of the block allocated */

      //puts("need more heap\n");
      
      if (size > size_to_allocate)
         size_to_allocate = size + META_DATA_SIZE;

      prev_top_of_heap = sbrk(size_to_allocate);
      //puts("extending heap\n");

      if (unused_space != 0)
         bottom_of_block = (char*) prev_block + 
         (prev_block->size + META_DATA_SIZE);
      else
         bottom_of_block = (Block*) prev_top_of_heap;

      /* calculate the new top of the heap */
      top_of_heap = prev_top_of_heap + size_to_allocate;

      /* check if memory allocation failed */
      if (prev_top_of_heap == (void*) -1)
      {
         //printf("sbrk failed\n");
         return NULL;
      }
   }
   else
   {
      //puts("don't need more heap\n");
      bottom_of_block = (char*)prev_block + (META_DATA_SIZE + prev_block->size);
      //printf("new block data starts at: %p\n", bottom_of_block->d);
   }

   last = bottom_of_block;

   /* link the prev block to the new block */
   if (prev_block)
   {
      prev_block->next = bottom_of_block;
      //printf("next of previous block: %p\n", bottom_of_block->d);
   }

   /* store the metadata of the current block */
   bottom_of_block->size = size;
   bottom_of_block->next = NULL;
   bottom_of_block->prev = prev_block;
   bottom_of_block->in_use = 1;

   return bottom_of_block;
}
