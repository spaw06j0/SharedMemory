#include <iostream>
#include <queue>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <random>
#include <chrono>

using namespace std;

// Constants
const int BUFFER_SIZE = 5;
const int NUM_PRODUCERS = 2;
const int NUM_CONSUMERS = 2;
const int NUM_ITEMS = 10;

// Shared resources
queue<int> buffer;
pthread_mutex_t mutex;
sem_t empty_slots;
sem_t filled_slots;

// Function to generate random number
int generate_item() {
    return rand() % 100;
}

// Producer function
void* producer(void* arg) {
    int id = *((int*)arg);
    
    for (int i = 0; i < NUM_ITEMS; i++) {
        // Generate item before entering critical section
        int item = generate_item();
        
        // Wait if buffer is full
        sem_wait(&empty_slots);
        
        // critical section
        pthread_mutex_lock(&mutex);
        buffer.push(item);
        cout << "Producer " << id << " produced: " << item 
             << " (buffer size: " << buffer.size() << ")" << endl;
        pthread_mutex_unlock(&mutex);
        
        // Signal that new item is available
        sem_post(&filled_slots);
        
        // Random delay
        usleep(rand() % 1000000);  // Sleep up to 1 second
    }
    
    delete (int*)arg;
    return NULL;
}

// Consumer function
void* consumer(void* arg) {
    int id = *((int*)arg);
    
    for (int i = 0; i < NUM_ITEMS; i++) {
        // Wait if buffer is empty
        sem_wait(&filled_slots);
        
        // critical section
        pthread_mutex_lock(&mutex);
        int item = buffer.front();
        buffer.pop();
        cout << "Consumer " << id << " consumed: " << item 
             << " (buffer size: " << buffer.size() << ")" << endl;
        pthread_mutex_unlock(&mutex);
        
        // Signal that a buffer slot is free
        sem_post(&empty_slots);
        
        // Random delay
        usleep(rand() % 1000000);  // Sleep up to 1 second
    }
    
    delete (int*)arg;
    return NULL;
}

int main() {
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize synchronization primitives
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty_slots, 0, BUFFER_SIZE);  // Initialize empty_slots semaphore to BUFFER_SIZE
    sem_init(&filled_slots, 0, 0);           // Initialize filled_slots semaphore to 0
    
    // Create thread arrays
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    
    // Create producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        int* id = new int(i);
        if (pthread_create(&producers[i], NULL, producer, (void*)id) != 0) {
            cerr << "Error creating producer thread " << i << endl;
            return 1;
        }
    }
    
    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        int* id = new int(i);
        if (pthread_create(&consumers[i], NULL, consumer, (void*)id) != 0) {
            cerr << "Error creating consumer thread " << i << endl;
            return 1;
        }
    }
    
    // Wait for producer threads to finish
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    
    // Wait for consumer threads to finish
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    // Cleanup synchronization primitives
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty_slots);
    sem_destroy(&filled_slots);
    
    return 0;
}