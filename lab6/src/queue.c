#include "utils.h"
#include "queue.h"

Queue* queue_init(){
    Queue* queue = (Queue*)simple_malloc(sizeof(Queue));
    queue->front = 0;
    queue->size = 0;
    queue->rear = capacity-1;

    return queue;
}

int isFull(Queue* queue){
    return queue->size == capacity;
}

int isEmpty(Queue* queue){
    return queue->size == 0;
}

void enqueue(Queue* queue, char item){
    queue->rear = (queue->rear + 1) % capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size+1;
}

char dequeue(Queue* queue){
    if(isEmpty(queue)) return '\0';

    char item = queue->array[queue->front];
    queue->front = (queue->front+1) % capacity;
    queue->size = queue->size - 1;

    return item;
}

int front(Queue* queue){
    if(isEmpty(queue)) return 0;

    return queue->array[queue->front];
}

int rear(Queue* queue){
    if(isEmpty(queue)) return 0;
    return queue->array[queue->rear];
}