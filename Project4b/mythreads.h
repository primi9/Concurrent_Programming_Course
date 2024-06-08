#ifndef _MYTHREADS_H_
#define _MYTHREADS_H_

typedef struct thr_t thr_t;

typedef struct sem_t sem_t;

int mythreads_init();

int mythreads_create(thr_t **thr,void (body)(),void *arg);

int mythreads_yield();

int mythreads_join(thr_t *thr);

int mythreads_self();

int mythreads_destroy(thr_t *thr);

int mythreads_sem_init(sem_t **s,int val);

void mythreads_sem_down(sem_t *s);

int mythreads_sem_up(sem_t *s);

int mythreads_sem_destroy(sem_t *s);

void mythreads_destroy_libres();

int threads_done();

#endif