# Supermarket Simulation Problem Definition and Requirements

## Step 1: Define the Problem and Requirements

**Customer Behavior:**
- Customers arrive at the supermarket every 2 to 4 seconds.

**Resources Available:**
- 20 carts.
- 10 handheld scanners.
- 3 cashiers for normal checkout.
- 2 checkout terminals for handheld scanner users.

**Synchronization Needs:**
- Manage the availability of carts, handheld scanners, cashiers, and checkout terminals.

## Step 2: Identify Activities (Threads)

### Customer Thread:
1. Arrives at the supermarket.
2. Takes a cart (if available).
3. Takes a handheld scanner (if available).
4. Shops for groceries.
5. Chooses the appropriate checkout method (cashier or terminal).
6. Pays for the groceries.
7. Returns the cart.

### Cashier Thread:
- Registers and processes payments for customers without handheld scanners.

### Checkout Terminal Thread:
- Registers and processes payments for customers with handheld scanners.

## Step 3: Define Semaphores and Shared Data

### Semaphores:

- **Carts Semaphore:** Manages the availability of 20 carts.
- **Handheld Scanners Semaphore:** Manages the availability of 10 handheld scanners.
- **Cashiers Semaphore:** Manages the availability of 3 cashiers.
- **Checkout Terminals Semaphore:** Manages the availability of 2 checkout terminals.



````c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_CARTS 20
#define NUM_SCANNERS 10
#define NUM_CASHIERS 3
#define NUM_TERMINALS 2

// Global counters for resources
volatile int cartsAvailable = NUM_CARTS;
volatile int scannersAvailable = NUM_SCANNERS;

// Semaphore declarations
sem_t cashierSem[NUM_CASHIERS];
sem_t terminalSem[NUM_TERMINALS];

// Function prototypes
void *customer(void *arg);
void *cartManager(void *arg);
void *scannerManager(void *arg);
void *cashier(void *arg);
void *terminal(void *arg);



void *cartManager(void *arg) {
    while (1) {
        // Simulate shopping time
      usleep(rand() % 9000000 + 1000000);  // Shopping time between 1-10 seconds
    // Simulate choosing between cart and handheld scanner
    int useScanner = rand() % 2;  // 0 for cart, 1 for handheld scanner

        // Return cart or scanner
        if (useScanner == 0 && cartsAvailable < NUM_CARTS) {
            // Simulate leaving the supermarket
            usleep(rand() % 2000000 + 1000000);  // Time to leave between 1-3 seconds
            cartsAvailable++;
            printf("Customer returned the cart. Carts available: %d\n", cartsAvailable);
        } else if(useScanner == 1 && scannersAvailable < NUM_SCANNERS) {
            scannersAvailable++;
            printf("Customer returned the handheld scanner. Scanners available: %d\n", scannersAvailable);
        }
        }

}



void *customer(void *arg) {
    while (1) {
        // Simulate customer behavior
        usleep(rand() % 2000000 + 1000000);  // Wait between 1-3 seconds before entering

        // Simulate choosing between cart and handheld scanner
        int useScanner = rand() % 2;  // 0 for cart, 1 for handheld scanner

        if (useScanner == 0) {
            // Use cart
            if (cartsAvailable > 0) {
                cartsAvailable--;
                printf("Customer took a cart. Carts available: %d\n", cartsAvailable);
            } else {
                printf("Customer left: No carts available.\n");
            }
        } 
        }

        // Simulate shopping time
  /*      usleep(rand() % 9000000 + 1000000);  // Shopping time between 1-10 seconds

        // Return cart or scanner
        if (useScanner == 0) {
            // Simulate leaving the supermarket
            usleep(rand() % 2000000 + 1000000);  // Time to leave between 1-3 seconds
            cartsAvailable++;
            printf("Customer returned the cart. Carts available: %d\n", cartsAvailable);
        } else {
            scannersAvailable++;
            printf("Customer returned the handheld scanner. Scanners available: %d\n", scannersAvailable);
        }*/

    }
}

/*void *scannerManager(void *arg) {
    // Simulate scanner management (e.g., returning abandoned scanners)
    while (1) {
        usleep(3000000);  // Every 3 seconds

        if (scannersAvailable < NUM_SCANNERS) {
            scannersAvailable++;
            printf("Scanner manager returned an abandoned handheld scanner. Scanners available: %d\n", scannersAvailable);
        }
    }
}*/
/*

void *cashier(void *arg) {
    int cashierId = (intptr_t)arg;

    // Cashier behavior: serve customers
    while (1) {
        sem_wait(&cashierSem[cashierId]);  // Wait until a customer is available
        usleep(rand() % 8000000 + 3000000);  // Checkout time between 3-10 seconds
        printf("Cashier %d served a customer.\n", cashierId);
    }
}

void *terminal(void *arg) {
    int terminalId = (intptr_t)arg;

    // Terminal behavior: serve customers
    while (1) {
        sem_wait(&terminalSem[terminalId]);  // Wait until a customer is available
        usleep(1000000);  // Checkout time for scanner is 1 second
        printf("Terminal %d served a customer.\n", terminalId);
    }
}


*/


int main() {
    pthread_t customerThread, cartThread, scannerThread, cashierThread[NUM_CASHIERS], terminalThread[NUM_TERMINALS];

/*    // Initialize semaphores for cashiers and terminals
    for (int i = 0; i < NUM_CASHIERS; i++) {
        sem_init(&cashierSem[i], 0, 0);
    }
    for (int i = 0; i < NUM_TERMINALS; i++) {
        sem_init(&terminalSem[i], 0, 0);
    }*/

    // Create threads for cart manager, scanner manager, cashiers, terminals
    pthread_create(&cartThread, NULL, cartManager, NULL);
   // pthread_create(&scannerThread, NULL, scannerManager, NULL);
/*    for (int i = 0; i < NUM_CASHIERS; i++) {
        pthread_create(&cashierThread[i], NULL, cashier, (void *)(intptr_t)i);
    }
    for (int i = 0; i < NUM_TERMINALS; i++) {
        pthread_create(&terminalThread[i], NULL, terminal, (void *)(intptr_t)i);
    }*/

    // Create thread for generating customers
    pthread_create(&customerThread, NULL, customer, NULL);

    // Join threads
    pthread_join(customerThread, NULL);
    pthread_join(cartThread, NULL);
  //  pthread_join(scannerThread, NULL);
/*    for (int i = 0; i < NUM_CASHIERS; i++) {
        pthread_join(cashierThread[i], NULL);
    }
    for (int i = 0; i < NUM_TERMINALS; i++) {
        pthread_join(terminalThread[i], NULL);
    }*/

/*    // Destroy semaphores
    for (int i = 0; i < NUM_CASHIERS; i++) {
        sem_destroy(&cashierSem[i]);
    }
    for (int i = 0; i < NUM_TERMINALS; i++) {
        sem_destroy(&terminalSem[i]);
    }*/

    return 0;
}

````