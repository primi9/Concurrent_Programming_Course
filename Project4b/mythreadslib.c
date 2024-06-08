#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include "mythreads.h"
#define SWAP_FREQUENCY 15

struct thr_t{
    
    ucontext_t cor;
    struct thr_t *next;
    struct thr_t *prev;
    struct thr_t *blocked_for;
    int id;    
};

struct sem_t{
    
    int waiting;
    int value;
    thr_t *blocked;
    
};

int counter = 1;
thr_t *running;
thr_t *my_main;
ucontext_t scheduler;
ucontext_t ret_handler;

struct sigaction sact;
struct itimerval it_val;
struct itimerval atomic;
sigset_t blocked_handler;

void catcher(int signum){
    
    swapcontext(&(running -> cor),&scheduler);
}

void enqueue_sched(thr_t *ptr){
    
    ptr -> next = running;
    ptr -> prev = running -> prev;
    
    running -> prev -> next = ptr;
    running -> prev = ptr;
    
}

void dequeue_sched(thr_t *ptr){
    
    if(ptr == running){
        
        running = running -> prev;
    }
    ptr -> prev -> next = ptr -> next;
    ptr -> next -> prev = ptr -> prev;
}

thr_t *search(thr_t *ptr){
    
    thr_t *temp;
    
    temp = running -> next;
    
    while(temp != running){
        
        if(temp == ptr){
            
            return temp;
        }
        temp = temp -> next;
    }
    return NULL;
}

void enqueue_join(thr_t *thr,thr_t *to_join){
    
    to_join -> next = NULL;
    
    if(thr -> blocked_for == NULL){
        
        thr -> blocked_for = to_join;
        to_join -> prev = NULL;
        return ;
    }
    
    thr_t *current = thr -> blocked_for;
    
    while(current -> next != NULL){
        
        current = current -> next;
    }
    current -> next = to_join;
    to_join -> prev = current;
}

void dequeue_sem(sem_t *s){
    
    thr_t *temp = s -> blocked;
    
    s -> blocked = temp -> next;
    
    enqueue_sched(temp);
}

void enqueue_sem(sem_t *s,thr_t *ptr){
    
    thr_t *current = s -> blocked;
    
    ptr -> next = NULL;
    
    if(s -> blocked == NULL){
        
        ptr -> prev = NULL;
        s -> blocked = ptr;
        return ;
    }
    
    while(current -> next != NULL){
        
        current = current -> next;
    }
    
    current -> next = ptr;
    ptr -> prev = current;
}

int mythreads_sem_init(sem_t **s,int val){
    
    *s = (sem_t*)malloc(sizeof(sem_t));
    
    (*s) -> value = val;
    
    (*s) -> waiting = 0;
    
    (*s) -> blocked = NULL;
    
    return 0;
}

void mythreads_sem_down(sem_t *s){
    
    setitimer(ITIMER_VIRTUAL, &atomic, NULL);
    
    thr_t *temp = running;
    
    if((s -> value) > 0){
        
        (s -> value)--;
        
        setitimer(ITIMER_VIRTUAL, &it_val, NULL);
        
        swapcontext(&(running -> cor),&scheduler);
        
        return ;
    }
    
    (s -> waiting) ++;
    
    dequeue_sched(temp);
    
    enqueue_sem(s,temp);
    
    setitimer(ITIMER_VIRTUAL, &it_val, NULL);
    
    swapcontext(&(temp -> cor),&scheduler);
    
    return ;
}

int mythreads_sem_up(sem_t *s){
    
    setitimer(ITIMER_VIRTUAL, &atomic, NULL);
    
    if(s -> value == 0 && s -> waiting > 0){
        
        dequeue_sem(s);
        
        (s -> waiting)--;
        
        setitimer(ITIMER_VIRTUAL, &it_val, NULL);
        
        swapcontext(&(running -> cor),&scheduler);
        
        return 0;
    }
    
    (s -> value)++;
    
    setitimer(ITIMER_VIRTUAL, &it_val, NULL);
    
    swapcontext(&(running -> cor),&scheduler);
    
    return 0;
}

int mythreads_sem_destroy(sem_t *s){
    
    if(s -> waiting > 0){
        
        return 1;
    }
    
    free(s);
    
    return 0;
}

void return_handler(){
    
    thr_t *current;
    thr_t *temp;
        
    while(1){
        
        current = running -> blocked_for;
        
        while(current != NULL){
            
            temp = current -> next;
            enqueue_sched(current);
            current = temp;
        }
        
        temp = running;
        
        running = running -> prev;
        
        dequeue_sched(temp);
        
        swapcontext(&ret_handler,&scheduler);
    }
}

