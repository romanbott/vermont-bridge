#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Mutexes modeling the two parts of the bridge
pthread_mutex_t south_entrance;
pthread_mutex_t north_entrance;

// Binary semaphore acting as the "direction mutex"
sem_t direction_sem;

// Mutexes and counters to protect the driver counts
pthread_mutex_t north_mutex;
pthread_mutex_t south_mutex;
int north_count = 0;
int south_count = 0;

int use_semaphore = 0;

void *northbound_driver(void *arg) {
  int id = *((int *)arg);

  // --- Entry Protocol ---
  pthread_mutex_lock(&north_mutex);
  north_count++;
  if (north_count == 1) {
    if (use_semaphore) {
      if (north_count == 1) {
        sem_wait(&direction_sem);
        printf("[direction] bridge direction set to northbound.\n");
      }
    }
  }
  pthread_mutex_unlock(&north_mutex);

  // --- Bridge Crossing Logic ---
  printf("Northbound %d: Arrived and waiting at South Entrance...\n", id);

  pthread_mutex_lock(&south_entrance);
  printf("Northbound %d: Locked South Entrance.\n", id);

  sleep(2);
  pthread_mutex_lock(&north_entrance);
  printf("Northbound %d: Locked North Entrance.\n", id);

  pthread_mutex_unlock(&south_entrance);
  printf("Northbound %d: Unlocked South Entrance, crossing...\n", id);

  pthread_mutex_unlock(&north_entrance);
  printf("Northbound %d: Unlocked North Entrance, EXITED bridge.\n", id);

  // --- Exit Protocol ---
  pthread_mutex_lock(&north_mutex);
  north_count--;

  if (use_semaphore) {
    if (north_count == 0) {
      printf(
          "[DIRECTION] Last Northbound driver exited. Direction UNLOCKED.\n");
      sem_post(&direction_sem);
    }
  }

  pthread_mutex_unlock(&north_mutex);

  free(arg);
  return NULL;
}

void *southbound_driver(void *arg) {
  int id = *((int *)arg);

  // --- Entry Protocol ---
  pthread_mutex_lock(&south_mutex);
  south_count++;
  if (use_semaphore) {
    if (south_count == 1) {
      sem_wait(&direction_sem);
      printf("[direction] bridge direction set to southbound.\n");
    }
  }
  pthread_mutex_unlock(&south_mutex);

  // --- Bridge Crossing Logic ---
  printf("Southbound %d: Arrived and waiting at North Entrance...\n", id);

  pthread_mutex_lock(&north_entrance);
  printf("Southbound %d: Locked North Entrance.\n", id);

  sleep(2);
  pthread_mutex_lock(&south_entrance);
  printf("Southbound %d: Locked South Entrance.\n", id);

  pthread_mutex_unlock(&north_entrance);
  printf("Southbound %d: Unlocked North Entrance, crossing...\n", id);

  pthread_mutex_unlock(&south_entrance);
  printf("Southbound %d: Unlocked South Entrance, EXITED bridge.\n", id);

  // --- Exit Protocol ---
  pthread_mutex_lock(&south_mutex);
  south_count--;
  if (use_semaphore) {
    if (south_count == 0) {
      printf(
          "[DIRECTION] Last Southbound driver exited. Direction UNLOCKED.\n");
      sem_post(&direction_sem);
    }
  }
  pthread_mutex_unlock(&south_mutex);

  free(arg);
  return NULL;
}

// Generator thread function
void *driver_generator(void *arg) {
  int total_drivers = *((int *)arg);
  pthread_t *threads = malloc(total_drivers * sizeof(pthread_t));

  int north_id_counter = 1;
  int south_id_counter = 1;

  for (int i = 0; i < total_drivers; i++) {
    sleep(1); // Wait 1 second before spawning the next driver

    int direction = rand() % 2; // 0 for North, 1 for South
    int *id = malloc(sizeof(int));

    if (direction == 0) {
      *id = north_id_counter++;
      pthread_create(&threads[i], NULL, northbound_driver, id);
    } else {
      *id = south_id_counter++;
      pthread_create(&threads[i], NULL, southbound_driver, id);
    }
  }

  // Wait for all generated drivers to finish their routes
  for (int i = 0; i < total_drivers; i++) {
    pthread_join(threads[i], NULL);
  }

  free(threads);
  return NULL;
}

int main(int argc, char *argv[]) {
  int total_drivers = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--total_drivers") == 0 && i + 1 < argc) {
      total_drivers = atoi(argv[i + 1]);
    } else if (strcmp(argv[i], "--semaphore") == 0) {
      use_semaphore = 1; // Enable deadlock prevention
    }
  }

  if (total_drivers <= 0) {
    printf("Usage: %s --total_drivers <number_greater_than_0> [--semaphore]\n", argv[0]);
    return 1;
  }

  srand(time(NULL));

  // Initialize synchronization primitives
  pthread_mutex_init(&south_entrance, NULL);
  pthread_mutex_init(&north_entrance, NULL);
  pthread_mutex_init(&north_mutex, NULL);
  pthread_mutex_init(&south_mutex, NULL);
  sem_init(&direction_sem, 0, 1);

  printf("Starting simulation with %d drivers...\n", total_drivers);

  // Create the generator thread
  pthread_t generator_thread;
  pthread_create(&generator_thread, NULL, driver_generator, &total_drivers);

  // Main thread waits for the generator thread to finish
  // (which in turn waits for all driver threads to finish)
  pthread_join(generator_thread, NULL);

  printf("All drivers have crossed. Simulation complete.\n");

  // Cleanup
  pthread_mutex_destroy(&south_entrance);
  pthread_mutex_destroy(&north_entrance);
  pthread_mutex_destroy(&north_mutex);
  pthread_mutex_destroy(&south_mutex);
  sem_destroy(&direction_sem);

  return 0;
}
