#include "queue.h"
#include <pthread.h>


pthread_mutex_t q_lock;
pthread_cond_t qEmpty;

void addq(int k, int id, struct work_queue* q){
	pthread_mutex_lock(&q_lock);
	 // Create a new LL node
    struct qnode *temp = new_node(k,id);
 
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL || q->front == NULL)
    {
       q->front = q->rear = temp;
       pthread_cond_broadcast(&qEmpty);
       pthread_mutex_unlock(&q_lock);
       return;

    }
     // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
		pthread_mutex_unlock(&q_lock);
		//It's making it through the Q, but it wont' recv data from socket struct!!!!!
		return;
	}
	




struct qnode* removeq(struct work_queue *q){
	//Bug is here. Thinks Q is empty...?
	//Doesn't come out of wait?! Why?

	pthread_mutex_lock(&q_lock);

	// If queue is empty, return NULL.
    while (q->front == NULL)
    {
	    printf("Thread is waiting for a insert into Q...\n");
    	pthread_cond_wait(&qEmpty, &q_lock);
 	}

    // Store previous front and move front one node ahead
    struct qnode *temp = q->front;
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
       q->rear = NULL;
    pthread_mutex_unlock(&q_lock);
    return temp;

}


struct qnode *new_node(int k, int id){
    struct qnode *temp = (struct qnode*)malloc(sizeof(struct qnode));
    temp->key = k;
    temp->id = id;
    temp->next = NULL;
    return temp; 
}

