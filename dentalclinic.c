#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> // for sleep

int buffer[35];        // Fixed circular buffer to hold patient IDs (35 chairs)
int in = 0, out = 0;   // Buffer indices for dentist (producer) and patients (consumers)

sem_t empty;           // Semaphore for tracking empty slots in the buffer
sem_t full;            // Semaphore for tracking full slots in the buffer
pthread_mutex_t mutex; // Mutex for critical section (access to buffer)
int totalNumberOfChairs = 35;  // Fixed number of chairs
int totalServedPatients = 0;    // Total number of patients served
int totalNumberOfPatients = 0;   // Total number of patients allowed to be treated


void* patient(void* arg)
{
    int patientId = *(int*)arg;

    sem_wait(&empty);             // Wait for an empty slot (available chair)
    pthread_mutex_lock(&mutex);   // Enter critical section

    // Check after waiting if we can treat the patient
    if (totalServedPatients >= 40)
    {

        printf("\n");
        printf("Clinic is full for today. Next available times are Sunday, Tuesday, and Thursday from 6 PM to 10 PM .\n");

        return NULL; // Patient leaves without occupying a chair


    }

    buffer[in] = patientId;       // Place patient in the buffer (occupy chair)
    printf("Patient %d sits in chair %d\n", patientId, in);
    in = (in + 1) % totalNumberOfChairs;  // Move to the next slot in the circular buffer

    pthread_mutex_unlock(&mutex); // Exit critical section
    sem_post(&full);              // Signal a full slot (patient ready for treatment)

    return NULL;
}
void* dentist(void* arg)
{
    int patientId;
    while (totalServedPatients < 40 && totalServedPatients < totalNumberOfPatients)    // Limit to 40 patients
    {
        sem_wait(&full);              // Wait for a full slot (patient ready)
        pthread_mutex_lock(&mutex);   // Enter critical section

        patientId = buffer[out];      // Treat the patient (consume from buffer)
        printf("Dentist is treating patient %d from chair %d\n", patientId, out);
        out = (out + 1) % totalNumberOfChairs; // Move to the next slot in the circular buffer

        pthread_mutex_unlock(&mutex); // Exit critical section
        sem_post(&empty);             // Signal an empty slot (chair available)

        sleep(1); // Simulate treatment time
        printf("Dentist has completed treatment for patient %d\n", patientId);

        totalServedPatients++;        // Increment the count of served patients
    }

    printf("Clinic closed for today after serving %d patients.\n", totalServedPatients);
    return NULL;
}


int main()
{
    printf("   _____            _             _    _           _     _   _____             _        _    _____ _ _       _      \n");
    printf("  / ____|          (_)           | |  | |         | |   (_) |  __ \\           | |      | |  / ____| (_)     (_)     \n");
    printf(" | (___  _   _ _ __ _  ___ _ __  | |__| | __ _ ___| |__  _  | |  | | ___ _ __ | |_ __ _| | | |    | |_ _ __  _  ___ \n");
    printf("  \\___ \\| | | | '__| |/ _ \\ '__| |  __  |/ _` / __| '_ \\| | | |  | |/ _ \\ '_ \\| __/ _` | | | |    | | | '_ \\| |/ __|\n");
    printf("  ____) | |_| | |  | |  __/ |    | |  | | (_| \\__ \\ | | | | | |__| |  __/ | | | || (_| | | | |____| | | | | | | (__ \n");
    printf(" |_____/ \\__,_|_|  | |\\___|_|    |_|  |_|\\__,_|___/_| |_|_| |_____/ \\___|_| |_|\\__\\__,_|_|  \\_____|_|_|_| |_|_|\\___|\n");
    printf("                  _/ |                                                                                             \n");
    printf("                 |__/                                                                                              \n");


    // Get input for the number of patients
    printf("Enter the total number of patients (maximum 40): ");
    scanf("%d", &totalNumberOfPatients);


    printf("\n");

    // Initialize semaphores and mutex
    sem_init(&empty, 0, totalNumberOfChairs); // Initially, all buffer slots are empty
    sem_init(&full, 0, 0);                    // Initially, no buffer slots are full
    pthread_mutex_init(&mutex, NULL);          // Initialize the mutex

    // Create dentist (producer) thread
    pthread_t dentistThread;
    pthread_create(&dentistThread, NULL, dentist, NULL);

    // Create patient (consumer) threads for up to the specified number of patients
    pthread_t* patientsThread = malloc(sizeof(pthread_t) * totalNumberOfPatients);
    int* patientNumber = malloc(sizeof(int) * totalNumberOfPatients);

    // Create patient threads
    for (int i = 0; i < totalNumberOfPatients; i++)
    {
        patientNumber[i] = i + 1;
        pthread_create(&patientsThread[i], NULL, patient, (void*)&patientNumber[i]);
        sleep(1); // Fixed sleep to ensure consistent arrival of patients
    }

    // Wait for patient threads to finish for the specified number of patients
    for (int i = 0; i < totalNumberOfPatients; i++)
    {
        pthread_join(patientsThread[i], NULL);
    }

    // Join dentist thread after all patient threads
    pthread_join(dentistThread, NULL);

    // Free allocated memory
    free(patientsThread);
    free(patientNumber);

    // Destroy semaphores and mutex
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}


