#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<pthread.h>
#ifdef _WIN32
#include<Windows.h>
#define sleep_milli(milli) Sleep(milli)
#define producer_printf printf
#define consumer_printf printf
#define UNUSED
#else
#include<unistd.h>
#define sleep_milli(milli) usleep((milli)*1000)
#define producer_printf(fmt, ...) printf("\033[92m" fmt "\033[0m", ##__VA_ARGS__)
#define consumer_printf(fmt, ...) printf("\033[94m" fmt "\033[0m", ##__VA_ARGS__)
#define UNUSED __attribute__((unused))
#endif

typedef struct {
	int id;
	int gram;
}Apple;

#define CQSIZE 10
typedef struct {
	Apple queue[CQSIZE + 1];
	int front;
	int rear;
}CirQueue;

#define CQ_CLEAR(cq) ((cq).front = (cq).rear = 0)
#define CQ_EMPTY(cq) ((cq).front == (cq).rear)
#define CQ_FULL(cq) ((cq).front == ((cq).rear + 1) % (CQSIZE + 1))
#define CQ_SIZE(cq) ((((cq).rear - (cq).front) + (CQSIZE + 1)) % (CQSIZE + 1))
#define CQ_ENQUEUE(cq, val) do{if (!CQ_FULL(cq)) {\
	(cq).queue[(cq).rear++] = val;\
	(cq).rear %= (CQSIZE + 1);\
	}}while(0)
#define CQ_DEQUEUE(cq, val) do{if (!CQ_EMPTY(cq)) {\
	val = (cq).queue[(cq).front++];\
	(cq).front %= (CQSIZE + 1);\
}}while(0)

CirQueue cq;
pthread_mutex_t mtx;
pthread_cond_t cond_empty, cond_full;

void *apple_producer(UNUSED void *arg) {
	static int count = 0;
	while (1) {
		Apple apple;
		apple.id = ++count;
		apple.gram = rand() % 150 + 150;
		pthread_mutex_lock(&mtx);
		while(CQ_FULL(cq)){
			producer_printf("producer: queue is full, waiting...\n");
			pthread_cond_wait(&cond_full, &mtx);
		}
		CQ_ENQUEUE(cq, apple);
		producer_printf("producer: produced apple %d, %d grams.\n", apple.id, apple.gram);
		pthread_cond_signal(&cond_empty);
		pthread_mutex_unlock(&mtx);
		sleep_milli(2000);
	}
	return NULL;
}

void *apple_consumer(void *arg) {
	int consumer_id = *(int*)arg;
	while(1){
		Apple apple;
		pthread_mutex_lock(&mtx);
		while(CQ_EMPTY(cq)) {
			consumer_printf("consumer %d: queue is empty, waiting...\n", consumer_id);
			pthread_cond_wait(&cond_empty, &mtx);
		}
		CQ_DEQUEUE(cq, apple);
		consumer_printf("consumer %d: consumed apple %d, %d grams.\n", consumer_id, apple.id, apple.gram);
		pthread_cond_signal(&cond_full);
		pthread_mutex_unlock(&mtx);
		sleep_milli(5000);
	}
	return NULL;
}

int main() {
	int i, consumer_ids[2] = {1, 2};
	pthread_t producer, consumers[2];
	srand((unsigned)time(NULL));
	CQ_CLEAR(cq);
	pthread_mutex_init(&mtx, NULL);
	pthread_cond_init(&cond_empty, NULL);
	pthread_cond_init(&cond_full, NULL);
	pthread_create(&producer, NULL, apple_producer, NULL);
	for (i = 0; i < 2; i++) {
		pthread_create(&consumers[i], NULL, apple_consumer, &consumer_ids[i]);
	}
	pthread_join(producer, NULL);
	for (i = 0; i < 2; i++) {
		pthread_join(consumers[i], NULL);
	}
    return 0;
}
