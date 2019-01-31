/********************************************/
/* Name: Richard Coffey and Greg Pedone     */
/* Project: CPSC 3220 Lock Project          */
/* Due: April 25, 2018                      */
/*                                          */
/* Compile: gcc ./lock.c -o ./lock -pthread */
/* Run: ./lock                              */
/********************************************/

#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int carbon, hydrogen, oxygen;
int quit;
int carbon_prod, hydrogen_prod, oxygen_prod;
int ethanol_prod, water_prod, ozone_prod;

/*HARDCODED molecule.c VALUES*/
/*****************************/
//molecules per iteration
int carbon_per_iter = 2;
int hydrogen_per_iter = 4; 
int oxygen_per_iter = 2;

//hardcoded sleep  values
//since usleep uses uSecs, I multiplied the hardcoded values by 1000
//in order to make them into mSecs
useconds_t carbon_sleep = 5000;
useconds_t hydrogen_sleep = 2500;
useconds_t oxygen_sleep = 1000;
useconds_t ethanol_sleep = 2000;
useconds_t water_sleep = 2000;
useconds_t ozone_sleep = 2000;

//needed molecules
int ethanol_required = 100;
int water_required = 100;
int ozone_required = 100;
/*****************************/


//oxygen producer, hydrogen producer, carbon producer
//oxygen consumer, hydrogen consumer, carbon consumer
pthread_t op, hp, cp, oc, wc, ec; 

//condition variables
pthread_cond_t h_cv, c_cv, o_cv;

//locks
pthread_mutex_t hl, cl, ol, consumer_lock;


/*
Producer functions:
- these functions run while quit == 0
- secure the lock for their intended producer
- generate a randomly generated amount of atoms based on hardcoded values
- then let go of lock, signal condition variable, and sleep
- also print out the number of atoms they created
*/
void *carbon_p (void *arg) { 
    int n;

    while (!quit) {
        pthread_mutex_lock(&cl);
        
        n = (rand()% carbon_per_iter) + 1;
        carbon += n;
        carbon_prod += n;
        printf("Carbon producer produced %d Carbons: C = %d\n", n, carbon);
        
        pthread_cond_signal(&c_cv);
        pthread_mutex_unlock(&cl);
        usleep(carbon_sleep);
    }
}

void *hydrogen_p (void *arg) { 
    int n;

    while (!quit) {
        pthread_mutex_lock(&hl);
        
        n = (rand()% hydrogen_per_iter) + 1;
        hydrogen += n;
        hydrogen_prod += n;
        printf("Hydrogen producer produced %d Hydrogen: H = %d\n", n, hydrogen);
        
        pthread_cond_signal(&h_cv);
        pthread_mutex_unlock(&hl);
        usleep(hydrogen_sleep);
    }
}

void *oxygen_p (void *arg) { 
    int n;

    while (!quit) {
        pthread_mutex_lock(&ol);
        
        n = (rand()% oxygen_per_iter) + 1;
        oxygen += n;
        oxygen_prod += n;
        printf("Oxygen producer produced %d Oxygen: O = %d\n", n, oxygen);
        
        pthread_cond_signal(&o_cv);
        pthread_mutex_unlock(&ol);
        usleep(oxygen_sleep);
    }
}


/*
Consumer functions:
- these functions run until they create a certain number of molecules based on hardcoded values
- secure the lock for their chemical components PLUS the consumer lock
    - secures one at a time
- if an atoms is needed, it will wait until a producer signals that it made some
    - waits for the atoms on an individual basis
- adjusts the statistics and updates values to be printed out
- then let go of atom locks and consumer locks IN REVERSE ORDER and sleeps
*/
void *ozone_consumer(void *arg) {
    int n = 0;
    
    while (n < ozone_required) {
        pthread_mutex_lock(&consumer_lock);
        pthread_mutex_lock(&ol);
        while (oxygen < 3) {
            pthread_cond_wait(&o_cv, &ol);
        }
    
        oxygen -= 3;
        ozone_prod += 1; 
        printf("Ozone consumer consumed O3: O = %d\n", oxygen);
        n++;
        pthread_mutex_unlock(&ol);
        pthread_mutex_unlock(&consumer_lock);
        usleep(ozone_sleep);

    }
}


