#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


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
int scannersPickup = 0;
int cashier_queue[NUM_CASHIERS] = {0};
int scanner_queue[NUM_CASHIERS] = {0};

void *customer(void *arg);

void *customerScanner(void *arg);

void shopping(int customer_id);

void checkout(int customer_id, int chosen_cashier);

void checkoutScanner(int customer_id, int chosen_terminal);

void return_cart(int customer_id);

void backupTerminal(int customer_id, int chosen_terminal);


/*
 This function simulates a customer who uses a handheld scanner for self-checkout.
 It involves the customer picking waiting the queue, and proceeding to a terminal
 for the final checkout process.
*/

void *customerScanner(void *arg) {
    int customer_id = *((int *) arg);


    sem_wait(&terminalSemaphore);

    // check if scannersAvailable < 5 to then return the scanners to the pickup location
    if (scannersAvailable < 5 && scannersPickup > 4) {
        printf("An Employee is moving the handheld scanners to the pickup Location.\n");
        sleep(4);
        scannersAvailable = scannersAvailable + scannersPickup;
        scannersPickup = 0;
        printf("An Employee has returned the handheld scanners to the pickup Location. Handheld scanners available: %d\n",
               scannersAvailable);
    }
        scannersAvailable--;
        printf("Customer [%d] takes a handheld scanner. Handheld scanners available: %d\n", customer_id,
               scannersAvailable);
        sem_post(&terminalSemaphore);

        // Customer starts shopping
        shopping(customer_id);

        int chosen_terminal = -1;

        // Choose the terminal with the no queue
        for (int i = 0; i < 1; i++) {
            sem_wait(&terminal_Semaphores[i]);

            // Check terminals queue for the smallest, if full join queue
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

        // loop to find the next empty terminal
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

        // Wait for the chosen terminal to signal the customer's turn
        sem_wait(&terminal_ready[chosen_terminal]);

        // Simulate checkout time
        checkoutScanner(customer_id, chosen_terminal);

        // Customer finishes checkout, signal to the next customer in line
        sem_post(&terminal_ready[chosen_terminal]);

        // Update terminal queue
        sem_wait(&terminal_queue_mutex[chosen_terminal]);
        printf("Customer [%d] checking at terminal [%d] has checked out.\n", customer_id, chosen_terminal);
        scanner_queue[chosen_terminal]--;
        sem_post(&terminal_queue_mutex[chosen_terminal]);

        // Return the scanner and increment the pickup
        sem_wait(&terminalSemaphore);
        scannersPickup++;
        printf("Customer [%d] returned the scanner.\n", customer_id);
        sem_post(&terminalSemaphore);

    pthread_exit(NULL); // exit the thread
}

/*
This function simulates the behavior of a customer who picks a shopping cart
and continues through the shopping process until he checks out at a cashier
or terminal. It involves selecting the shortest queue in the cashier, and
deciding on a checkout method depending on the queue.
*/


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

        int chosen_cashier = 0;
        int chosen_terminal;

        // Choose the cashier with the smallest or no queue
        for (int i = 1; i < NUM_CASHIERS; i++) {
            sem_wait(&cashiers_Semaphores[i]);

            if (cashier_queue[i] < cashier_queue[chosen_cashier]) {
                chosen_cashier = i;
            }
            sem_post(&cashiers_Semaphores[i]); // Release semaphore for each cashier
        }

        // if cashier queue for each one is more than two go to the terminal for check out
        if (cashier_queue[chosen_cashier] >= 2) {
            if (scanner_queue[0] <= 1) {
                chosen_terminal = 0;
                scanner_queue[chosen_terminal]++;
                backupTerminal(customer_id, 0);
                pthread_exit(NULL); // exit the thread
            } else if (scanner_queue[1] <= 0) {
                chosen_terminal = 1;
                scanner_queue[chosen_terminal]++;
                backupTerminal(customer_id, 1);
                pthread_exit(NULL); // exit the thread
            }

        }

        // Join the chosen cashier's queue
        sem_wait(&cashier_queue_mutex[chosen_cashier]);
        cashier_queue[chosen_cashier]++;
        if (cashier_queue[chosen_cashier] - 1 == 0) {
            printf("Customer [%d] is checking out at Cashier [%d].\n", customer_id, chosen_cashier);
        } else {
            printf("Customer [%d] is checking out at Cashier [%d] and his turn is after %d customer.\n", customer_id,
                   chosen_cashier,
                   cashier_queue[chosen_cashier] - 1);
        }

        sem_post(&cashier_queue_mutex[chosen_cashier]);


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

        // Return the cart
        return_cart(customer_id);

    } else {

        // No cart available, go home empty handed
        sem_post(&cartSemaphore);
        printf("Customer [%d] goes back home empty handed.\n", customer_id);
    }

    pthread_exit(NULL); // exit the thread
}

/*
This function provides a backup checkout option for customers with a cart if the cashier queue is too long.
The `customer_id` parameter is used to differentiate between customers, and `chosen_cashier` indicates
which terminal the customer has selected for checkout.
*/

void backupTerminal(int customer_id, int chosen_terminal) {

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

    // Return the cart and go home
    return_cart(customer_id);

}

/*
This function simulates the time taken by a customer to shop. The `customer_id` parameter is used to differentiate between customers.
*/

