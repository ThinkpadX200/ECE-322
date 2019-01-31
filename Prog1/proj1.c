/*
   Richard Coffey & Greg Pedone
   ECE 322
   Assignment 1
   proj1.c

    Written in C, with the standard compiler

    Compilation command I used: ./gcc -Wall -g ./proj1.c -o proj1
    Command I used to run: ./proj1

    You can omit the flag and naming if you want.

 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main() {

    int fd[2];
    int i,j;
    long long value;
    long long recValue;
    long long producedTotal = 0; 
    long long consumed50Total = 0;
    long long consumedMaxTotal = 0;
    int produced = 0;
    int producedAverage;
    int consumed50 = 0; 
    int consumed50Average;
    int consumedMax = 0; 
    int consumedMaxAverage;
    int readret = 1;

    pipe(fd);


    /*
    In the parent/producer block,close the input end, set the rand seed, produce the random 
    values with two for loops that generate 5000 iterations of 1-4 random numbers 
    between 0 and 1M. Write the numbers to the consumer,calculate the average, and 
    close the pipe.
    */

    if (fork() > 0) { //in parent/producer

        close(fd[0]);
        srand(time(NULL));

        for(i = 0; i < 5000; i++) {

            for(j = rand()%4 + 1; j>0; j--) {

                value = rand()%1000000 + 1;
                write(fd[1], &value, sizeof(value));
                produced++;
                producedTotal += value;
            }
        }
        
        producedAverage = producedTotal/produced;

        close(fd[1]);
        printf("Producer sent %d values with an average of %d\n", produced, producedAverage);
        printf("Pipe was closed\n");
    }


    /*
    In the child/consumer block, you close the producer end, and run an infinite while 
    loop to read in the values until it hits the producers EOF production. Break the
    loop to make sure you don't doubly record the last value. Calculate the small batch 
    average every 50 values consumed but store everything to calculate final averages.
    */

    else { //when you're in the child

        close(fd[1]);

        while(1) {

            readret = read(fd[0], &recValue, sizeof(recValue));

            if (readret == 0) {
                break;
            }            

            consumedMax++;
            consumedMaxTotal += recValue;

            consumed50++;
            consumed50Total += recValue;

            if(consumed50 == 50) {

                consumed50Average = consumed50Total/consumed50;

                printf("Consumer received 50 values with an average of %d\n", consumed50Average);

                consumed50Total = 0;
                consumed50 = 0;
            }

        }

        
        //pipe closed here

        //check to wrap up the last couple values if inner if loop didn't hit exact
        if (consumed50 < 50) {

            consumed50Average = consumed50Total/consumed50;

            printf("Consumer received %d values with an average of %d\n",consumed50, consumed50Average);

        }

        consumedMaxAverage = consumedMaxTotal/consumedMax;
        printf("Consumer received %d values with an average of %d\n\n", consumedMax, consumedMaxAverage);

    }

    return(0);

}




