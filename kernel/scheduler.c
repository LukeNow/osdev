#include <stdint.h>
#include <stddef.h>
#include "../include/kprint.h"
#include "../include/task.h"
#include "../include/linked_list.h"
#include "../include/machine.h"
#include "../include/string.h"
#include "../include/timer.h"
#include "../include/scheduler.h"
#include "../include/pit.h"
#include "../include/kdef.h"
#include "../include/kheap.h"

#define GET_FIRST_TASK(list) \
	(task_control_block_t *)(((linked_list_t)(list))->start_ptr->data)

#define QUEUE_NUM 3

LINKED_LIST_INIT(ready_list_0);
LINKED_LIST_INIT(ready_list_1);
LINKED_LIST_INIT(ready_list_2);

LINKED_LIST_INIT(blocked_list);

task_control_block_t *timer_task_tcb;
task_control_block_t *idle_task_tcb;

task_control_block_t *current_task;
task_control_block_t *next_task;

int scheduler_lock_counter = 0;
int scheduler_postponed_counter = 0;
int scheduler_postponed_flag = 0;

static linked_list_t *queue_num_to_list(int queue_num)
{
	switch (queue_num) {
		case 0:
			return &ready_list_0;
		case 1:
			return &ready_list_1;
		case 2:
			return &ready_list_2;
		default:
			return NULL;
	}
}

task_control_block_t *name_to_tcb(char *name)
{
	for (int i = 0; i < QUEUE_NUM; i++) {
		linked_list_t *ready_list = queue_num_to_list(i);
		linked_list_for_each(index, ready_list) {
			task_control_block_t *task = index->data;
			if (strcmp(name, task->name) == 0)
				return task;
		}
	}
		
	linked_list_for_each(index, &blocked_list) {
		task_control_block_t *task = index->data;
		if (strcmp(name, task->name) == 0)
			return task;
	}

	return NULL;
}

static void update_next_task()
{	
	/* Get the next available task from the highest priority, non-empty
	 * queue. If the task that is found is the same as current task, 
	 * cycle the linked list to make sure that we can schedule other tasks
	 * that might have just been queued */
	for (int i = 0; i < QUEUE_NUM; i++) {
		linked_list_t * ready_list = queue_num_to_list(i);
		if (ready_list->size != 0) {
			next_task = linked_list_get(0, ready_list);
			if (next_task == current_task && ready_list->size != 1) {
				linked_list_cycle(ready_list);
				next_task = linked_list_get(0, ready_list);
			}
			return;
		}
	}
	/* If we didnt find a ready task, we schedule the idle task */
	next_task = idle_task_tcb;
}

void print_ready_queue(int queue_num)
{
	linked_list_t *ready_list = 
		queue_num_to_list(queue_num);
	kprint(INFO, "Printing out queue num %d\n", queue_num);
	linked_list_for_each(index, ready_list) {
		task_control_block_t *task = index->data;
		kprint(INFO, "Task: %s\n", task->name);
	}
}

void soft_lock_scheduler() 
{
	disable_int();
	scheduler_lock_counter++;
}


void soft_unlock_scheduler()
{
	scheduler_lock_counter--;
	ASSERT(scheduler_lock_counter >= 0);
	if (scheduler_lock_counter == 0) {
		enable_int();
	}
}

void hard_lock_scheduler()
{
	soft_lock_scheduler();
	scheduler_postponed_counter++;
}

void hard_unlock_scheduler()
{
	scheduler_postponed_counter--;
	ASSERT(scheduler_postponed_counter >= 0);
	if (scheduler_lock_counter == 0) {
		if (scheduler_postponed_flag != 0) {
			scheduler_postponed_flag = 0;
			schedule();
		}
	}
	soft_unlock_scheduler();
}

/* MUST be called with soft_scheduler_lock called */
void schedule()
{
	/* If there is a hard scheduler lock being held
	 * do not switch_task() */
	if (scheduler_postponed_counter != 0) {
		scheduler_postponed_flag = 1;
		return;
	}
	
	/* Update ready list to get next ready task */
	update_next_task();
	
	update_time_used();
	/* If the next task happens to be the same task as our
	 * current one, there is no need to switch tasks */
	if (next_task == current_task) {
		/* Record time usage starting now */
		current_task->last_time = timer_get_ns();
		return;
	}
	switch_task();
	/* record start of time usage period */
	current_task->last_time = timer_get_ns();
}

/***
 * General task scheduling functions
 ***/
void schedule_task_ready(int queue_num, task_control_block_t *task)
{
	int blocked_index = linked_list_search(task, &blocked_list);
	if (blocked_index != -1)
		linked_list_remove(blocked_index, &blocked_list);
	
	linked_list_t *ready_list = queue_num_to_list(queue_num);
	
	if (ready_list != NULL)
		linked_list_enqueue(task, ready_list);
}

void schedule_task_blocked(task_control_block_t *task)
{
	int ready_index = -1;
	linked_list_t *ready_list = 
		queue_num_to_list(task->current_priority);
	
	if (ready_list != NULL) {
		ready_index = linked_list_search(task, ready_list);
	}
	if (ready_index != -1)
		linked_list_remove(ready_index, ready_list);
	
	linked_list_enqueue(task, &blocked_list);
}

void unschedule_task(task_control_block_t *task)
{
	int ready_index = -1;
	linked_list_t *ready_list = 
		queue_num_to_list(task->current_priority);

	if (ready_list != NULL)
		ready_index = linked_list_search(task, ready_list);
	int blocked_index = linked_list_search(task, &blocked_list);
	if (ready_index != -1)
		linked_list_remove(ready_index, ready_list);
	if (blocked_index != -1)
		linked_list_remove(blocked_index, &blocked_list);
}

void block_task(task_state_t new_state)
{
	soft_lock_scheduler();

	schedule_task_blocked(current_task);
	current_task->state = new_state;
	current_task->current_priority = -1;

	schedule();
	/* The task unblocks here, record start of time usage period */
	current_task->last_time = timer_get_ns(); 
	soft_unlock_scheduler();
}

void unblock_task(task_control_block_t *task)
{
	ASSERT(task->state == SLEEPING || 
	       task->state == PAUSED);
	soft_lock_scheduler();
	/* When unblocked put it in its original queue */
	schedule_task_ready(task->starting_priority, task);
	task->current_priority = task->starting_priority; 
	
	current_task->state = READY;
	schedule();
	soft_unlock_scheduler();
}

void init_scheduler(task_control_block_t *first_task)
{
	soft_lock_scheduler();
	
	/* Create and schedule timer task */
	timer_task_tcb = create_task(timer_task, 0, "timer_task");
	timer_task_tcb->current_priority = 0;
	timer_task_tcb->state = READY;
	schedule_task_ready(timer_task_tcb->starting_priority, timer_task_tcb);
	
	/* Create and schedule idle task 
	 * Priority is -1 meaning it is never actually in any priority queue. 
	 * This means that we never ever schedule idle task and just jump to it
	 * whenver we dont have anything to do */
	idle_task_tcb = create_task(idle_task, -1, "idle_task");
	idle_task_tcb->current_priority = -1;
	idle_task_tcb->state = IDLE;
	
	/* Finally schedule the first task */
	first_task->current_priority = 0;
	first_task->state = READY;
	schedule_task_ready(first_task->starting_priority, first_task);
	
	/* Initialize scheduler variables */
	current_task = first_task;
	next_task = NULL;

	soft_unlock_scheduler();
}
