#ifndef PQ_H_
#define PQ_H_

typedef struct queue{
	int item[50];
	int count;
	unsigned int reader;
	unsigned int writer;
}queue;


typedef struct priority_queue {
	int count;
	queue q1;
	queue q2;
	queue q3;
	queue q4;
	queue q5;
	queue q6;
	queue q7;
	queue q8;
	queue q9;
	queue q10;
	queue q11;
	queue q12;
	queue q13;
	queue q14;
	queue q15;
	queue q16;
	queue q17;
	queue q18;
	queue q19;
	queue q20;
	queue q21;
	queue q22;
	queue q23;
	queue q24;
	queue q25;
	queue q26;
	queue q27;
	queue q28;
	queue q29;
	queue q30;
	queue q31;
	queue q32;
} priority_queue;


void queue_initial(queue *q);

void queue_push(queue *q, int item);

int queue_pop(queue *q);

void priority_queue_initial(priority_queue *q);

void priority_queue_push(priority_queue *q, int priority, int item);

int priority_queue_pop(priority_queue *q);

#endif
