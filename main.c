#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N_CLIENTS 100
#define N_WAITERS 5
#define N_CHAIRS 20

#define TIME_CLEAN_TABLE 5
#define TIME_TAKE_ORDER 1
#define TIME_SERVE_CUSTOMER 1
#define TIME_CUSTOMER_EAT 10
#define TIME_COOK_A 2
#define TIME_COOK_B 4
#define TIME_COOK_C 8

pthread_mutex_t turno;

sem_t sem_chairs;
sem_t sem_clients;
sem_t sem_waiters;
sem_t sem_orders_A;
sem_t sem_dishes_A;
sem_t sem_orders_B;
sem_t sem_dishes_B;
sem_t sem_orders_C;
sem_t sem_dishes_C;

void* cooker_A(void* v) {
  while(1) {
    // Espera ter um pedido para um prato do tipo A
    sem_wait(&sem_orders_A);
    sem_post(&sem_dishes_A);
    printf("O cozinheiro A cozinhou um prato\n");
    sleep(TIME_COOK_A);
  }

  pthread_exit(0);
}

void* cooker_B(void* v) {
  while(1) {
    // Espera ter um pedido para um prato do tipo B
    sem_wait(&sem_orders_B);
    sem_post(&sem_dishes_B);
    printf("O cozinheiro B cozinhou um prato\n");
    sleep(TIME_COOK_B);
  }

  pthread_exit(0);
}

void* cooker_C(void* v) {
  while(1) {
    // Espera ter um pedido para um prato do tipo C
    sem_wait(&sem_orders_C);
    sem_post(&sem_dishes_C);
    printf("O cozinheiro C cozinhou um prato\n");
    sleep(TIME_COOK_C);
  }

  pthread_exit(0);
}

void serve_customer(int id) {
      // Tipo do pedido do cliente
    char client_order = 'A' + rand() % 3;

    printf("O cliente %d pediu o prato %c\n", id, client_order);
    sleep(TIME_TAKE_ORDER);

    // O garçom coloca o pedido do cliente na lista de pedidos
    if (client_order == 'A') sem_post(&sem_orders_A);
    if (client_order == 'B') sem_post(&sem_orders_B);
    if (client_order == 'C') sem_post(&sem_orders_C);

    // O garçom vai atender outros clientes
    sem_post(&sem_waiters);

    // O garçom serve o cliente quando o prato estiver pronto
    if (client_order == 'A') sem_wait(&sem_dishes_A);
    if (client_order == 'B') sem_wait(&sem_dishes_B);
    if (client_order == 'C') sem_wait(&sem_dishes_C);

    sem_wait(&sem_waiters);
    
    printf("Um garçom serviu a refeição %c ao cliente %d\n", client_order, id);
    sleep(TIME_SERVE_CUSTOMER);
    // O garçom vai atender outros clientes
    sem_post(&sem_waiters);
    sleep(TIME_CUSTOMER_EAT);
    sem_wait(&sem_waiters);
    printf("Um garçom está limpando a mesa do cliente %d\n", id);
    sleep(TIME_CLEAN_TABLE);
}

void* client(void* v) {
  int id = *(int*) v;
  while(1) {
    // Lock com o turno para evitar que os mesmos clientes sejam atendidos toda a vez
    pthread_mutex_lock(&turno); 
    // O cliente espera uma cadeira ficar vazia
    sem_wait(&sem_chairs);
    sem_post(&sem_clients);
    printf("O cliente %d entrou no restaurante\n", id);
    pthread_mutex_unlock(&turno); 
    // O cliente espera ser atendido por um garçom
    sem_wait(&sem_waiters);

    serve_customer(id);

    // O garçom vai atender outros clientes
    sem_post(&sem_waiters);
    sem_wait(&sem_clients);
    printf("O cliente %d foi embora\n", id);
    // O cliente desocupa uma cadeira
    sem_post(&sem_chairs);
  }

  pthread_exit(0);
}

int main() {
  pthread_t thr_clients[N_CLIENTS], thr_cooker_A, thr_cooker_B, thr_cooker_C;
  int i, id_client[N_CLIENTS];

  pthread_mutex_init(&turno, NULL);

  sem_init(&sem_chairs, 0, N_CHAIRS);
  sem_init(&sem_waiters, 0, N_WAITERS);
  sem_init(&sem_clients, 0, 0);

  sem_init(&sem_orders_A, 0, 0);
  sem_init(&sem_dishes_A, 0, 0);
  sem_init(&sem_orders_B, 0, 0);
  sem_init(&sem_dishes_B, 0, 0);
  sem_init(&sem_orders_C, 0, 0);
  sem_init(&sem_dishes_C, 0, 0);

  pthread_create(&thr_cooker_A, NULL, cooker_A, NULL);
  pthread_create(&thr_cooker_B, NULL, cooker_B, NULL);
  pthread_create(&thr_cooker_C, NULL, cooker_C, NULL);
  
  for (i = 0; i < N_CLIENTS; i++) {
    id_client[i] = i;
    pthread_create(&thr_clients[i], NULL, client, (void*) &id_client[i]);
  }
  
  for (i = 0; i < N_CLIENTS; i++) {
    pthread_join(thr_clients[i], NULL);
  }
  
  return 0;
}