void *water_consumer(void *arg) {
    int n = 0;
    
    while (n < water_required) {
        pthread_mutex_lock(&consumer_lock);
        pthread_mutex_lock(&ol);
        while (oxygen < 1) {
            pthread_cond_wait(&o_cv, &ol);
        }
    
        pthread_mutex_lock(&hl);
        while (hydrogen < 2) {
            pthread_cond_wait(&h_cv, &hl);
        }

        oxygen -= 1;
        hydrogen -= 2;
        water_prod += 1;
        printf("Water consumer consumed H2O: O = %d, H = %d\n", oxygen, hydrogen);
        n++;
        pthread_mutex_unlock(&hl);
        pthread_mutex_unlock(&ol);
        pthread_mutex_unlock(&consumer_lock);
        usleep(water_sleep);

    }
}


void *ethanol_consumer(void *arg) {
    int n = 0;
    
    while (n < ethanol_required) {
        pthread_mutex_lock(&consumer_lock);
        pthread_mutex_lock(&ol);
        while (oxygen < 1) {
            pthread_cond_wait(&o_cv, &ol);
        }
    
        pthread_mutex_lock(&cl);
        while (carbon < 2) {
            pthread_cond_wait(&c_cv, &cl);
        }
    
        pthread_mutex_lock(&hl);
        while (hydrogen < 6) {
            pthread_cond_wait(&h_cv, &hl);
        }
        
        oxygen -= 1;
        carbon -= 2;
        hydrogen -= 6;
        ethanol_prod += 1;
        printf("Ethanol consumer consumed C2H6O: O = %d, H = %d, C = %d\n", oxygen, hydrogen, carbon);
        n++;
        pthread_mutex_unlock(&hl);
        pthread_mutex_unlock(&cl);
        pthread_mutex_unlock(&ol);
        pthread_mutex_unlock(&consumer_lock);
        usleep(ethanol_sleep);
    }
}


int main () {

    //keeping track of program run time
    clock_t begin = clock();
    

    //init mutex
    pthread_mutex_init(&ol, NULL);
    pthread_mutex_init(&hl, NULL);
    pthread_mutex_init(&cl, NULL);
    pthread_mutex_init(&consumer_lock, NULL);

    quit = 0;

    //creating producers
    pthread_create(&hp, NULL, &hydrogen_p, NULL);
    pthread_create(&cp, NULL, &carbon_p, NULL);
    pthread_create(&op, NULL, &oxygen_p, NULL);
    
    //creating consumers
    pthread_create(&ec, NULL, &ethanol_consumer, NULL);
    pthread_create(&wc, NULL, &water_consumer, NULL);
    pthread_create(&oc, NULL, &ozone_consumer, NULL);

    //waiting for consumer threads to finish
    pthread_join(oc, NULL);
    pthread_join(wc, NULL);
    pthread_join(ec, NULL);

    quit = 1;
    
    //waits for producer threads to finish
    pthread_join(cp, NULL);
    pthread_join(hp, NULL);
    pthread_join(op, NULL);
    
    clock_t end = clock();
    double time_spent = ((double)(end - begin) / CLOCKS_PER_SEC) * 1000;


    printf("Carbon Atoms produced = %d\n", carbon_prod);    
    printf("Hydrogen Atoms produced = %d\n", hydrogen_prod);    
    printf("Oxygen Atoms produced = %d\n", oxygen_prod);    
    printf("%d Ethanol molecules produced\n", ethanol_prod);    
    printf("%d Water molecules produced\n", water_prod);    
    printf("%d Ozone molecules produced\n", ozone_prod);    

    printf("Time to finish execution: %0.2fmSec\n", time_spent);

}








