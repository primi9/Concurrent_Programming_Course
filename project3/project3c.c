 #include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define N 10

pthread_mutex_t mtx;
pthread_cond_t passengers,train,riding;

int j;
int wait = 0;
int line = 0;
int numofpassengers;
int inside = 0;
int out = 0;
short int endofride = 0;
short int left = 0;

void *ps_func(void *arg){
    //printf("I AM OUT\n");
    pthread_mutex_lock(&mtx);
    //printf("I AM IN\n");
    if(wait + inside >= N){
        
        wait++;
        
        pthread_cond_wait(&passengers,&mtx);
        
    }else{
        
        inside++;
    }
    
    line++;
    
    if(inside < N && wait > 0){
        
        inside++;
        wait--;
        pthread_cond_signal(&passengers);
    }
    
    if(numofpassengers < N) {
        
        printf("Not enougn passengers for the ride.Leaving...: %lu \n",pthread_self());
        
        if(numofpassengers == 1){
            
            //printf("Last one in...\n");
            
            left = 1;
            
            pthread_cond_signal(&train);
            
            pthread_mutex_unlock(&mtx);
            
            return NULL;
        }
        
        numofpassengers--;
        //printf("Pass: %d\n",numofpassengers);
        
        if(wait > 0){
            wait--;
            pthread_cond_signal(&passengers);
        }
        
        pthread_mutex_unlock(&mtx);
        
        return NULL;
    }
    
    printf("I am inside,passenger: %d: %lu \n",line,pthread_self());
    
    if(line == N){
        
        pthread_cond_signal(&train);
    }
    
    if(!endofride){
        
        pthread_cond_wait(&riding,&mtx);
    }
    
    numofpassengers--;
    
    out++;
    
    printf("I am out,passenger: %d %lu \n",out,pthread_self());
    
    if(out == N){
        
        pthread_cond_signal(&train);
    }else{
        
        pthread_cond_signal(&riding);
    }
    
    pthread_mutex_unlock(&mtx);
    
    return NULL;
}

void *train_func(void *arg){
    
    while(1){
        
        pthread_mutex_lock(&mtx);
        //inside = 0;
        if(numofpassengers < N ){
            
            printf("Train is closing...Not enough passengers\n");
            
            if(numofpassengers == 0){
                
                pthread_mutex_unlock(&mtx);
                
                return NULL;
            }
            
            if(wait > 0){
                wait--;
                pthread_cond_signal(&passengers);
            }
            
            if(!left){
                
                pthread_cond_wait(&train,&mtx);
            }
            
            pthread_mutex_unlock(&mtx);
            
            return NULL;
        }
        
        if(inside < N && wait > 0){
            
            wait--;
            inside++;
            pthread_cond_signal(&passengers);
        }
        
        if(line < N){
            
            pthread_cond_wait(&train,&mtx);
        }
        
        pthread_mutex_unlock(&mtx);
        
        printf("Press 'ENTER' when you want the ride to begin:");
        getchar();
        
        printf("Riding...\n");//rides
        
        sleep(5);
        
        pthread_mutex_lock(&mtx);
        
        endofride = 1;
        
        pthread_cond_signal(&riding);
        
        if(out < N){
            
            pthread_cond_wait(&train,&mtx);
        }
        
        endofride = 0;
        line = 0;
        out = 0;
        inside = 0;
        
        printf("Coming back...\n");//come back
        sleep(5);
        
        pthread_mutex_unlock(&mtx);
    }
}

int main(){
    
    int i;
    pthread_t thread_passenger,thread_train;
    
    printf("The number of the passengers:");
    scanf("%d",&numofpassengers);
    
    getchar() ;
    
    pthread_mutex_lock(&mtx);
    
    for(i = 0; i < numofpassengers; i++){
        pthread_create(&thread_passenger,NULL,ps_func,NULL);
    }
    
    pthread_create(&thread_train,NULL,train_func,NULL);
    
    pthread_mutex_unlock(&mtx);
    
    pthread_join(thread_train,NULL);
    
    printf("Main is exiting\n");
    
    return 0;    
}
 