void shopping(int customer_id) {
    int shopping_time = (rand() % 10) + 1;; // Random shopping time between 1 to 10 seconds
    printf("Customer [%d] is shopping for %d seconds.\n", customer_id, shopping_time);
    sleep(shopping_time); // wait
}


/*
This function simulates the checkout time at a cashier for a given customer. The `customer_id` parameter is used
to differentiate between customers, and `chosen_cashier` indicates which cashier the customer has selected for checkout.
*/

void checkout(int customer_id, int chosen_cashier) {
    int checkout_time = (rand() % 8) + 3;; // Random checkout time between 3 to 10 seconds
    printf("Customer [%d] is checking out at Cashier [%d] for %d seconds.\n", customer_id, chosen_cashier,
           checkout_time);
    sleep(checkout_time);
}

/*
This function simulates the checkout time using a handheld scanner at a terminal. The `customer_id` parameter is used to
differentiate between customers, and `chosen_cashier` indicates which terminal the customer has selected for checkout.
Additionally, there's a 25% chance that the customer will be checked.
 */

void checkoutScanner(int customer_id, int chosen_terminal) {

    int checkout_time = 1; // Random checkout time 1 seconds
    int randomCheck = rand() % 4; // 25 % check of checking
    if (randomCheck == 0) {
        printf("Customer [%d] is being checked at Terminal [%d]. Total checkout time is now %d seconds.\n", customer_id,
               chosen_terminal, checkout_time + 4);
        sleep(checkout_time + 4);
    } else {
        printf("Customer [%d] is checking out at Terminal [%d] for %d seconds.\n", customer_id, chosen_terminal,
               checkout_time);
        sleep(checkout_time); // wait
    }
}

/*
This function simulates the time of returning a cart after the customer has completed their shopping and checkout.
The `customer_id` parameter is used to differentiate between customers.
*/

void return_cart(int customer_id) {
    int return_time = (rand() % 2) + 1; // Random return cart time between 1 to 2 seconds
    sleep(return_time); // wait
    sem_wait(&cartSemaphore);
    carts_available++;
    printf("Customer [%d] returned the cart. Carts available: %d\n", customer_id, carts_available);
    sem_post(&cartSemaphore);
}

/*
 This function is used to initialize the semaphores, create the threads(customers, cashiers, terminals),
 create a customer every 2 to 4 seconds, afterwards join the threads, and finally destroy the semaphores creadted.
*/

int main() {
    pthread_t customers[NUM_CUSTOMERS];
    int customer_ids[NUM_CUSTOMERS];

    // initialize cart and cashier semaphores
    sem_init(&cartSemaphore, 0, 1);
    for (int i = 0; i < NUM_CASHIERS; i++) {
        sem_init(&cashiers_Semaphores[i], 0, 1);
        sem_init(&cashier_queue_mutex[i], 0, 1);
        sem_init(&cashier_ready[i], 0, 1);
    }

    // initialize scanner and terminal semaphores
    sem_init(&terminalSemaphore, 0, 1);
    sem_init(&terminalSemaphorequeue, 0, 1);
    for (int i = 0; i < NUM_CASHIERS; i++) {
        sem_init(&terminal_Semaphores[i], 0, 1);
        sem_init(&terminal_queue_mutex[i], 0, 1);
        sem_init(&terminal_ready[i], 0, 1);
    }

    srand(time(NULL));

    // create customer thread every 2 to 4 seconds
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        int useScanner = rand() % 2; // 50% chance cart or scanner
        customer_ids[i] = i + 1;
        if (useScanner == 0) {
            pthread_create(&customers[i], NULL, customer, &customer_ids[i]);
            sleep((rand() % 3) + 2); // Random time between 2 to 4 seconds for customer arrival
        } else {
            if (scannersAvailable > 0) {
                pthread_create(&customers[i], NULL, customerScanner, &customer_ids[i]);
                sleep((rand() % 3) + 2); // Random time between 2 to 4 seconds for customer arrival
            } else {
                printf("No handheld scanners available. Customer [%d] will go for a cart.\n", customer_ids[i]);
                pthread_create(&customers[i], NULL, customer, &customer_ids[i]);
                sleep((rand() % 3) + 2); // Random time between 2 to 4 seconds for customer arrival
            }
        }
    }

    // join the threads
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    // return the rest of the scanners
    if (scannersAvailable < 11) {
        printf("An Employee is moving the rest of the handheld scanners to the pickup Location.\n");
        sleep(4);
        scannersAvailable = scannersAvailable + scannersPickup;
        scannersPickup = 0;
        printf("An Employee has returned the handheld scanners to the pickup Location. Handheld scanners available: %d\n",
               scannersAvailable);
    }


    // initialize cart and cashier semaphores
    sem_destroy(&cartSemaphore);
    for (int i = 0; i < NUM_CASHIERS; i++) {
        sem_destroy(&cashiers_Semaphores[i]);
        sem_destroy(&cashier_queue_mutex[i]);
        sem_destroy(&cashier_ready[i]);
    }

    // destroy scanner and terminal semaphores
    sem_destroy(&terminalSemaphore);
    sem_destroy(&terminalSemaphorequeue);
    for (int i = 0; i < NUM_CASHIERS; i++) {
        sem_destroy(&terminal_Semaphores[i]);
        sem_destroy(&terminal_queue_mutex[i]);
        sem_destroy(&terminal_ready[i]);
    }
    return 0;
}
