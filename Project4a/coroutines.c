#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include "lcoroutines.h"

struct co_t{
    
    ucontext_t cox;
    int id;    
};

ucontext_t ret_handler;
co_t *running;
co_t *my_main;
int counter = 0;

void ret_handling(){
    
    while(1){
        
        running = my_main;
        
        if(swapcontext(&ret_handler,&(my_main -> cox)) == -1){
            fprintf(stderr,"error\n");
            sleep(1);
        }
    }
}

void mycoroutines_destroy_libres(){
    
    free(ret_handler.uc_stack.ss_sp);
    free(my_main);
}

int mycoroutines_init(co_t *main_t){
    
    getcontext(&ret_handler);
    
    ret_handler.uc_stack.ss_sp = (char*)malloc(SIGSTKSZ);
    ret_handler.uc_stack.ss_size = SIGSTKSZ;
    
    makecontext(&ret_handler,ret_handling,0);
    
    my_main = main_t;
    
    my_main -> id = 0;
    counter++;
    
    //getcontext(&(my_main -> cox));
    running = my_main;
    return 0;
}

int mycoroutines_create(co_t *co, void(body)(),void *arg){
    
    getcontext(&(co -> cox));
    co -> id = counter;
    counter++;
    (co -> cox).uc_stack.ss_sp = (char*)malloc(SIGSTKSZ);
    (co -> cox).uc_stack.ss_size = SIGSTKSZ;
    (co -> cox).uc_link = &ret_handler;
    
    makecontext(&(co -> cox),body,0);
    return 0;
}

int mycoroutines_switchto(co_t *co){
    
    co_t *temp;
    
    temp = running;
    
    running = co;
    
    swapcontext(&(temp -> cox),&(running -> cox));
    return 0;
}

int mycoroutines_destroy(co_t *co){
    
    free((co -> cox).uc_stack.ss_sp);
    free(co);
    return 0;
}

int mycoroutines_self(){
    
    return(running -> id);
}
