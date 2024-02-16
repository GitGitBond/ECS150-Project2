#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
// list queue rather than sequence queue

struct queue_node {
	void *data;
	struct queue_node *next;
};


struct queue {
	struct queue_node *head;
	struct queue_node *tail;
	int size;
};

queue_t queue_create(void) {
	queue_t my_queue = (queue_t)malloc(sizeof(struct queue));
	if(my_queue == NULL) {
		return NULL;
	}
	my_queue->head = NULL;
	my_queue->tail = NULL;
	my_queue->size = 0;
	return my_queue;
}

int queue_destroy(queue_t queue) {
	if(queue == NULL || queue->size != 0){
		return -1;
	}
	free(queue);
	queue = NULL;
	return 0;
}

int queue_enqueue(queue_t queue, void *data) {
	if(queue == NULL || data == NULL) {
		return -1;
	}
	queue_node_t my_queue_node = (queue_node_t)malloc(sizeof(struct queue_node));
	if(my_queue_node == NULL) {
		return -1;
	}
	my_queue_node->data = data;
	my_queue_node->next = NULL;

	if(queue->size == 0) {
		queue->head = my_queue_node;
		queue->tail = my_queue_node;
	} else {
		queue->tail->next = my_queue_node;
		queue->tail = my_queue_node;
	}
	queue->size++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data) {
	if(queue == NULL || data == NULL || queue->size == 0) {
		return -1;
	}
	*data = queue->head->data;
	queue_node_t temp = queue->head;
	queue->head = queue->head->next;
	free(temp);
	queue->size--;
	return 0;
}

int queue_delete(queue_t queue, void *data) {
	if(queue == NULL || data == NULL || queue->size == 0) {
		return -1;
	}
	queue_node_t curr_node = queue->head;
	queue_node_t prev = NULL;

	while(curr_node != NULL) {
		if(curr_node->data == data) {
			queue->size--;
			break;
		}
		prev = curr_node;
		curr_node = curr_node->next;
	}

	// handle special situation
	// if the queue is empty after delete the element
	if(queue->head == curr_node && queue->tail == curr_node) {
		queue->head = NULL;
		queue->tail = NULL;
		free(curr_node);
		return 0;
	}else if(queue->head == curr_node) {
		queue->head = curr_node->next;
		free(curr_node);
		return 0;
	}else if(queue->tail == curr_node) {
		queue->tail = prev;
		queue->tail->next = NULL;
		free(curr_node);
		return 0;
	}else if(curr_node != NULL) {
		prev->next = curr_node->next;
		free(curr_node);
		return 0;
	}
	// other unexpected condition
	return -1;

}

int queue_iterate(queue_t queue, queue_func_t func) {
	if(queue == NULL || func == NULL) {
		return -1;
	}
	queue_node_t curr_node = queue->head;
	while(curr_node != NULL) {
		(*func)(queue, curr_node->data);
		curr_node = curr_node->next;
	}
	return 0;
}

int queue_length(queue_t queue) {
	if(queue == NULL) {
		return -1;
	}
	return queue->size;
}

