 //Primikyris Athanasios 02615
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int boss_flag;

short int finished = 0;
short int end = 0;

pthread_mutex_t mtx_threads,mtx_main;
pthread_cond_t boss_worker;

struct info{
    short int result;
    long unsigned int numtocheck;
    int flag;
};

typedef struct info info_t;

info_t *table;

void *primetest(void *arg){
    
    info_t *ptr = arg;//deikse sta dedomena sou me ton info anti na kano typecast
    int i;//enas counter
    
    while(1){
        
        printf("Thread number: %lu is examining number: %lu\n",pthread_self(),ptr -> numtocheck);
        
        for(i = 2; i < ptr -> numtocheck; i++){//eksetazei an einai protos
            
            if(ptr -> numtocheck % i == 0){
                ptr -> result = 0;//den einai protos
                break;
            }
        }
        
        pthread_mutex_lock(&mtx_threads);
        
        pthread_mutex_lock(&mtx_main);
        
        boss_flag = ptr -> flag;//teleiosa
        
        finished = 1;
        
        pthread_cond_signal(&boss_worker);
        
        if(!end){
            
            pthread_cond_wait(&boss_worker,&mtx_main);
        }
        
        end = 0;
        //printf("Count is %lu\n",count);
        pthread_mutex_unlock(&mtx_main);
        
        pthread_mutex_unlock(&mtx_threads);
        
        if(ptr -> flag == 0){
            
            return NULL;
        }
        
        ptr = &table[ptr -> flag];
        
        printf("Thread number: %lu taken new number to check : %lu\n",pthread_self(),ptr -> numtocheck);
        
    }
}

int main(int argc,char *argv[]){
    
       if(argc != 2){
        printf("Error.Wrong number of arguments.Exiting \n");
        return 0;
    }
    
    int numofthreads = atoi(argv[1]);//ta threads pou tha dhmiourgithoun
    int howmany,i,next,done = 0;//howmany posoi arithmoi gia eksetasi,i enas counter,next deixei sthn thesi tou pinaka opou vrisketai o epomenos arithmos(to struct tou) gia na paradothei ston proto diathesimo thread
    pthread_t thread;
    
    printf("Enter how many numbers: ");
    scanf("%d",&howmany);//diabazei posous arithnous gia eksetasi
    
    table = (info_t*)malloc(sizeof(info_t) * howmany);
    
    printf("Start entering numbers: \n");
    
    for(i = 0; i < howmany; i++){
        
        scanf("%lu",&table[i].numtocheck);
        table[i].result = 1;
        table[i].flag = i;
    }
    
    for(i = 0; i < numofthreads; i++){//dhmiourgei ta threads
        
        pthread_create(&thread,NULL,primetest,&table[i]);
    }
    
    next = i;//o next deixnei sthn epomeni thesi gia eksetasi...thesi table[numofthreads].. 
    i = 0;
   
    while(1){
        
        pthread_mutex_lock(&mtx_main);
        
        if(!finished){
            
            pthread_cond_wait(&boss_worker,&mtx_main);
        }
        
        finished = 0;
        
        if(next == howmany){
            
            table[boss_flag].flag = 0; 
            
            done++;
            
            if(done == numofthreads){
                
                end = 1;
                
                pthread_cond_signal(&boss_worker);
                
                pthread_mutex_unlock(&mtx_main);
                
                break;
            }
            
        }else{
            
            table[boss_flag].flag = next;
            
            next++;
        }
        
        end = 1;
        
        pthread_cond_signal(&boss_worker);
        
        pthread_mutex_unlock(&mtx_main);
    }
    
    for(i = 0; i < howmany; i++){
        
        printf("Number : %lu ,result : %d\n",table[i].numtocheck,table[i].result);
    }
    
    free(table);
    
    return 0;
}

 
 
