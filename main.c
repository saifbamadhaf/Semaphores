#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> // for sleep()


#define NUM_CARTS 20
#define NUM_CASHIERS 3
#define NUM_TERMINAL 2

sem_t cartSemaphore;
sem_t terminalSemaphore;
sem_t terminalSemaphorequeue;
sem_t terminal_Semaphores[NUM_TERMINAL];
sem_t terminal_queue_mutex[NUM_TERMINAL];
sem_t terminal_ready[NUM_TERMINAL];
sem_t cashiers_Semaphores[NUM_CASHIERS];
sem_t cashier_queue_mutex[NUM_CASHIERS];
sem_t cashier_ready[NUM_CASHIERS];

int queue = 0;
int NUM_CUSTOMERS = 20;
int carts_available = NUM_CARTS;
int scannersAvailable = 10;
int cashier_queue[NUM_CASHIERS] = {0};
int scanner_queue[NUM_CASHIERS] = {0};

void *customer(void *arg);
void *customerScanner(void *arg);
void shopping(int customer_id);
void checkout(int customer_id, int chosen_cashier);
void checkoutScanner(int customer_id, int chosen_terminal);
void return_cart(int customer_id);



void *customerScanner(void *arg) {
    int customer_id = *((int *) arg);

    // Customer arrives and tries to take a cart
    sem_wait(&terminalSemaphore);
    if (scannersAvailable > 0) {
        scannersAvailable--;
        printf("Customer [%d] takes a handheld scanner. Handheld scanners available: %d\n", customer_id, scannersAvailable);
        sem_post(&terminalSemaphore);

        // Customer starts shopping
        shopping(customer_id);

        // After shopping, proceed to checkout
        int chosen_terminal = -1;

        // Choose the terminal with the no queue
        for (int i = 0; i < 1; i++) {
            sem_wait(&terminal_Semaphores[i]);

            // Check if current cashier's queue is smaller or non-existent
            if (scanner_queue[i] == 0) {
                chosen_terminal = i;
                scanner_queue[chosen_terminal]++;
            } else if (scanner_queue[i + 1] == 0) {
                chosen_terminal = i + 1;
                scanner_queue[chosen_terminal]++;
            } else {
                queue++;
                printf("Customer [%d] has joined the queue. The current queue is [%d].\n", customer_id, queue);
            }

            sem_post(&terminal_Semaphores[i]); // Release semaphore for each cashier
        }

        if (chosen_terminal == -1) {
            sem_wait(&terminalSemaphorequeue);
            while (1) {
                if (scanner_queue[0] == 0) {
                    chosen_terminal = 0;
                    scanner_queue[chosen_terminal]++;
                    queue--;
                    sem_post(&terminalSemaphorequeue);
                    break;
                } else if (scanner_queue[1] == 0) {
                    chosen_terminal = 1;
                    queue--;
                    scanner_queue[chosen_terminal]++;
                    sem_post(&terminalSemaphorequeue);
                    break;
                }
            }
        }

            sem_wait(&terminal_queue_mutex[chosen_terminal]);
            printf("Customer [%d] is checking out at terminal [%d].\n", customer_id, chosen_terminal);
            sem_post(&terminal_queue_mutex[chosen_terminal]);

            // Wait for the chosen cashier to signal the customer's turn
            sem_wait(&terminal_ready[chosen_terminal]);

            // Simulate checkout time
            checkoutScanner(customer_id, chosen_terminal);

            // Customer finishes checkout, signal to the next customer in line
            sem_post(&terminal_ready[chosen_terminal]);

            // Update cashier queue
            sem_wait(&terminal_queue_mutex[chosen_terminal]);
            printf("Customer [%d] checking at terminal [%d] has checked out.\n", customer_id, chosen_terminal);
            scanner_queue[chosen_terminal]--;
            sem_post(&terminal_queue_mutex[chosen_terminal]);

            // Return the scanner and go home
            sem_wait(&terminalSemaphore);
            scannersAvailable++;
            printf("Customer [%d] returned the scanner. Scanners available: %d\n", customer_id, scannersAvailable);
            sem_post(&terminalSemaphore);

        }

        pthread_exit(NULL);
    }

    void *customer(void *arg) {
        int customer_id = *((int *) arg);

        // Customer arrives and tries to take a cart
        sem_wait(&cartSemaphore);
        if (carts_available > 0) {
            carts_available--;
            printf("Customer [%d] takes a cart. Carts available: %d\n", customer_id, carts_available);
            sem_post(&cartSemaphore);

            // Customer starts shopping
            shopping(customer_id);

            // After shopping, proceed to checkout
            int chosen_cashier = 0;

            // Choose the cashier with the smallest or no queue
            for (int i = 1; i < NUM_CASHIERS; i++) {
                sem_wait(&cashiers_Semaphores[i]); // Wait on semaphore for each cashier

                // Check if current cashier's queue is smaller or non-existent
                if (cashier_queue[i] < cashier_queue[chosen_cashier]) {
                    chosen_cashier = i;
                }

                sem_post(&cashiers_Semaphores[i]); // Release semaphore for each cashier
            }

            // Join the chosen cashier's queue
            sem_wait(&cashier_queue_mutex[chosen_cashier]);
            cashier_queue[chosen_cashier]++;
            if(cashier_queue[chosen_cashier] - 1 == 0){
                printf("Customer [%d] is checking out at Cashier [%d].\n", customer_id, chosen_cashier);
            } else{
                printf("Customer [%d] is checking out at Cashier [%d] and his turn is after %d customer.\n", customer_id, chosen_cashier,
                       cashier_queue[chosen_cashier] - 1);
            }

            sem_post(&cashier_queue_mutex[chosen_cashier]);

            // Signal the chosen cashier that a customer is ready
            // sem_post(&cashier_ready[chosen_cashier]);

            // Wait for the chosen cashier to signal the customer's turn
            sem_wait(&cashier_ready[chosen_cashier]);

            // Simulate checkout time
            checkout(customer_id, chosen_cashier);

            // Customer finishes checkout, signal to the next customer in line
            sem_post(&cashier_ready[chosen_cashier]);

            // Update cashier queue
            sem_wait(&cashier_queue_mutex[chosen_cashier]);
            printf("Customer [%d] checking at Cashier [%d] has checked out.\n", customer_id, chosen_cashier);
            cashier_queue[chosen_cashier]--;
            sem_post(&cashier_queue_mutex[chosen_cashier]);

            // Return the cart and go home
            return_cart(customer_id);
        } else {
            // No cart available, go home empty handed
            sem_post(&cartSemaphore);
            printf("Customer [%d] goes back home empty handed.\n", customer_id);
        }

        pthread_exit(NULL);
    }


    void shopping(int customer_id) {
        int shopping_time = (rand() % 8) + 3; // Random checkout time between 3 to 10 seconds
        printf("Customer [%d] is shopping for %d seconds.\n", customer_id, shopping_time);
        sleep(shopping_time);
    }

    void checkout(int customer_id, int chosen_cashier) {
        int checkout_time = (rand() % 4) + 9; // Random checkout time between 9 to 12 second
        printf("Customer [%d] is checking out at Cashier [%d] for %d seconds.\n", customer_id, chosen_cashier,
               checkout_time);
        sleep(checkout_time);
    }
    void checkoutScanner(int customer_id, int chosen_terminal) {
        int checkout_time = (rand() % 4) + 9; // Random checkout time between 9 to 12 seconds


        int randomCheck = rand() % 4;
        if(randomCheck == 0){
            printf("Customer [%d] is being checked at Terminal [%d]. Total checkout time is now %d seconds.\n", customer_id, chosen_terminal, checkout_time + 4);
            sleep(checkout_time + 4);
        } else{
            printf("Customer [%d] is checking out at Terminal [%d] for %d seconds.\n", customer_id, chosen_terminal,
                   checkout_time);
            sleep(checkout_time);
        }


    }


    void return_cart(int customer_id) {
        int return_time = (rand() % 2) + 1; // Random return time between 1 to 2 seconds
        sleep(return_time);
        sem_wait(&cartSemaphore);
        carts_available++;
        printf("Customer [%d] returned the cart. Carts available: %d\n", customer_id, carts_available);
        sem_post(&cartSemaphore);
    }


    int main() {
        pthread_t customers[NUM_CUSTOMERS];
        int customer_ids[NUM_CUSTOMERS];


        sem_init(&cartSemaphore, 0, 1);
        for (int i = 0; i < NUM_CASHIERS; i++) {
            sem_init(&cashiers_Semaphores[i], 0, 1);
            sem_init(&cashier_queue_mutex[i], 0, 1);
            sem_init(&cashier_ready[i], 0, 1);
        }

        sem_init(&terminalSemaphore, 0, 1);
        sem_init(&terminalSemaphorequeue, 0, 1);
        for (int i = 0; i < NUM_CASHIERS; i++) {
            sem_init(&terminal_Semaphores[i], 0, 1);
            sem_init(&terminal_queue_mutex[i], 0, 1);
            sem_init(&terminal_ready[i], 0, 1);
        }

        srand(time(NULL));

        for (int i = 0; i < NUM_CUSTOMERS; i++) {
            int useScanner = rand() % 2;
            customer_ids[i] = i + 1;
            if (useScanner == 0) {
                pthread_create(&customers[i], NULL, customer, &customer_ids[i]);
                usleep((rand() % 3 + 2) * 1000000); // Random interval between 2 to 4 seconds for customer arrival
            } else {
                if (scannersAvailable > 0) {
                    pthread_create(&customers[i], NULL, customerScanner, &customer_ids[i]);
                    usleep((rand() % 3 + 2) * 1000000); // Random interval between 2 to 4 seconds for customer arrival
                } else {
                    printf("No handheld scanners available. Customer [%d] will go for a cart.\n", customer_ids[i]);
                    pthread_create(&customers[i], NULL, customer, &customer_ids[i]);
                    usleep((rand() % 3 + 2) * 1000000); // Random interval between 2 to 4 seconds for customer arrival
                }

            }


        }

        for (int i = 0; i < NUM_CUSTOMERS; i++) {
            pthread_join(customers[i], NULL);
        }

        sem_destroy(&cartSemaphore);
        for (int i = 0; i < NUM_CASHIERS; i++) {
            sem_destroy(&cashiers_Semaphores[i]);
            sem_destroy(&cashier_queue_mutex[i]);
            sem_destroy(&cashier_ready[i]);
        }

        sem_destroy(&terminalSemaphore);
        sem_destroy(&terminalSemaphorequeue);
        for (int i = 0; i < NUM_CASHIERS; i++) {
            sem_destroy(&terminal_Semaphores[i]);
            sem_destroy(&terminal_queue_mutex[i]);
            sem_destroy(&terminal_ready[i]);
        }

        return 0;
    }
