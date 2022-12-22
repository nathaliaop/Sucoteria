#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N_CLIENTS 20
#define N_WAITERS 5
#define N_CHAIRS 3

#define TIME_CLEAN_TABLE 1
#define TIME_TAKE_ORDER 1
#define TIME_CUSTOMER_EAT 3

pthread_mutex_t turno;

sem_t sem_chairs;
sem_t sem_clients;
sem_t sem_cooker_A;
sem_t sem_cooker_B;
sem_t sem_cooker_C;
sem_t sem_waiters;

void* client(void* v) {
  int id = *(int*) v;
  while(1) {
    pthread_mutex_lock(&turno); 
    sem_wait(&sem_chairs);
    sem_post(&sem_clients);
    printf("O cliente %d entrou no restaurante\n", id);
    pthread_mutex_unlock(&turno); 
    sem_wait(&sem_waiters);
    printf("O cliente %d foi atendido\n", id);
    sleep(TIME_TAKE_ORDER);
    sem_post(&sem_waiters);
    printf("O cliente %d está comendo a refeição\n", id);
    sleep(TIME_CUSTOMER_EAT);
    sem_wait(&sem_waiters);
    printf("O garçom está limpando a mesa do cliente %d\n", id);
    sleep(TIME_CLEAN_TABLE);
    sem_post(&sem_waiters);
    sem_wait(&sem_clients);
    printf("O cliente %d foi embora\n", id);
    sem_post(&sem_chairs);
  }

  pthread_exit(0);
}

int main() {
  pthread_t thr_clients[N_CLIENTS], thr_cooker_A, thr_cooker_B, thr_cooker_C, thr_waiters[N_WAITERS];
  int i, id_client[N_CLIENTS];
  int j, id_waiter[N_WAITERS];

  pthread_mutex_init(&turno, NULL);

  sem_init(&sem_chairs, 0, N_CHAIRS);
  sem_init(&sem_waiters, 0, N_WAITERS);
  sem_init(&sem_clients, 0, 0);
  
  for (i = 0; i < N_CLIENTS; i++) {
    id_client[i] = i;
    pthread_create(&thr_clients[i], NULL, client, (void*) &id_client[i]);
  }
  
  for (i = 0; i < N_CLIENTS; i++) {
    pthread_join(thr_clients[i], NULL);
  }
  
  return 0;
}

/*
Um restaurante tem N cadeiras. Os clientes chegam ao restaurante e senta em uma cadeira. Quando um cliente está pronto para fazer um pedido, ele acorda um dos garçons, que anota o pedido e comunica aos cozinheiros.
O restaurante serve 3 tipos diferentes de sucos e cada cozinheiro é responsável por preparar um tipo de suco diferente. Se o cozinheiro estiver dormindo, o garçom acorda o cozinheiro para preparar o suco, caso o contrário o garçom deixa o pedido com o cozinheiro. Enquanto o pedido está sendo preparado o garçom vai atender outro cliente ou volta a dormir se não houver clientes para atender.
Quando o pedido fica pronto, o cozinheiro acorda o garçom se ele estiver dormindo e cozinha o próximo pedido ou volta a dormir se não houver pedidos para preparar. O garçom serve o pedido, continua atendendo outros clientes e quando o cliente termina de comer, o garçom tem que limpar a mesa antes de outro cliente poder ocupar a mesa.
*/