#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <queue.h>
#include <string.h>
/* Test macro*/
#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)


// void print_queue(queue_t queue) {
//     if (queue == NULL) {
//         printf("Queue is NULL\n");
//         return;
//     }

//     printf("Queue contents:\n");
//     queue_node_t current = queue->head;
//     while (current != NULL) {
//         printf("%p -> ", current->data); // Assuming data is a pointer
//         current = current->next;
//     }
//     printf("NULL\n");
// }


/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}

int main(void)
{
	test_create();
	test_queue_simple();
	return 0;
}
