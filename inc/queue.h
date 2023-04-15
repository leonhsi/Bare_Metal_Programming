#ifndef QUEUE_H
#define QUEUE_H

#define capacity 100

typedef struct{
    int front;
    int rear;
    int size;
    char array[capacity];
}Queue;

Queue* queue_init();
int isFull(Queue* queue);
int isEmpty(Queue* queue);
void enqueue(Queue* queue, char item);
char dequeue(Queue* queue);
int front(Queue* queue);
int rear(Queue* queue);

#endif