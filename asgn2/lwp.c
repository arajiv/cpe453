/*
 * ============================================================================
 *
 *       Filename:  lwp.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/03/2017 10:28:52 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Heriberto Rodriguez (HR), hrodri02@calpoly.edu
 *          Class:  
 *
 * ============================================================================
 */

#include	<stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "lwp.h"

#define INIT_STACK 2048
thread thr[10000000];

/* function prototypes */
void rr_admit(thread new);
void rr_remove(thread victim);
thread rr_next(void);
void thread_cpy(thread old, thread new);

static struct scheduler rr_sche = {NULL, NULL, rr_admit, rr_remove, rr_next};
scheduler sche = &rr_sche;

thread current_thread = NULL;
rfile *system_rfile;

unsigned long main_sp;
unsigned long num_threads = 0;

typedef struct list
{
   thread t;
   struct list *next;
   struct list *prev;
} List;

thread start = NULL;
thread next_of_sched = NULL;
thread end = NULL;


/* 
 * ===  FUNCTION  =============================================================
 *         Name:  lwp_get_scheduler
 *  Description:  returns pointer to current scheduler
 * ============================================================================
 */
scheduler lwp_get_scheduler(void)
{
   return sche;
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  lwp_gettid
 *  Description:  Returns the tid of the calling LWP or NO_THREAD if not called
 *  by a LWP.
 * ============================================================================
 */
tid_t lwp_gettid(void)
{
   if (current_thread == NULL)
      return NO_THREAD;
   return current_thread->tid;       
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  lwp_set_scheduler
 *  Description:  Causes the LWP package to use the given scheduler to choose
 *  the next process to run. Transfers all threads from the old scheduler to the
 *  new one in next() order. If scheduler is NULL the library should return to
 *  round-robin scheduling.
 * ============================================================================
 */
void lwp_set_scheduler(scheduler fun)
{
   thread new_thr;
   int i;

   if (fun == NULL)
   {
      printf("fun is NULL\n");
      sche = &rr_sche;
   }

   if (fun->init != NULL)
      fun->init();

   if (num_threads == 0 && fun != NULL)
   {
      sche = fun;
      return;
   }

   i = 0;
   thread old_thr = sche->next();

   /* select fun as the new scheduler */
   for (i = 0; i < num_threads; i++)
   {
      //printf("%d\n", i);
      new_thr = malloc(sizeof(context));
      new_thr->stack = malloc(old_thr->stacksize*sizeof(unsigned long));

      /* copy context of old thread to new thread */
      thread_cpy(old_thr, new_thr);

      /* remove old_thr */
      sche->remove(old_thr);

      /* admit new thread */
      fun->admit(new_thr);

      /* move to next node */
      old_thr = sche->next();

      /*  
      if (old_thr == NULL)
      {
         //printf("old_thr is NULL\n");
         //exit(EXIT_FAILURE);
      }
      */
   }

   if (sche->shutdown != NULL)
      sche->shutdown();

   sche = fun;
}

void thread_cpy(thread old, thread new)
{
   new->tid = old->tid;

   new->stack = old->stack;
   new->stacksize = old->stacksize;
   memcpy(new->stack, old->stack, old->stacksize*sizeof(unsigned long));

   new->state = old->state;

   /* fix ptr arithmetic */
   void *ptr = new->stack;
   ptr +=  old->state.rsp - (unsigned long) old->stack;
   new->state.rsp = ptr;

   new->lib_one = old->lib_one;
   new->lib_two = old->lib_two;
   new->sched_one = old->sched_one;
   new->sched_two = old->sched_two;
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  rr_admit
 *  Description:  add a thread to the pool
 * ============================================================================
 */
void rr_admit(thread new)
{
   thread cursor;

   /* new nodes are added to the end */
   for (cursor = start; cursor && cursor != end; 
      cursor = cursor->sched_one)
      ;

   if (start == NULL)
   {
      start = new;
      new->sched_two = NULL;
      new->sched_one = start;
   }
   else
   {
      cursor->sched_one = new;

      if (cursor == start)
         cursor->sched_two = new;

      new->sched_two = cursor;
      new->sched_one = start;
   }

   end = new;
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  rr_remove
 *  Description:  remove a thread from the pool
 * ============================================================================
 */
void rr_remove(thread victim)
{
   thread tmp;

   /* if pool is empty there is nothing to remove */
   if (start == NULL)
      return;

   /* removing the first node */
   if (victim->tid == start->tid)
   {
      tmp = start; 
      next_of_sched = NULL;

      if (start != start->sched_one)
      {
         start = start->sched_one;
      }
      else
         start = NULL;

      tmp->sched_one = NULL;
      tmp->sched_two = NULL;

      if (start != NULL)
      {
         start->sched_two = end;
         end->sched_one = start;
         //printf("rr_remove: end->sched_one: %d\n", end->sched_one->tid);
         //printf("rr_remove: end: %d\n", end->tid);
      }
   }
   /* removing the last node */
   else if (victim->tid == end->tid)
   {
      thread cursor;
      tmp = end;

      for (cursor = start; cursor && (cursor->sched_one) != end; 
         cursor = cursor->sched_one)
         ;

      /* end points to the second to last node */
      end = cursor;
      end->sched_one = start;
      tmp->sched_one = NULL;
      /* tmp next is already NULL since it is the last node */
   }
   /* removing a middle node */
   else
   {
      thread cursor;
      
      for (cursor = start; cursor && ((cursor->sched_one)->tid != victim->tid); 
         cursor = cursor->sched_one)
         ;
      
      /* tmp is the node we want to remove */
      tmp = cursor->sched_one;

      /* link prev node to the one after the one we want to remove */
      cursor->sched_one = tmp->sched_one;

      /* set next of node we want to remove to NULL */
      tmp->sched_one = NULL;
      tmp->sched_two = NULL;
   }

   /* free stack */
   free(tmp->stack);

   /* free thread */
   free(tmp);
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  rr_next
 *  Description:  select a thread to schedule
 * ============================================================================
 */
thread rr_next(void)
{
   if (start == NULL)
   {
      //printf("start is NULL\n");
      return NULL;
   }

   if (next_of_sched == NULL)
   {
      next_of_sched = start;
      return next_of_sched;
   }

   next_of_sched = next_of_sched->sched_one;

   if (next_of_sched == NULL)
      printf("next one is null\n");

   return next_of_sched; 
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  lwp_create
 *  Description:  Creates a new lightweight process which executes the given
 *  function with the given argument. The new processes's stack will be
 *  stacksize words.
 *
 *  lwp_create() returns the (lightweight) thread id of the new thread or -1 if
 *  the thread cannot be created.
 *
 *  First parameter is a function pointer that points to a function that does
 *  not return anything, but takes a void* as its only argument
 * ============================================================================
 */
tid_t lwp_create(lwpfun func, void* arg, size_t stacksize)
{
   thread new_thr = (thread) malloc(sizeof(context));
   unsigned long *start_stack;

   /* assign thread an id */
   new_thr->tid = ++num_threads;

   /* allocate a stack for each LWP */
   new_thr->stack = (unsigned long*) malloc(stacksize*sizeof(unsigned long));
   start_stack = (new_thr->stack + stacksize) - 1;

   new_thr->stacksize = stacksize;

   /* initialize floating point unit of thread */
   new_thr->state.fxsave = FPU_INIT;

   new_thr->state.rdi = arg;

   /* Make the stack look as if the thread called you and was suspended */
   *(start_stack) = lwp_exit;
   start_stack--;
   *(start_stack) = func;  
   start_stack--;
   new_thr->state.rbp = start_stack;
   *(start_stack) = new_thr->state.rbp;

   sche->admit(new_thr);
   thr[num_threads-1] = new_thr;

   return new_thr->tid;
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  lwp_start
 *  Description:  Starts the LWP system. Saves the original context, picks a
 *  LWP (for lwp_stop() to use later), picks a LWP and starts it running. If
 *  there are no LWPs, returns immediately.
 * ============================================================================
 */
void lwp_start(void)
{
   system_rfile = malloc(sizeof(rfile));
   current_thread = sche->next();

   if (current_thread == NULL)
      return;

   swap_rfiles(system_rfile, &(current_thread->state));
}

void lwp_exit_really()
{
   thread prev = current_thread;

   sche->remove(prev);

   /* so that scheduler picks the right thread */
   next_of_sched = NULL;

   current_thread = sche->next();

   if (current_thread == NULL)
      swap_rfiles(NULL, system_rfile);
   else
      swap_rfiles(NULL, &(current_thread->state));
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  lwp_exit
 *  Description:  Terminates the current LWP and frees its resources. Calls
 *  sched->next() to get the next thread. If there are no other threads,
 *  restores the original system thread.
 * ============================================================================
 */
void lwp_exit(void)
{
   SetSP(system_rfile->rsp);
   lwp_exit_really();
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  lwp_yield
 *  Description:  Yields control to another LWP. Which one depends on the
 *  scheduler. Saves the current LWP's context, picks the next one, restores
 *  that thread's context, and returns.
 * ============================================================================
 */
void lwp_yield(void)
{
   thread prev_thread = current_thread;
   current_thread = sche->next();

   if (current_thread == NULL)
      swap_rfiles(NULL, system_rfile);
   swap_rfiles(&(prev_thread->state), &(current_thread->state)); 
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  lwp_stop
 *  Description:  Stops the LWP system, restores the original stack pointer and
 *  returns to that context. (Wherever lwp_start() was called from. lwp_stop()
 *  does not destroy nay existing contexts, and thread processing will be
 *  restarted by a call to lwp_start().)
 * ============================================================================
 */
void lwp_stop(void)
{
   swap_rfiles(&(current_thread->state), system_rfile);   
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  tid2thread
 *  Description:  Returns the thread corresponding to the given thread ID, or
 *  NULL if the ID is invalid.
 * ============================================================================
 */
thread tid2thread(tid_t tid)
{
   int i;

   if (thr[0] == NULL)
      return NULL;

   for (i = 0; i < num_threads; i++)
   {
      if (thr[i]->tid == tid)
         return thr[i];
   }

   return NULL;
}

void dummy(void* ptr)
{
   printf("hi\n");
}

void dummy2(rfile *state_dummy2)
{
   save_context(state_dummy2);
}

/* 
 * ===  FUNCTION  =============================================================
 *         Name:  main
 *  Description:  
 * ============================================================================
int main ( int argc, char *argv[] )
{
   //rfile *state_dummy2 = (rfile*) malloc(sizeof(rfile));
   
  //lwp_create(dummy, NULL, INIT_STACK);
  //lwp_start();
   
   thread t1, t2, t3, victim;
   int i;
   
   t1 = malloc(sizeof(context));
   t2 = malloc(sizeof(context));
   t3 = malloc(sizeof(context));

   sche->admit(t1);
   sche->admit(t2);
   sche->admit(t3);

   for (i = 0; i < 3; i++)
   {
      thread current_thread = sche->next();
      if (i == 2)
         victim = current_thread;
      printf("id of thread that is running: %d\n", current_thread->tid);
   }

   sche->remove(victim);

   for (i = 0; i < 3; i++)
   {
      thread current_thread = sche->next();
      printf("id of thread that is running: %d\n", current_thread->tid);
   }

   if (sche->next() == NULL)
      printf("no more nodes left\n");

   //sche->remove(victim);

   dummy2(state_dummy2);
   printf("state_dummy2: %lu\n", state_dummy2);
   printf("rax: %lu\n", state_dummy2->rax);
   printf("rbx: %lu\n", state_dummy2->rbx);
   printf("rcx: %lu\n", state_dummy2->rcx);
   printf("rdx: %lu\n", state_dummy2->rdx);
   printf("rsi: %lu\n", state_dummy2->rsi);
   printf("rdi: %lu\n", state_dummy2->rdi);
   printf("rbp: %lu\n", state_dummy2->rbp);
   printf("rsp: %lu\n", state_dummy2->rsp);
   printf("r8: %lu\n", state_dummy2->r8);
   printf("r9: %lu\n", state_dummy2->r9);
   printf("r10: %lu\n", state_dummy2->r10);
   printf("r11: %lu\n", state_dummy2->r11);
   printf("r12: %lu\n", state_dummy2->r12);
   printf("r13: %lu\n", state_dummy2->r13);
   printf("r14: %lu\n", state_dummy2->r14);
   printf("r15: %lu\n", state_dummy2->r15);

   call ing lwp_create
   int x = 4;
   void *ptr = &x;
   void (*f)(void*);
   f = &dummy;


   lwp_create(f, ptr, INIT_STACK); 
//   printf("back in main\n");
   return EXIT_SUCCESS;
}
*/
