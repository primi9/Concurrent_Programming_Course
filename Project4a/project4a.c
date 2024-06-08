#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "lcoroutines.h"
#include <unistd.h>

struct co_t{
    
    ucontext_t cox;
    int id;    
};

co_t* formain;
char buffer[64];
int w_index = 0;
int r_index = 0;
short int end = 0;
co_t *consumer;
co_t *producer;

void produce(){
    
    char toread;
    
    while(1){
    
        while(w_index < 64){
        
            fread(&toread,1,1,stdin);
            
            if(feof(stdin)){
                
                end = 1;
                
                #ifdef PRINT
                
                    fprintf(stderr,"switching\n");
                #endif
                mycoroutines_switchto(consumer);
                #ifdef PRINT
                    fprintf(stderr,"Exiting producer: %d\n",mycoroutines_self());
                #endif
                return ;
            }
            
            buffer[w_index] = toread;
            
            w_index++;
        }
        #ifdef PRINT
            fprintf(stderr,"switching , id of running before the switch is %d\n",mycoroutines_self());
            sleep(1);
        #endif
        mycoroutines_switchto(consumer);
        w_index = 0;
    }
}

void consume(){
    
    while(1){
        
        while(r_index != w_index){
            
            fwrite(&buffer[r_index],1,1,stdout);
            
            r_index++;
        }
        
        if(end){
            
            #ifdef PRINT
                fprintf(stderr,"Exiting consumer: %d\n",mycoroutines_self());
            #endif
            return ;
        }
        #ifdef PRINT
            fprintf(stderr,"switching , id of running before the switch is %d\n",mycoroutines_self());
            sleep(1);
        #endif
        
        mycoroutines_switchto(producer);
        r_index = 0;
    }
}

int main(int argc ,char *argv[]){
    
    formain = (co_t*)malloc(sizeof(co_t));
    producer = (co_t*)malloc(sizeof(co_t));
    consumer = (co_t*)malloc(sizeof(co_t));
    
    mycoroutines_init(formain);
    
    mycoroutines_create(consumer,consume,NULL);
    mycoroutines_create(producer,produce,NULL);
    
    #ifdef PRINT
        fprintf(stderr,"switching , id of running before the switch is %d\n",mycoroutines_self());
        sleep(1);
    #endif
    
    mycoroutines_switchto(producer);
    
    #ifdef PRINT
        fprintf(stderr,"switching , id of running before the switch is %d\n",mycoroutines_self());
        sleep(1);
    #endif
    
    mycoroutines_switchto(producer);
    
    mycoroutines_destroy(producer);
    mycoroutines_destroy(consumer);
    
    mycoroutines_destroy_libres();
    #ifdef PRINT
        fprintf(stderr,"This is the end\n");
    #endif
    return 0;
}
