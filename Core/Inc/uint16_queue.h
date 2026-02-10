#ifndef UINT16_QUEUE_H
#define UINT16_QUEUE_H

#include "stdint.h"
#include "stdbool.h"
#define MAX_SIZE 128

typedef struct
{
    uint16_t arr[MAX_SIZE];
    int16_t front;    // don't forget to initialize
    int16_t rear;     // with -1 at declaration to signal empty
} uint16_queue;

bool uint16_queue_isFull (volatile uint16_queue *q);
bool uint16_queue_isEmpty (volatile uint16_queue *q);
void uint16_queue_enqueue (volatile uint16_queue *q, uint16_t data);
uint16_t uint16_queue_dequeue(volatile uint16_queue *q);

#endif
