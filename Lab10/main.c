#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


#define MAX_MEDKID_SIZE 6

typedef struct {
    int capacity;
    int current_size;
}medkid;
medkid m = {MAX_MEDKID_SIZE,MAX_MEDKID_SIZE};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_doctor = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_patient = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_pharmacist = PTHREAD_COND_INITIALIZER;

//len
int waiting_patients = 0;
int consults_waiting[3];

//flags
int patient_finished = 0;
int pharmacist_waiting = 0;
int total_patients = 0;
int finished = 0;

//Time displayer
void log_time() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
}

void* patient_thread(void *arg) {
    int id = *(int*)arg;
    free(arg);

    while(1) {
        int wait_time = rand() % 4 + 2;
        log_time();
        printf(" - Pacjent(%d): Ide do szpitala, bede za %d sekund.\n", id, wait_time);
        sleep(wait_time);

        pthread_mutex_lock(&mutex);

        if (finished) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        if (waiting_patients >= 3) {
            int back_time = rand() % 3 + 1;
            log_time();
            printf(" - Pacjent(%d): za dużo pacjentów, wracam później za %d s\n", id, back_time);
            pthread_mutex_unlock(&mutex);
            sleep(back_time);
        } else {
            waiting_patients++;
            consults_waiting[waiting_patients - 1] = id;
            log_time();
            printf(" - Pacjent(%d): czeka %d pacjentów na lekarza.\n", id, waiting_patients);

            if (waiting_patients == 3) {
                log_time();
                printf(" - Pacjent(%d): budzę lekarza.\n", id);
                pthread_cond_signal(&cond_doctor);
            }

            int my_turn = 1;
            while (my_turn && !finished) {
                my_turn = 0;
                for (int i = 0; i < 3; i++) {
                    if (consults_waiting[i] == id) {
                        my_turn = 1;
                        break;
                    }
                }

                if (my_turn) {
                    pthread_cond_wait(&cond_patient, &mutex);
                }
            }
            
            if(!finished) {
                log_time();
                printf(" - Pacjent(%d): kończę wizytę.\n", id);
                patient_finished++;
            }
        
            pthread_mutex_unlock(&mutex);
            break;
        }
    }

    return NULL;
}

void* pharmacist_thread(void* arg) {
    int id = *(int*)arg;
    free(arg);

    while(1) {
        int wait_time = rand() % 11 + 5;
        log_time();
        printf(" - Farmaceuta(%d): ide do szpitala, bede za %d s\n", id,wait_time);
        sleep(wait_time);

        pthread_mutex_lock(&mutex);

        if (patient_finished >= total_patients || finished) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        while (m.current_size >= 3 && !pharmacist_waiting) {
            log_time();
            printf(" - Farmaceuta(%d): czekam na opróżnienie apteczki.\n", id);
            pthread_cond_wait(&cond_pharmacist, &mutex);

            if (patient_finished >= total_patients) {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }

        if (m.current_size < 3 && !finished) {
            pharmacist_waiting = 1;
            log_time();
            printf(" - Farmaceuta(%d): budzę lekarza.\n", id);
            pthread_cond_signal(&cond_doctor);
        

            while (pharmacist_waiting == 1 && !finished) {
                pthread_cond_wait(&cond_pharmacist, &mutex);
            }

            if (!finished) {
                log_time();
                printf(" - Farmaceuta(%d): zakończyłem dostawę.\n", id);
            }
        }

        pthread_mutex_unlock(&mutex);
        break;
    }

    return NULL;
}


void* doctor_thread(void* arg) {
    (void)arg;

    while(1) {
        pthread_mutex_lock(&mutex);

        if (patient_finished >= total_patients) {
            log_time();
            printf(" - Lekarz: wszyscy pacjenci obsłużeni, kończę pracę.\n");
            finished = 1;
            pthread_cond_broadcast(&cond_pharmacist);
            pthread_cond_broadcast(&cond_patient);
            pthread_mutex_unlock(&mutex);
            break;
        }

        while (!((waiting_patients >= 3 && m.current_size >= 3) 
                || (m.current_size < 3 && pharmacist_waiting == 1))) {
            
            if (patient_finished >= total_patients) {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            pthread_cond_wait(&cond_doctor, &mutex);
        }

        log_time();
        printf(" - Lekarz: budzę się.\n");

        if (waiting_patients >= 3 && m.current_size >= 3) {
            log_time();
            printf(" - Lekarz: konsultuję pacjętów %d, %d, %d.\n",consults_waiting[0], consults_waiting[1], consults_waiting[2]);

            m.current_size -= 3;
            int consult_time = rand() % 3 + 2;
            pthread_mutex_unlock(&mutex);
            sleep(consult_time);
            pthread_mutex_lock(&mutex);
            waiting_patients = 0;
            patient_finished += 3;

            consults_waiting[0] = consults_waiting[1] = consults_waiting[2] = -1;

            pthread_cond_broadcast(&cond_patient);
            if (m.current_size < 3 && pharmacist_waiting) {
                log_time();
                printf(" - Lekarz: przyjmuję dostawę leków.\n");
                pthread_cond_signal(&cond_pharmacist);
            }

            pthread_mutex_unlock(&mutex);
        } else if (m.current_size < 3 && pharmacist_waiting == 1) {
            m.current_size = m.capacity;
            log_time();
            printf(" - Lekarz: przyjmuję dostawę leków.\n");
            int delivery_time = rand() % 3 + 1;
            pthread_mutex_unlock(&mutex);
            sleep(delivery_time);
            pthread_mutex_lock(&mutex);
            pharmacist_waiting = 0;
            pthread_cond_broadcast(&cond_pharmacist);
        }

        log_time();
        printf(" - Lekarz: zasypiam.\n");
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Zła liczba argumentów. Użyj: %s liczba_pacjentów liczba_farmaceutów\n", argv[0]);
        return 1;
    }

    int num_p = atoi(argv[1]);
    int num_ph = atoi(argv[2]);
    total_patients = num_p;

    srand(time(NULL));

    pthread_t doctor;
    pthread_t patients[num_p];
    pthread_t pharmacists[num_ph];

    pthread_create(&doctor, NULL, doctor_thread, NULL);

    for (int i = 0; i < num_p; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&patients[i], NULL, patient_thread, id);
    }

    for (int i = 0; i < num_ph; i++) {
        int* id = malloc(sizeof(int));
        *id = i+1;
        pthread_create(&pharmacists[i], NULL, pharmacist_thread, id);
    }

    for (int i = 0; i < num_p; i++) {
        pthread_join(patients[i], NULL);
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond_doctor);
    pthread_mutex_unlock(&mutex);

    pthread_join(doctor, NULL);

    // Wake up to finish
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond_pharmacist);
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < num_ph; i++) {
        pthread_join(pharmacists[i], NULL);
    }

    log_time();
    printf("Program zakończył działanie.\n");

    return 0;
}