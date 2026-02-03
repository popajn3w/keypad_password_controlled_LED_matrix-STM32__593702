#ifndef UINT16_QUEUE_H
#define UINT16_QUEUE_H

#include "stdbool.h"
#define MAX_SIZE 128

typedef struct
{
    uint16_t arr[MAX_SIZE];
    int16_t front;    // don't forget to initialize
    int16_t rear;     // with -1 at declaration to signal empty
} uint16_queue;

bool uint16_queue_isFull (uint16_queue *q);
bool uint16_queue_isEmpty (uint16_queue *q);
void uint16_queue_enqueue (uint16_queue *q, uint16_t data);
uint16_t uint16_queue_dequeue(uint16_queue *q);

#endif
