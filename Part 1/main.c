#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

const int max = 10000000; //number of message to be sent
pthread_mutex_t lock;
int length = 0; //number of entries in the linked list
pthread_t tid[2];

struct node* head;
struct node* tail;

struct node
{
	struct node* next;
	int data;
};


void *consumer(void *vargp)
{
	int count = 0;
	while(count < max)
	{
	//consume messages (data from 0 to max-1, throw error if data out of order), invoke free on the head
	    if(length != 0){
		struct node* temp;
            pthread_mutex_lock(&lock);
            if(head->data != count) {printf("ERROR! data %d should be %d!\n", head->data, count);}

            temp = head->next;
            printf("Consumer : %d\n", head->data);
            free(head);
            head = temp;
            count++;
            length--;
            pthread_mutex_unlock(&lock);
	    }
	}
	return NULL;
}

void *producer(void *vargp)
{
	int count = 0;
	while(count < max)
	{
	//produce messages (data from 0 to max-1), malloc new tails
	    pthread_mutex_lock(&lock);
	    if(length == 0){
	        head = tail = malloc(sizeof(struct node));
	        head->data = count++;
	        head->next = NULL;
	    }
	    else{
            tail->next = malloc(sizeof(struct node));
            tail = tail->next;
            tail->data = count++;
            tail->next = NULL;
	    }
	    printf("Producer : %d\n", tail->data);
        length++;
	    pthread_mutex_unlock(&lock);
	}
	return NULL;
}

int main()
{
	pthread_mutex_init(&lock, NULL);
	pthread_create(&tid[0], NULL, &producer, NULL);
	pthread_create(&tid[1], NULL, &consumer, NULL);
	pthread_join(tid[1], NULL);
	pthread_join(tid[0], NULL);
	if(head != NULL) {printf("ERROR! List not empty\n");}
	exit(0);
}

/*
Useful commands:
pthread_mutex_init(&lock, NULL)
pthread_create(&tid[0], NULL, &producer, NULL);
pthread_join(&tid[1], NULL);
pthread_mutex_lock(&lock);
pthread_mutex_unlock(&lock);
*/
