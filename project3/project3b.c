#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#define N 5
#define M 7

int numofbluecars,numofredcars;
int j;

pthread_mutex_t mtx;
pthread_cond_t redcars,bluecars,giamain;

int inside;
int tocross_b = 0;
int tocross_r = 0;
int crossed;
int waiting_b = 0;
int waiting_r = 0;
int line = 0;
short int end = 0;

void *bluecar(void *arg){
    
    pthread_mutex_lock(&mtx);
    //printf
    if((tocross_b  + waiting_b >= M) || (waiting_b + inside >= N)){
        
        waiting_b++;
        
        pthread_cond_wait(&bluecars,&mtx);
    }else{
    
        inside++;
        tocross_b++;
    }
    
    if(inside < N && tocross_b < M && waiting_b > 0){
        
        tocross_b++;
        waiting_b--;
        inside++;
        pthread_cond_signal(&bluecars);
        
    }
    
    line++;
    
    printf("I am in(blue): %lu\n",pthread_self());
    sleep(1);
    printf("We are %d inside now(blue)\n",line);
    
    pthread_mutex_unlock(&mtx);
    
    printf("Crossing(blue): %lu\n",pthread_self());//crossing
    sleep(10);
    
    pthread_mutex_lock(&mtx);
    
    crossed++;
    line--;
    inside--;
    numofbluecars--;
    
    printf("I am out(blue)...I am the %d car that crossed in this turn: %lu \n",crossed,pthread_self());
    
    if(crossed == M || numofbluecars == 0){
        
        line = 0;
        inside = 0;
        tocross_r = 0;
        crossed = 0;
        
        if(numofredcars == 0){
            
            if(numofbluecars == 0){
                
                pthread_mutex_unlock(&mtx);
                
                end = 1;
                
                pthread_cond_signal(&giamain);
                
                return NULL;
            }
            
            tocross_b = 0;
            
            printf("There are no more cars from the other side...so press 'ENTER' to signal the same side to continue");
            
            getchar();
            
            //system("@cls || clear");
            
            if(waiting_b > 0){
                inside++;
                tocross_b++;
                waiting_b--;
                pthread_cond_signal(&bluecars);
            }
            
        }else{
          
            printf("Press 'ENTER' to signal the other side to begin:");
            getchar();
            
            //system("@cls || clear");
            if(waiting_r > 0){
                inside++;
                tocross_r++;
                waiting_r--;
                pthread_cond_signal(&redcars);
            }
        }
    }else{
    
        if(inside == N - 1 && tocross_b < M){
            
            if(waiting_b > 0){
                printf("Unblocking(blue)\n");
                waiting_b--;
                inside++;
                tocross_b++;
                pthread_cond_signal(&bluecars);
            }
        }
    }
    
    
    pthread_mutex_unlock(&mtx);
    
    return NULL;
}

void *redcar(void *arg){
    
    pthread_mutex_lock(&mtx);
    //printf
    if((tocross_r + waiting_r >= M) || (waiting_r + inside >= N)){
        
        waiting_r++;
        
        pthread_cond_wait(&redcars,&mtx);
    }else{
    
        inside++;
        tocross_r++;
    }
    
    if(inside < N && tocross_r < M && waiting_r > 0){
        
        tocross_r++;
        inside++;
        waiting_r--;
        pthread_cond_signal(&redcars);
    }
    
    line++;
    
    printf("I am in(red): %lu\n",pthread_self());
    
    sleep(1);
    
    printf("We are %d inside now(red)\n",line);
    
    pthread_mutex_unlock(&mtx);
    
    printf("Crossing(red): %lu\n",pthread_self());//crossing
    sleep(10);
    
    pthread_mutex_lock(&mtx);
    
    line--;
    crossed++;
    inside--;
    numofredcars--;
    
    printf("I am out(red)...I am the %d car that crossed in this turn: %lu \n",crossed,pthread_self());
    
    if(crossed == M || numofredcars == 0){
        
        line = 0;
        inside = 0;
        tocross_b = 0;
        crossed = 0;
        
        if(numofbluecars == 0){
            
            if(numofredcars == 0){
                
                pthread_mutex_unlock(&mtx);
                
                end = 1;
                
                pthread_cond_signal(&giamain);
                
                return NULL;
            }
            
            tocross_r = 0;
            
            printf("There are no more cars from the other side...so press 'ENTER' to signal the same side to continue");
            
            getchar();
            
            //system("@cls || clear");
            
            if(waiting_r > 0){
                inside++;
                tocross_r++;
                waiting_r--;
                pthread_cond_signal(&redcars);
            }
            
        }else{
          
            printf("Press 'ENTER' to signal the other side to begin:");
            getchar();
            
            //system("@cls || clear");
            
           if(waiting_b > 0){
                inside++;
                tocross_b++;
                waiting_b--;
                pthread_cond_signal(&bluecars);
            }
        }
    }else{
    
        if(inside == N - 1 && tocross_r < M){
            
            if(waiting_r > 0){
                printf("Unblocking(red)\n");
                waiting_r--;
                inside++;
                tocross_r++;
                pthread_cond_signal(&redcars);
            }
        }
    }
    pthread_mutex_unlock(&mtx);
    
    return NULL;
}

int main(){
    
    int i;
    pthread_t threadcar_b,threadcar_r;
    
    printf("Enter how many blue and then how many red:\n");
    scanf("%d",&numofbluecars);
    getchar();
    
    scanf("%d",&numofredcars);
    getchar();
    
    if(numofbluecars > numofredcars){
        
        tocross_r = M;
    }else{
        
        tocross_b = M;
    }
    
    for(i = 0; i < numofbluecars; i++){
        
        pthread_create(&threadcar_b,NULL,bluecar,NULL);
    }
    
    for(i = 0; i < numofredcars; i++){
        
        pthread_create(&threadcar_r,NULL,redcar,NULL);
    }
    
    pthread_mutex_lock(&mtx);
    
    if(!end){
        
        pthread_cond_wait(&giamain,&mtx);
    }
    
    pthread_mutex_unlock(&mtx);
    
    printf("Main exiting\n");
    
    
    return 0;
}

 
