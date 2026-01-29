#include "uint16_queue.h"

bool uint16_queue_isFull (uint16_queue *q)
{
    return (q->rear + 1) % MAX_SIZE == q->front;
}

bool uint16_queue_isEmpty (uint16_queue *q)
{
    return q->front == -1;
}

void uint16_queue_enqueue (uint16_queue *q, uint16_t data)
{
    // error if queue is full, cannot push
    if ((q->rear + 1) % MAX_SIZE == q->front)
    {
        //printf("Queue overflow\n");    // add error handling
        return;
    }

    // If the queue is empty, set the front to the first position
    if (q->front == -1)
        q->front = 0;

    // Add the data to the queue and move the rear pointer
    q->rear = (q->rear + 1) % MAX_SIZE;
    q->arr[q->rear] = data;
}

uint16_t dequeue(uint16_queue *q)
{
    // error if queue is empty, cannot pop
    if (q->front == -1)
    {
        //printf("Queue underflow\n");    // add error handling
        return -1;
    }

    uint16_t data = q->arr[q->front];
    if (q->front == q->rear)    // if last element
        q->front = q->rear = -1;
    else
        q->front = (q->front + 1) % MAX_SIZE;

    return data;
}

