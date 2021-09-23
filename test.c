#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int create_thread (char t_name [], t_funct()) {
    
    pthread_t t_name;
    pthread_t * p_t_name = &t_name;

    pthread_create(&p_t_name, NULL, t_funct, (void *) p_t_name);
    
    pthread_exit(NULL);
    
    return p_t_name;
}

//gcc test.c -o thr -lpthread