void schedule(){
    
    while(1){
        
        running = running -> next;
        swapcontext(&scheduler,&(running -> cor));
    }
}

int mythreads_init(){
    
    sigemptyset(&sact.sa_mask);
    sigaddset(&sact.sa_mask,SIGVTALRM);
    sact.sa_flags = 0;
    sact.sa_handler = catcher;
    sigaction(SIGVTALRM,&sact,NULL);
    
    my_main = (thr_t*)malloc(sizeof(thr_t));
    
    getcontext(&(my_main -> cor));
    
    running = my_main;
    running -> next = my_main;
    running -> prev = my_main;
    running -> id = 0;
    running -> blocked_for = NULL;
    
    getcontext(&ret_handler);
    
    ret_handler.uc_stack.ss_sp = (char*)malloc(SIGSTKSZ);
    ret_handler.uc_stack.ss_size = SIGSTKSZ;
    ret_handler.uc_link = &(my_main -> cor);
    
    sigemptyset(&ret_handler.uc_sigmask);
    sigaddset(&ret_handler.uc_sigmask, SIGVTALRM);
    
    makecontext(&ret_handler,return_handler,0);
    
    getcontext(&scheduler);
    
    scheduler.uc_stack.ss_sp = (char*)malloc(SIGSTKSZ);
    scheduler.uc_stack.ss_size = SIGSTKSZ;
    scheduler.uc_link = &(my_main -> cor);
    
    sigemptyset(&scheduler.uc_sigmask);
    sigaddset(&scheduler.uc_sigmask, SIGVTALRM);
    
    makecontext(&scheduler,schedule,0);
    
    atomic.it_value.tv_sec = 0;
    atomic.it_value.tv_usec = 0;
    atomic.it_interval = atomic.it_value;
    
    it_val.it_value.tv_sec = 0;//INTERVAL / 100000;
    it_val.it_value.tv_usec = SWAP_FREQUENCY;//(INTERVAL*1000) % 1000000;   
    it_val.it_interval = it_val.it_value;
    setitimer(ITIMER_VIRTUAL, &it_val, NULL);//
    
    return 0;
}

int mythreads_create(thr_t **thr,void(*body)(void),void *arg){
    
    setitimer(ITIMER_VIRTUAL, &atomic, NULL);
    
    (*thr) = (thr_t*)malloc(sizeof(thr_t));
    
    (*thr) -> prev = NULL;
    (*thr) -> blocked_for = NULL;
    (*thr) -> next = NULL;
    (*thr) -> id = counter;
    counter ++;
    
    getcontext(&((*thr) -> cor));
    
    ((*thr) -> cor).uc_stack.ss_sp = (char*)malloc(SIGSTKSZ);
    ((*thr) -> cor).uc_stack.ss_size = SIGSTKSZ;
    ((*thr) -> cor).uc_link = &ret_handler;
    
    makecontext(&((*thr) -> cor),body,1,arg);
    
    enqueue_sched(*thr);
    
    setitimer(ITIMER_VIRTUAL, &it_val, NULL);
    
    return 0;
}

int mythreads_yield(){
    
    swapcontext(&(running -> cor),&scheduler);
    
    return 0;
}

int mythreads_join(thr_t *thr){
    
    setitimer(ITIMER_VIRTUAL, &atomic, NULL);
    
    thr_t *to_join = running;
    
    if(search(thr) != NULL){
        
        running = running -> prev;
        
        dequeue_sched(to_join);
        
        enqueue_join(thr,to_join);
        
        setitimer(ITIMER_VIRTUAL, &it_val, NULL);
        
        swapcontext(&(to_join -> cor),&scheduler);
        
        return 0;
    }else{
        
        setitimer(ITIMER_VIRTUAL, &it_val, NULL);
        
        return 1;
    }
}

int mythreads_destroy(thr_t *thr){
    
    setitimer(ITIMER_VIRTUAL, &atomic, NULL);
    
    thr_t *current = search(thr);
    
    if(current == NULL){
        
        setitimer(ITIMER_VIRTUAL, &it_val, NULL);
        
        swapcontext(&(running -> cor),&scheduler);
        
        return 1;
    }
    
    free((current -> cor).uc_stack.ss_sp);
    free(current);
    
    setitimer(ITIMER_VIRTUAL, &it_val, NULL);
    
    swapcontext(&(running -> cor),&scheduler);
    
    return 0;
}

void mythreads_destroy_libres(){
    
    setitimer(ITIMER_VIRTUAL,&atomic,NULL);
    
    free(ret_handler.uc_stack.ss_sp);
    free(scheduler.uc_stack.ss_sp);
    
}

int mythreads_self(){
    
    return (running -> id);
}