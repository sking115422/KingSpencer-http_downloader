#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

struct things {
    int val1 ;
    char * val2;
};

void* testFunc (void * arg) {

    struct things test = *(struct things *) arg;

    printf("Testing val1 %d\n", test.val1);
    printf("Testing val2 %s\n", test.val2);

}

int main () {
    
    struct things test[5];
    
    pthread_t threadArr [5];

   
    
    for (int i = 0; i < 5; i++)
    {
        test[i].val1 = i;

        test[i].val2 = "i like pie";

        printf("i %p\n", &i);

        pthread_create(&threadArr [i], NULL, (void *) testFunc, &test[i]);  
    }  

    for (int i = 0; i < 5; i++)
    {
        pthread_join(threadArr [i], NULL);  
    } 


    printf("\nprogram completed!\n");
    pthread_exit(NULL);

    return 0;
}



//gcc test.c -o thr -lpthread