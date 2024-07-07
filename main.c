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
sem_t cashiers_Semaphores[NUM_CASHIERS];
sem_t cashier_queue_mutex[NUM_CASHIERS];
sem_t cashier_ready[NUM_CASHIERS];

int NUM_CUSTOMERS = 20;
int carts_available = NUM_CARTS;
int scannersAvailable = 10;
int cashier_queue[NUM_CASHIERS] = {0};

void *customer(void *arg);
void *customerScanner(void *arg);
void shopping(int customer_id);
void checkout(int customer_id, int chosen_cashier);
void return_cart(int customer_id);



void *customerScanner(void *arg) {
    int customer_id = *((int *)arg);

    // Customer arrives and tries to take a cart
    sem_wait(&terminalSemaphore);
    if (scannersAvailable > 0) {
        scannersAvailable--;
        printf("Customer %d takes a handheld scanner. Handheld scanners available: %d\n", customer_id, scannersAvailable);
        sem_post(&terminalSemaphore);

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
        printf("Customer [%d] is checking out at Cashier [%d] and the queue is %d.\n", customer_id, chosen_cashier, cashier_queue[chosen_cashier]);
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

void *customer(void *arg) {
    int customer_id = *((int *)arg);

    // Customer arrives and tries to take a cart
    sem_wait(&cartSemaphore);
    if (carts_available > 0) {
        carts_available--;
        printf("Customer %d takes a cart. Carts available: %d\n", customer_id, carts_available);
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
        printf("Customer [%d] is checking out at Cashier [%d] and the queue is %d.\n", customer_id, chosen_cashier, cashier_queue[chosen_cashier]);
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
    int shopping_time = (rand() % 10) + 1; // Random shopping time between 6 to 10 seconds
    printf("Customer [%d] is shopping for %d seconds.\n", customer_id, shopping_time);
    sleep(shopping_time);
}

void checkout(int customer_id, int chosen_cashier) {
    int checkout_time = (rand() % 8) + 3; // Random checkout time between 3 to 10 seconds
    printf("Customer [%d] is checking out at Cashier [%d] for %d seconds.\n", customer_id, chosen_cashier, checkout_time);
    sleep(checkout_time);
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

    sem_init(&terminalSemaphore, 0, 1);
    sem_init(&cartSemaphore, 0, 1);
    for (int i = 0; i < NUM_CASHIERS; i++) {
        sem_init(&cashiers_Semaphores[i], 0, 1);
        sem_init(&cashier_queue_mutex[i], 0, 1);
        sem_init(&cashier_ready[i], 0, 1);
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
                pthread_create(&customers[i], NULL, customer, &customer_ids[i]);
                usleep((rand() % 3 + 2) * 1000000); // Random interval between 2 to 4 seconds for customer arrival

            } else {
                printf("No handheld scanners available. Customer [%d] will go for a cart.\n", customer_ids[i]);
                pthread_create(&customers[i], NULL, customerScanner, &customer_ids[i]);
                usleep((rand() % 3 + 2) * 1000000); // Random interval between 2 to 4 seconds for customer arrival
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

        return 0;
    }
}