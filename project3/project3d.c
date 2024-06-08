#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define N 10

#define CCR_DECLARE(label)\
    \
    pthread_mutex_t label;\
    pthread_cond_t label ## queue1 , label ## queue2 , label ## w_queue;\
    
#define CCR_INIT(label)\
    int label ## rn1 = 0;\
    int label ## rn2 = 0;\
    
#define CCR_EXEC( label , cond , body )\
    \
    pthread_mutex_lock(&label);\
    if( label ## rn2 > 0 ){\
        \
        pthread_cond_wait(& label ## w_queue , &label);\
    }\
    \
    while(! ( cond )){\
        label ## rn1 ++;\
        if( label ## rn2 > 0 ){\
            \
            pthread_cond_signal(& label ## queue2 );\
        }else{\
            \
            pthread_cond_signal(& label ## w_queue );\
            \
        }\
        \
        pthread_cond_wait(& label ## queue1 , &label);\
        if( label ## rn1 > 0){\
            label ## rn1 --;\
            label ## rn2 ++;\
            pthread_cond_signal(& label ## queue1 );\
            pthread_cond_wait(& label ## queue2 , &label);\
        \
        }else if( label ## rn2 > 1){\
                pthread_cond_signal(& label ## queue2);\
                pthread_cond_wait(& label ## queue2 , &label);\
        }\
        label ## rn2 --;\
    }\
    body();\
    if( label ## rn1 > 0){\
        label ## rn1 --;\
        label ## rn2 ++;\
        pthread_cond_signal(& label ## queue1 );\
    }\
    else if( label ## rn2 > 0){\
        \
        pthread_cond_signal(& label ## queue2);\
    }\
    else{\
        pthread_cond_signal(& label ## w_queue);\
    }\
    pthread_mutex_unlock(&label);



CCR_DECLARE(mtx1)
CCR_INIT(mtx1)

int numofpassengers;
int inside = 0;
int out = 0;
short int endofride = 0;
int theend = 0;

int i;
pthread_t thread_passenger,thread_train;

void ps_enter(){
    
    if(numofpassengers < N){
        
        printf("Not enough passengers for the ride...Leaving: %lu \n",pthread_self());
        numofpassengers--;
        return ;
    }
    
    inside++;
    
    printf("I am inside:%d, %lu\n",inside , pthread_self());
}

void ps_exit(){
    
    out++;
    numofpassengers--;
    if(out == N){
        endofride = 0;
    }
    
    printf("I am out:%d, %lu\n",out , pthread_self());
}

void train_enter(){
    
    if(numofpassengers < N){
        
        if(!numofpassengers){
            theend = 1;
        }
        
        printf("Not enough passengers for the ride...Train is closing\n");
        return ;
    }
    
    printf("Press 'ENTER' when you want the ride to begin:");
    getchar();
    
    printf("RIDING\n");
    sleep(3);
    endofride = 1;
    
    return ;
}

void train_exit(){
    
    if(numofpassengers == 0){
        printf("COMING BACK\n");
        sleep(3);
        theend = 1;
        printf("Train exiting\n");
        return ;
    }
    
    printf("COMING BACK\n");
    sleep(3);
    inside = 0;
    out = 0;
}

void *ps_func(void *arg){
    
    CCR_EXEC(mtx1 , inside < N , ps_enter)
    CCR_EXEC(mtx1 , endofride , ps_exit)
    
    return NULL;
}

void *train_func(void *arg){
    
    while(1){
        CCR_EXEC(mtx1 , inside == N || numofpassengers < N, train_enter)
        if(theend) return NULL;
        CCR_EXEC(mtx1 , out == N || numofpassengers == 0 , train_exit)
        
        if(theend) return NULL;
    }
    return NULL;
}

void main_create(){
    
     for(i = 0; i < numofpassengers; i++){
        pthread_create(&thread_passenger,NULL,ps_func,NULL);
    }
    
    pthread_create(&thread_train,NULL,train_func,NULL);
    
}

int main(){
    
    printf("The number of the passengers:");
    scanf("%d",&numofpassengers);
    
    getchar() ;
    
    CCR_EXEC(mtx1, 1 , main_create)
    
    pthread_join(thread_train,NULL);
    
    printf("Main is exiting\n");
    
    return 0;    
}
 
 
