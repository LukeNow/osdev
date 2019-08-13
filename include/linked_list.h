#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

#define LINKED_LIST_INIT(list) \
	static linked_list_t (list) = { NULL, NULL, 0 }

#define linked_list_for_each(pos, list) \
	for (linked_list_node_t *(pos) = (list)->start_ptr; pos != NULL; pos = pos->next_ptr)


typedef struct linked_list_node {
	void *next_ptr;
	void *prev_ptr;
	void *data;
}linked_list_node_t;

typedef struct linked_list {
	linked_list_node_t *start_ptr;
	linked_list_node_t *end_ptr;
	unsigned int size;
}linked_list_t;

void linked_list_cycle(linked_list_t *list);
int linked_list_enqueue(void *data, linked_list_t *list);
void *linked_list_dequeue(linked_list_t *list);
int linked_list_push(void *data, linked_list_t *list);
void *linked_list_pop(linked_list_t *list);

void *linked_list_get(unsigned int index, linked_list_t *list);
int linked_list_search(void *data, linked_list_t *list);
int linked_list_add(void *data, unsigned int index, linked_list_t *list);
int linked_list_remove(unsigned int index, linked_list_t *list);
linked_list_t *linked_list_init();
#endif
