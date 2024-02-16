#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <queue.h>
#include <string.h>

/* Test macro*/
#define TEST_ASSERT(assertion)						\
do {									\
	printf("ASSERT: " #assertion " ... ");			\
	if (assertion) {							\
		printf("PASS\n");						\
	} else {								\
		printf("FAIL\n");						\
		exit(1);							\
	}									\
} while(0)

/* Create test function */
void test_queue_create(void)
{
	fprintf(stderr, "*** TEST queue_create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple test function */
void test_simple_enqueue_dequeue(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST simple_enqueue_dequeue ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}

/* Test queue length */
void test_queue_length(void){
	int items[5] = {0, 1, 2, 3, 4};
	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST queue length ***\n");

	for (int i = 0; i < 5; i++) {
		queue_enqueue(queue, &items[i]);	
	}

	TEST_ASSERT(queue_length(queue) == 5);
}

/* Test queue length when queue is NULL */
void test_queue_null_length(void){
	queue_t queue = NULL;
	fprintf(stderr, "*** TEST null queue length ***\n");

	int result = queue_length(queue);
	TEST_ASSERT(result == -1);
}

/* Test deletion from queue */
void test_delete_item(void) 
{
	int items[6] = {0, 1, 2, 3, 4, 5};
	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST delete item from queue ***\n");

	for (int i = 0; i < 6; i++) {
		queue_enqueue(queue, &items[i]);	
	}

	queue_delete(queue, &items[1]);
	TEST_ASSERT(queue_length(queue) == 5);
}

/* Test deletion from queue and verify item not found */
void test_delete_item_not_found(void)
{
	int items[3] = {0, 1, 2};
	int not_found_value = 100;
	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST delete item not found ***\n");

	for (int i = 0; i < 3; i++) {
		queue_enqueue(queue, &items[i]);	
	}

	int result = queue_delete(queue, &not_found_value);
	TEST_ASSERT(result == -1);
}

/* Test queue destruction */
void test_queue_destruction(void)
{
	int items[3] = {0, 1, 2};
	int *ptr;

	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST queue destruction ***\n");

	for (int i = 0; i < 3; i++) {
		queue_enqueue(queue, &items[i]);	
	}

	for (int i = 0; i < 3; i++) {
		queue_dequeue(queue, (void**)&ptr);	
	}

	int result = queue_destroy(queue);
	TEST_ASSERT(result == 0);
}

/* Test queue destruction when queue is not empty */
void test_queue_destruction_not_empty(void){
	int items[3] = {0, 1, 2};

	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST queue destruction not empty ***\n");

	for (int i = 0; i < 3; i++) {
		queue_enqueue(queue, &items[i]);	
	}

	int result = queue_destroy(queue);
	TEST_ASSERT(result == -1);
}

/* Test dequeue operation on an empty queue */
void test_dequeue_empty_queue(void) {
	int *ptr;

	fprintf(stderr, "*** TEST dequeue empty queue ***\n");

	queue_t queue = queue_create();

	TEST_ASSERT(queue_dequeue(queue, (void**)&ptr) == -1);
}

/* Test dequeue operation on a queue containing string array */
void test_dequeue_string_array(void) {
	char** ptr;
	char* items[] = {"zero", "one", "two"};
	queue_t queue = queue_create();

	fprintf(stderr, "*** TEST dequeue string queue ***\n");
	
	for (int i = 0; i < 3; i++) {
		queue_enqueue(queue, &items[i]);	
	}

	queue_dequeue(queue, (void**)&ptr);

	TEST_ASSERT(!strcmp(*ptr, "zero"));
}

/* Callback function for queue_iterate test */
static void iterate_func(queue_t queue, void *data) {
	(void)queue; // Unused
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

    int items[3] = {0, 1, 2};

    for (int i = 0; i < 3; i++) {
        queue_enqueue(queue, &items[i]);    
    }

	fprintf(stderr, "*** TEST queue iteration***\n");
    test_iterate_data(queue, "0 1 2 ");
}


int main(void)
{
	test_queue_create();
	test_simple_enqueue_dequeue();
	test_queue_length();
	test_queue_null_length();
	test_delete_item();
	test_delete_item_not_found();
	test_queue_destruction();
	test_queue_destruction_not_empty();
	test_dequeue_empty_queue();
	test_dequeue_string_array();
	test_queue_iteration();
	return 0;
}