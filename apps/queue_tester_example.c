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

void test_queue_length(){
	int set[3] = {0, 1, 2};
	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST queue length ***\n");

	for (int i = 0; i < 3; i++) {
		queue_enqueue(queue, &set[i]);	
	}

	TEST_ASSERT(queue_length(queue) == 3);
}

void test_queue_null_length(){
	queue_t queue = NULL;
	fprintf(stderr, "*** TEST null queue length ***\n");

	int error_value = queue_length(queue);
	TEST_ASSERT(error_value == -1);
}

void test_delete(void) 
{
	int set[5] = {0, 1, 2, 3, 4};
	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST delete queue ***\n");

	for (int i = 0; i < 5; i++) {
		queue_enqueue(queue, &set[i]);	
	}

	queue_delete(queue, &set[1]);
	TEST_ASSERT(queue_length(queue) == 4);
}

void test_delete_not_found(void)
{
	int set[3] = {0, 1, 2};
	int doesnt_exist = 100;
	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST delete not found value ***\n");

	for (int i = 0; i < 3; i++) {
		queue_enqueue(queue, &set[i]);	
	}

	int error_value = queue_delete(queue, &doesnt_exist);
	TEST_ASSERT(error_value == -1);
}

void test_queue_destroy()
{
	int set[3] = {0, 1, 2};
	int *ptr;

	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST destroy queue ***\n");

	for (int i = 0; i < 3; i++) {
		queue_enqueue(queue, &set[i]);	
	}

	for (int i = 0; i < 3; i++) {
		queue_dequeue(queue, (void**)&ptr);	
	}

	int error_value = queue_destroy(queue);
	TEST_ASSERT(error_value == 0);
}

void test_queue_destroy_not_empty(){
	int set[3] = {0, 1, 2};

	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST destroy not empty queue ***\n");

	for (int i = 0; i < 3; i++) {
		queue_enqueue(queue, &set[i]);	
	}

	int error_value = queue_destroy(queue);
	TEST_ASSERT(error_value == -1);
}

void test_dequeue_size_zero() {
	int *ptr;

	fprintf(stderr, "*** TEST dequeue empty queue***\n");

	queue_t queue = queue_create();

	TEST_ASSERT(queue_dequeue(queue, (void**)&ptr) == -1);
}

void test_dequeue_string_array() {
	char** ptr;
	char* set[] = {"zero", "one", "two"};
	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST dequeue string queue***\n");
	
	for (int i = 0; i < 3; i++) {
		queue_enqueue(queue, &set[i]);	
	}

	queue_dequeue(queue, (void**)&ptr);

	TEST_ASSERT(!strcmp(*ptr, "zero"));
}

/* Callback function for queue_iterate test */
static void iterate_func(queue_t queue, void *data) {
	(void)queue; //unused
    printf("%d ", *(int *)data);
}

/* Iterate function to be used in queue_iterate test */
static void test_iterate_data(queue_t queue, const char *expected_output) {
    char buffer[1024];
    FILE *temp_stdout = tmpfile();
    assert(temp_stdout != NULL);
    
    // Redirect stdout to the temporary file
    int saved_stdout = dup(fileno(stdout));
    dup2(fileno(temp_stdout), fileno(stdout));

    // Perform the actual iteration
    queue_iterate(queue, iterate_func);

    // Reset stdout
    fflush(stdout);
    dup2(saved_stdout, fileno(stdout));
    close(saved_stdout);

    // Read the output from the temporary file
    rewind(temp_stdout);
    fread(buffer, 1, sizeof(buffer), temp_stdout);
    fclose(temp_stdout);

    // Check if the output matches the expected output
    TEST_ASSERT(strcmp(buffer, expected_output) == 0);
}

/* Test queue iteration */
void test_queue_iteration() {
    queue_t queue = queue_create();

    int set[3] = {0, 1, 2};

    for (int i = 0; i < 3; i++) {
        queue_enqueue(queue, &set[i]);    
    }

	fprintf(stderr, "*** TEST queue iteration***\n");
    test_iterate_data(queue, "0 1 2 ");
}


int main(void)
{
	test_create();
	test_queue_simple();
	test_queue_length();
	test_queue_null_length();
	test_delete();
	test_delete_not_found();
	test_queue_destroy();
	test_queue_destroy_not_empty();
	test_dequeue_size_zero();
	test_dequeue_string_array();
	test_queue_iteration();
	return 0;
}
