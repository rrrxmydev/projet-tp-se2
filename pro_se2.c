#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>


#define NB_BUS_X 5
#define NB_BUS_Y 4
#define NB_TRAJETS 10


typedef struct {
    int id;
    char ville; 
} BusInfo;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int buses_in_tunnel = 0;
char current_direction = 0; 
int waiting_X = 0;
int waiting_Y = 0;


void sleep_random() {
    usleep((rand() % 500 + 1000) * 1000);
}


void entrer_tunnel(char direction) {
    pthread_mutex_lock(&mutex);

    if (direction == 'X') waiting_X++;
    else waiting_Y++;

    while ((current_direction != 0 && current_direction != direction) ||
           (buses_in_tunnel == 0 && ((direction == 'X' && waiting_Y > 0) || (direction == 'Y' && waiting_X > 0)))) {
        pthread_cond_wait(&cond, &mutex);
    }

    if (direction == 'X') waiting_X--;
    else waiting_Y--;

    current_direction = direction;
    buses_in_tunnel++;

    pthread_mutex_unlock(&mutex);
}


void sortir_tunnel() {
    pthread_mutex_lock(&mutex);

    buses_in_tunnel--;
    if (buses_in_tunnel == 0) {
        current_direction = 0;
        pthread_cond_broadcast(&cond); 
    }

    pthread_mutex_unlock(&mutex);
}


void* bus_thread(void* arg) {
    BusInfo* info = (BusInfo*)arg;
    int id = info->id;
    char ville_origine = info->ville;
    char direction = ville_origine;
    free(info); 

    for (int i = 1; i <= NB_TRAJETS; i++) {
        
        entrer_tunnel(direction);
        printf("Bus %d de %c : %c -> %c (Trajet %d)\n", id, ville_origine, direction, direction == 'X' ? 'Y' : 'X', i);
        fflush(stdout);
        sleep_random();
        sortir_tunnel();

        
        direction = (direction == 'X') ? 'Y' : 'X';
        entrer_tunnel(direction);
        printf("Bus %d de %c : %c -> %c (Trajet %d)\n", id, ville_origine, direction, direction == 'X' ? 'Y' : 'X', i);
        fflush(stdout);
        sleep_random();
        sortir_tunnel();

     
        direction = (direction == 'X') ? 'Y' : 'X';
    }

    return NULL;
}


int main() {
    srand(time(NULL));
    pthread_t threads[NB_BUS_X + NB_BUS_Y];
    int index = 0;

    
    for (int i = 0; i < NB_BUS_X; i++) {
        BusInfo* info = malloc(sizeof(BusInfo));
        info->id = i + 1;
        info->ville = 'X';
        pthread_create(&threads[index++], NULL, bus_thread, info);
    }

    
    for (int i = 0; i < NB_BUS_Y; i++) {
        BusInfo* info = malloc(sizeof(BusInfo));
        info->id = i + 1;
        info->ville = 'Y';
        pthread_create(&threads[index++], NULL, bus_thread, info);
    }

    
    for (int i = 0; i < index; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
