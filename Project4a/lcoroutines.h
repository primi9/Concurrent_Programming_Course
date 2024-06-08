#ifndef _LCOROUTINES_H_
#define _LCOROUTINES_H_

typedef struct co_t co_t;

int mycoroutines_init(co_t *main_t);

int mycoroutines_create(co_t *co, void(body)(),void *arg);

int mycoroutines_switchto(co_t *co);

int mycoroutines_destroy(co_t *co);

int mycoroutines_self();

void mycoroutines_destroy_libres();

#endif 
