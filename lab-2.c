#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

int *matUm;
int *matDois; //matriz de entrada
int *saidaSeq;
int *saidaConc;
int nthreads; //numero de threads

typedef struct{
   int id; //identificador do elemento que a thread ira processar
   int dim; //dimensao das estruturas de entrada
} tArgs;

//funcao que as threads executarao
void * tarefa(void *arg) {
   tArgs *args = (tArgs*) arg;
   
   for(int i=args->id; i<args->dim; i+=nthreads)
        for(int j=0; j<args->dim; j++)
            for(int k = 0; k <args->dim; k++)
                saidaConc[i*(args->dim)+j] += matUm[i*(args->dim) + k] * matDois[k*(args->dim)+j];

   pthread_exit(NULL);
}

int main (int argc, char* argv[]){

    int dim; //dimensao da matriz de entrada
    pthread_t *tid; //identificadores das threads no sistema
    tArgs *args; //identificadores locais das threads e dimensao
    double inicio, fim, deltaSeq, deltaConc, desempenho;
   
    //leitura e avaliacao dos parametros de entrada
    if(argc<3) {
        printf("Digite: %s <dimensao da matriz> <numero de threads>\n", argv[0]);
        return 1;
    }

    dim = atoi(argv[1]);
    nthreads = atoi(argv[2]); //passando os argumentos para as variáveis que eu quero

    //alocacao de memoria para as estruturas de dados
    matUm = (int *) malloc(sizeof(int) * dim * dim);
    if (matUm == NULL) {printf("ERRO--malloc\n"); return 2;} //isso é para caso ele não consiga alocar espaço na memória para essa matriz.
    matDois = (int *) malloc(sizeof(int) * dim * dim);
    if (matDois == NULL) {printf("ERRO--malloc\n"); return 2;}
    saidaSeq = (int *) malloc(sizeof(int) * dim * dim);
    if (saidaSeq == NULL) {printf("ERRO--malloc\n"); return 2;}
    saidaConc = (int *) malloc(sizeof(int) * dim * dim);
    if (saidaConc == NULL) {printf("ERRO--malloc\n"); return 2;}

    //inicializacao das estruturas de dados de entrada e saida
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++){
            matUm[i*dim+j] = rand()%100;
            matDois[i*dim+j] = rand()%100;
            saidaSeq[i*dim+j] = 0;
            saidaConc[i*dim+j] = 0;
        }
    }

    GET_TIME(inicio);
    //multiplicação sequencial de matrizes
    for(int i = 0;i < dim; i++){
        for(int j = 0; j < dim; j++){
            for(int k = 0; k < dim; k++){
                saidaSeq[i*dim+j] += matUm[i*dim+k] * matDois[k*dim+j];
            }
        }
    }
    GET_TIME(fim)   
    deltaSeq = fim - inicio;

    GET_TIME(inicio);
    tid = (pthread_t*) malloc(sizeof(pthread_t)*nthreads);
    if(tid==NULL) {puts("ERRO--malloc"); return 2;}
    args = (tArgs*) malloc(sizeof(tArgs)*nthreads);
    if(args==NULL) {puts("ERRO--malloc"); return 2;}

    //criacao das threads
    for(int i=0; i<nthreads; i++) {
        (args+i)->id = i;
        (args+i)->dim = dim;
        if(pthread_create(tid+i, NULL, tarefa, (void*) (args+i))){
            puts("ERRO--pthread_create"); return 3;
        }
    }

    //espera pelo termino da threads
    for(int i=0; i<nthreads; i++) {
        pthread_join(*(tid+i), NULL);
    }

    GET_TIME(fim)   
    deltaConc = fim - inicio;

    //conferência dos resultados
    for(int i = 0;i < dim; i++){
        for(int j = 0; j < dim; j++){
            if(saidaSeq[i*dim+j] != saidaConc[i*dim+j]){
                printf("Algum dos resultados está diferente entre a multiplicação sequencial e a concorrente.\n");
                return 1;
            }
        }
    }

    printf("Tudo certo\n");

    desempenho = deltaSeq/deltaConc;
    printf("Tempo da multiplicação sequencial:%lf\n", deltaSeq);
    printf("Tempo da multiplicação concorrente:%lf\n", deltaConc);
    printf("Desempenho da solução:%lf\n", desempenho);

    //liberacao da memoria
    free(matUm);
    free(matDois);
    free(saidaSeq);
    free(saidaConc);
    free(args);
    free(tid);

    return 0;
}