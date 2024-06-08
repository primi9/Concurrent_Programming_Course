 //Primikyris Athanasios 02615
#include <stdio.h>
#include <stdlib.h>
#include "mythreads.h"

int boss_flag;

sem_t *talking;
sem_t *worker;
sem_t *boss;

struct info{
    short int result;
    long unsigned int numtocheck;
    int flag;
};

typedef struct info info_t;

info_t *table;

void primetest(void *arg){
    
    /*if(argv == &table[0] || argv == &table[1] || argv == &table[2]){
        printf("wow\n");fflush(stdout);
    }*/
    
    info_t *ptr = arg;//deikse sta dedomena sou me ton info anti na kano typecast
    int i;//enas counter
    
    while(1){
        
        printf("Thread number: %d is examining number: %lu\n",mythreads_self(),ptr -> numtocheck);
        
        for(i = 2; i < ptr -> numtocheck; i++){//eksetazei an einai protos
            
            if(ptr -> numtocheck % i == 0){
                ptr -> result = 0;//den einai protos
                break;
            }
        }
        printf("Thread number: %d finished\n",mythreads_self());
        
        mythreads_sem_down(talking);
        
        boss_flag = ptr -> flag;//teleiosa
        
        mythreads_sem_up(boss);
        
        mythreads_sem_down(worker);
        
        mythreads_sem_up(talking);
        
        if(ptr -> flag == 0){
            printf("Thread number: %d returned\n",mythreads_self());
            return ;
        }
        
        ptr = &table[ptr -> flag];
        
        printf("Thread number: %d taken new number to check : %lu\n",mythreads_self(),ptr -> numtocheck);
    }
}

int main(int argc,char *argv[]){
    
    if(argc != 2){
        printf("Error.Wrong number of arguments.Exiting \n");
        return 0;
    }
    
    thr_t **vars;
    int numofthreads = atoi(argv[1]);//ta threads pou tha dhmiourgithoun
    int howmany,i,next,done = 0;//howmany posoi arithmoi gia eksetasi,i enas counter,next deixei sthn thesi tou pinaka opou vrisketai o epomenos arithmos(to struct tou) gia na paradothei ston proto diathesimo thread
    
    printf("Enter how many numbers: ");
    scanf("%d",&howmany);//diabazei posous arithnous gia eksetasi
    
    table = (info_t*)malloc(sizeof(info_t) * howmany);
    
    printf("Start entering numbers: \n");
    
    for(i = 0; i < howmany; i++){
        
        scanf("%lu",&table[i].numtocheck);
        table[i].result = 1;
        table[i].flag = i;
    }
    
    i = 0;
    
    mythreads_init();
    
    vars = (thr_t**)malloc(sizeof(thr_t*) * numofthreads);
    
    while(i < numofthreads){
        
        mythreads_create(&vars[i],&primetest,&table[i]);
        i++;
    }
    
    mythreads_sem_init(&boss,0);
    mythreads_sem_init(&worker,0);
    mythreads_sem_init(&talking,1);
    
    next = numofthreads;//o next deixnei sthn epomeni thesi gia eksetasi...thesi table[numofthreads].. 
    i = 0;
   
    while(1){
        
        mythreads_sem_down(boss);
        
        if(next == howmany){
            
            table[boss_flag].flag = 0; 
            
            done++;
            
            if(done == numofthreads){
                
                mythreads_sem_up(worker);
                
                break;
            }
            
        }else{
            
            table[boss_flag].flag = next;
            
            next++;
        }
        
        mythreads_sem_up(worker);
    }
    
    for(i = 0; i < howmany; i++){
        
        printf("Number : %lu ,result : %d\n",table[i].numtocheck,table[i].result);
    }
    
    for(i = 0; i < numofthreads; i++){
        
        mythreads_join(vars[i]);
    }
    
    if(mythreads_sem_destroy(boss) == 1)printf("boss problem\n");;
    if(mythreads_sem_destroy(worker) == 1)printf("worker problem\n");
    if(mythreads_sem_destroy(talking) == 1)printf("talking problem\n");
    
    i = 0;
    
    while(i < numofthreads){
        
        mythreads_destroy(vars[i]);
        i++;
    }
    
    free(vars);
    
    mythreads_destroy_libres(); 
    
    free(table);
    
    return 0;
}