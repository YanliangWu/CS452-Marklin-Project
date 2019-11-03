#include "priority_queue.h"
#include "bwio.h"


void queue_initial(queue *q){
    q->reader = 0;
    q->writer = 0;
    q->count = 0;
}


void queue_push(queue *q, int item){
    q->item[q->writer] = item;
    q->writer = (q->writer + 1)%50;
    q->count++;
}

int queue_pop(queue *q){
    int item = 0;
    if(q->count>0){
        item = q->item[q->reader];
        q->reader = (q->reader + 1)%50;
        q->count--;
    }
    //bwprintf(COM2,"pop:%d\n\r",item);
    return item;
}


void priority_queue_initial(priority_queue *q){
    q->count = 0;
    queue_initial(&(q->q1));
    queue_initial(&(q->q2));
    queue_initial(&(q->q3));
    queue_initial(&(q->q4));
    queue_initial(&(q->q5));
    queue_initial(&(q->q6));
    queue_initial(&(q->q7));
    queue_initial(&(q->q8));
    queue_initial(&(q->q9));
    queue_initial(&(q->q10));
    queue_initial(&(q->q11));
    queue_initial(&(q->q12));
    queue_initial(&(q->q13));
    queue_initial(&(q->q14));
    queue_initial(&(q->q15));
    queue_initial(&(q->q16));
    queue_initial(&(q->q17));
    queue_initial(&(q->q18));
    queue_initial(&(q->q19));
    queue_initial(&(q->q20));
    queue_initial(&(q->q21));
    queue_initial(&(q->q22));
    queue_initial(&(q->q23));
    queue_initial(&(q->q24));
    queue_initial(&(q->q25));
    queue_initial(&(q->q26));
    queue_initial(&(q->q27));
    queue_initial(&(q->q28));
    queue_initial(&(q->q29));
    queue_initial(&(q->q30));
    queue_initial(&(q->q31));
    queue_initial(&(q->q32));
}

void priority_queue_push(priority_queue *q, int priority, int item){
    switch(priority){
        case 1:
            queue_push(&(q->q1),item);
        break;
        case 2:
            queue_push(&(q->q2),item);
        break;
        case 3:
            queue_push(&(q->q3),item);
        break;
        case 4:
            queue_push(&(q->q4),item);
        break;
        case 5:
            queue_push(&(q->q5),item);
        break;
        case 6:
            queue_push(&(q->q6),item);
        break;
        case 7:
            queue_push(&(q->q7),item);
        break;
        case 8:
            queue_push(&(q->q8),item);
        break;
        case 9:
            queue_push(&(q->q9),item);
        break;
        case 10:
            queue_push(&(q->q10),item);
        break;
        case 11:
            queue_push(&(q->q11),item);
        break;
        case 12:
            queue_push(&(q->q12),item);
        break;
        case 13:
            queue_push(&(q->q13),item);
        break;
        case 14:
            queue_push(&(q->q14),item);
        break;
        case 15:
            queue_push(&(q->q15),item);
        break;
        case 16:
            queue_push(&(q->q16),item);
        break;
        case 17:
            queue_push(&(q->q17),item);
        break;
        case 18:
            queue_push(&(q->q18),item);
        break;
        case 19:
            queue_push(&(q->q19),item);
        break;
        case 20:
            queue_push(&(q->q20),item);
        break;
        case 21:
            queue_push(&(q->q21),item);
        break;
        case 22:
            queue_push(&(q->q22),item);
        break;
        case 23:
            queue_push(&(q->q23),item);
        break;
        case 24:
            queue_push(&(q->q24),item);
        break;
        case 25:
            queue_push(&(q->q25),item);
        break;
        case 26:
            queue_push(&(q->q26),item);
        break;
        case 27:
            queue_push(&(q->q27),item);
        break;
        case 28:
            queue_push(&(q->q28),item);
        break;
        case 29:
            queue_push(&(q->q29),item);
        break;
        case 30:
            queue_push(&(q->q30),item);
        break;
        case 31:
            queue_push(&(q->q31),item);
        break;
        case 32:
            queue_push(&(q->q32),item);
        break;
    }
    q->count++;
}

int priority_queue_pop(priority_queue *q){
    if(q->count>0){
        q->count--;
        if(q->q1.count>0)
            return queue_pop(&(q->q1));
        if(q->q2.count>0)
            return queue_pop(&(q->q2));
        if(q->q3.count>0)
            return queue_pop(&(q->q3));
        if(q->q4.count>0)
            return queue_pop(&(q->q4));
        if(q->q5.count>0)
            return queue_pop(&(q->q5));
        if(q->q6.count>0)
            return queue_pop(&(q->q6));
        if(q->q7.count>0)
            return queue_pop(&(q->q7));
        if(q->q8.count>0)
            return queue_pop(&(q->q8));
        if(q->q9.count>0)
            return queue_pop(&(q->q9));
        if(q->q10.count>0)
            return queue_pop(&(q->q10));
        if(q->q11.count>0)
            return queue_pop(&(q->q11));
        if(q->q12.count>0)
            return queue_pop(&(q->q12));
        if(q->q13.count>0)
            return queue_pop(&(q->q13));
        if(q->q14.count>0)
            return queue_pop(&(q->q14));
        if(q->q15.count>0)
            return queue_pop(&(q->q15));
        if(q->q16.count>0)
            return queue_pop(&(q->q16));
        if(q->q17.count>0)
            return queue_pop(&(q->q17));
        if(q->q18.count>0)
            return queue_pop(&(q->q18));
        if(q->q19.count>0)
            return queue_pop(&(q->q19));
        if(q->q20.count>0)
            return queue_pop(&(q->q20));
        if(q->q21.count>0)
            return queue_pop(&(q->q21));
        if(q->q22.count>0)
            return queue_pop(&(q->q22));
        if(q->q23.count>0)
            return queue_pop(&(q->q23));
        if(q->q24.count>0)
            return queue_pop(&(q->q24));
        if(q->q25.count>0)
            return queue_pop(&(q->q25));
        if(q->q26.count>0)
            return queue_pop(&(q->q26));
        if(q->q27.count>0)
            return queue_pop(&(q->q27));
        if(q->q28.count>0)
            return queue_pop(&(q->q28));
        if(q->q29.count>0)
            return queue_pop(&(q->q29));
        if(q->q30.count>0)
            return queue_pop(&(q->q30));
        if(q->q31.count>0)
            return queue_pop(&(q->q31));
        if(q->q32.count>0)
            return queue_pop(&(q->q32));
     }
	 return 0;
}

