# include <pthread.h>
# include <stdio.h>
# include <stdbool.h>
# include <stdlib.h>
# include <math.h>
# include <semaphore.h>
# include <unistd.h>

# define TIME_DIV 20000000

struct montagem {
        int thread_chassi_ativo;
        int thread_motor_ativo;
        int thread_roda_ativo;
        pthread_mutex_t mutex_chassi;
        pthread_mutex_t mutex_motor;
        pthread_mutex_t mutex_roda;
        sem_t semaforo_roda;
        sem_t semaforo_print;
        int n_carros;
        int condicional_carro_montado;
        int chassis_em_processamento;
        int motores_em_processamento;
        int rodas_em_processamento;
};

struct encomenda {
    int tempo;
    int n_chassis;
    int n_motores;
    int n_rodas;
    struct montagem *dados_montagem;
};

void print_situação_atual(int chassis, int motores, int rodas, int carros) {
    printf("-----------------------------------------\n"
    "-            Fila de Montagem           -\n"
    "-                                       -\n"
    "-                __%d__                 -\n"
    "-               /|_||_\\`.__            -\n"
    "-              (   _    _ _\\           -\n"
    "-               =`----------'           -\n"
    "-                                       -\n"
    "-                  %d                   -\n"
    "-                _______                -\n"
    "-               /       \\              -\n"
    "-              | [O] [O] |              -\n"
    "-              |_________|              -\n"
    "-                                       -\n"
    "-                  %d                   -\n"
    "-                 .---.                 -\n"
    "-                /     \\               -\n"
    "-               |   O   |               -\n"
    "-                \\     /               -\n"
    "-                 '---'                 -\n"
    "-                                       -\n"
    "-            Carros Produzidos          -\n"
    "-                  %d                   -\n"
    "-                ______                 -\n"
    "-               /|_||_\\`.__            -\n"
    "-              (   _    _ _\\           -\n"
    "-               =`-(_)--(_)-'           -\n"
    "-----------------------------------------\n\n\n"
    , chassis, motores, rodas, carros);
}

void* chassi(void *data) {
    // Extrai dados da memória compartilhada
    struct montagem *dados = (struct montagem*) data;

    // Apenas 1 chassi entra na fila de montagem por vez.
    pthread_mutex_lock(&dados->mutex_chassi);
    
    // Indica que há 1 motor a espera das outras partes do carro.
    dados->thread_chassi_ativo += 1;

    // Print da situação da linha de montagem e número de carros produzidos.
    sem_wait(&dados->semaforo_print);
    print_situação_atual(dados->thread_chassi_ativo, dados->thread_motor_ativo, dados->thread_roda_ativo, dados->n_carros);
    sem_post(&dados->semaforo_print);

    // Busy-wait até haver, além do motor, também 1 chassi e 4 rodas.
    while (dados->thread_motor_ativo < 1 || dados->thread_roda_ativo < 4) {
        NULL;
    }

    // Indica que o chassi está sendo usado para montar o carro
    dados->chassis_em_processamento += 1;

    // Busy-wait que recebe variável de outra thread, indicando que o carro foi montado. 
    while (dados->chassis_em_processamento > 0) {
        NULL;
    }
    
    // Como o carro foi montado, libera a fila para outro chassi.
    pthread_mutex_unlock(&dados->mutex_chassi);

    return NULL;
}

void* motor(void *data) {
    // Extrai dados da memória compartilhada
    struct montagem *dados = (struct montagem*) data;

    // Apenas 1 motor entra na fila de montagem por vez.
    pthread_mutex_lock(&dados->mutex_motor);

    // Indica que há 1 motor a espera das outras partes do carro.
    dados->thread_motor_ativo += 1;

    // Print da situação da linha de montagem e número de carros produzidos.
    sem_wait(&dados->semaforo_print);
    print_situação_atual(dados->thread_chassi_ativo, dados->thread_motor_ativo, dados->thread_roda_ativo, dados->n_carros);
    sem_post(&dados->semaforo_print);

    // Busy-wait até haver, além do motor, também 1 chassi e 4 rodas.
    while (dados->thread_chassi_ativo < 1 || dados->thread_roda_ativo < 4) {
        NULL;
    }

    // Indica que o motor está sendo usado para montar o carro.
    dados->motores_em_processamento += 1;

    // Busy-wait que recebe variável de outra thread, indicando que o carro foi montado.
    while (dados->motores_em_processamento > 0) {
        NULL;
    }

    // Como o carro foi montado, libera a fila para outro motor.
    pthread_mutex_unlock(&dados->mutex_motor);

    return NULL;
}

void* roda(void *data) {
    // Extrai dados da memória compartilhada
    struct montagem *dados = (struct montagem*) data;

    // Semafóro que foi iniciado com value 4 e permite 4 rodas na fila de montagem por vez.
    sem_wait(&dados->semaforo_roda);
    
    // Indica que há 1 roda a espera das outras partes do carro, incluindo um total de 4 rodas.
    dados->thread_roda_ativo += 1;

    // Print da situação da linha de montagem e número de carros produzidos.
    sem_wait(&dados->semaforo_print);
    print_situação_atual(dados->thread_chassi_ativo, dados->thread_motor_ativo, dados->thread_roda_ativo, dados->n_carros);
    sem_post(&dados->semaforo_print);

    // Busy-wait até haver, 4 rodas, 1 chassi e 1 motor.
    while (dados->thread_roda_ativo < 4 || dados->thread_chassi_ativo < 1 || dados->thread_motor_ativo < 1) {
        NULL;
    }

    // Realiza mutex após a fila de montagem estar completa por 4 rodas, 1 chassi e 1 motor.
    // Resulta em processar 1 roda por vez.
    pthread_mutex_lock(&dados->mutex_roda);

    // Indica que há mais uma roda sendo usada para montar o carro.
    dados->rodas_em_processamento += 1;

    // Se as 4 rodas foram processadas, entra no condicional.
    if (dados->rodas_em_processamento >= 4) {
        // Busy-wait que indica se o chassi e motor também já foram processados.
        while (dados->chassis_em_processamento < 1 || dados->motores_em_processamento < 1) {
            NULL;
        }
        // O carro foi montado.
        // A fila é liberada para receber novos componentes.
        dados->thread_chassi_ativo = 0;
        dados->thread_motor_ativo = 0;
        dados->thread_roda_ativo = 0;

        // Adiciona 1 carro à lista de carros produzidos
        dados->n_carros += 1;
        // Printa a situação atual da linha de montagem.
        print_situação_atual(dados->thread_chassi_ativo, dados->thread_motor_ativo, dados->thread_roda_ativo, dados->n_carros);

        // Como carro foi montado e, portanto, todos os itens saem do processamento.
        dados->chassis_em_processamento = 0;
        dados->motores_em_processamento = 0;
        dados->rodas_em_processamento = 0;
        for (int i = 0; i < 4; i++) {
            sem_post(&dados->semaforo_roda);
        }
    }

    pthread_mutex_unlock(&dados->mutex_roda);

    return NULL;
}

void* encomenda_chassi(void *data) {
    struct encomenda *dados = (struct encomenda*) data;

    int time;
    pthread_t thread_chassi[dados->n_chassis];
    pthread_t id_main_thread = pthread_self();
    for (int i = 0; i < dados->n_chassis; i++) {
        time = rand()/TIME_DIV;
        usleep(time);
        pthread_create(&thread_chassi[i], NULL, chassi, (void *) dados->dados_montagem);
    }

    sleep(dados->tempo);
    printf("Linha de chassis parou\n");
    for (int i = 0; i < dados->n_chassis; i++) {
        pthread_cancel(thread_chassi[i]);
    }

    return NULL;
}

void* encomenda_motor(void *data) {
    struct encomenda *dados = (struct encomenda*) data;

    int time;
    pthread_t thread_motor[dados->n_motores];
    pthread_t id_main_thread = pthread_self();
    for (int i = 0; i < dados->n_motores; i++) {
        time = rand()/TIME_DIV;
        usleep(time);
        pthread_create(&thread_motor[i], NULL, motor, (void *) dados->dados_montagem);
    }

    sleep(dados->tempo);
    printf("Linha de motores parou\n");
    for (int i = 0; i < dados->n_motores; i++) {
        pthread_cancel(thread_motor[i]);
    }

    return NULL;
}

void* encomenda_roda(void *data) {
    struct encomenda *dados = (struct encomenda*) data;

    int time;
    pthread_t thread_rodas[dados->n_rodas];
    pthread_t id_main_thread = pthread_self();
    int aux = 0;
    for (int i = 0; i < dados->n_rodas; i++) {
        time = rand()/TIME_DIV;
        usleep(time);
        pthread_create(&thread_rodas[i], NULL, roda, (void *) dados->dados_montagem);
    }

    sleep(dados->tempo);
    printf("Linha de rodas parou\n");
    for (int i = 0; i < dados->n_rodas; i++) {
        pthread_cancel(thread_rodas[i]);
    }

    return NULL;
}

int main() {
    printf("Indique o tempo de encomenda (em segundos), número de chassis, motores e rodas, respectivamente.\n");

    // Obtém o número de cada parte
    int tempo;
    scanf("%d", &tempo);
    int n_chassis;
    scanf("%d", &n_chassis);
    int n_motores;
    scanf("%d", &n_motores);
    int n_rodas;
    scanf("%d", &n_rodas);

    // Cria struct com valores compartilhados entre threads
    struct encomenda *dados_encomenda = malloc(sizeof(struct encomenda));

    // Atribui valores
    dados_encomenda->tempo = tempo;
    dados_encomenda->n_chassis = n_chassis;
    dados_encomenda->n_motores = n_motores;
    dados_encomenda->n_rodas = n_rodas;

    dados_encomenda->dados_montagem = malloc(sizeof(struct montagem));

    // Mutex para 'chassi', 'motor' e 'roda'
    pthread_mutex_t mutex_chassi;
    pthread_mutex_init(&mutex_chassi, NULL);
    dados_encomenda->dados_montagem->mutex_chassi = mutex_chassi;

    pthread_mutex_t mutex_motor;
    pthread_mutex_init(&mutex_motor, NULL);
    dados_encomenda->dados_montagem->mutex_motor = mutex_motor;

    pthread_mutex_t mutex_roda;
    pthread_mutex_init(&mutex_roda, NULL);
    dados_encomenda->dados_montagem->mutex_roda = mutex_roda;

    // Semáforo usado para threads 'rodas'
    sem_t semaforo_rodas;
    sem_init(&semaforo_rodas, 0, 4);
    dados_encomenda->dados_montagem->semaforo_roda = semaforo_rodas;

    // Semáforo usado para realizar 1 print por vez
    sem_t semaforo_print;
    sem_init(&semaforo_print, 0, 1);
    dados_encomenda->dados_montagem->semaforo_print = semaforo_print;

    // Define quantas threads estão na fila de espera da montagem.
    dados_encomenda->dados_montagem->thread_chassi_ativo = 0;
    dados_encomenda->dados_montagem->thread_motor_ativo = 0;
    dados_encomenda->dados_montagem->thread_roda_ativo = 0;

    // Define quantas threads estão em processamento para formar um carro.
    dados_encomenda->dados_montagem->chassis_em_processamento = 0;
    dados_encomenda->dados_montagem->motores_em_processamento = 0;
    dados_encomenda->dados_montagem->rodas_em_processamento = 0;

    // Número de carros montados
    dados_encomenda->dados_montagem->condicional_carro_montado = 0;
    dados_encomenda->dados_montagem->n_carros = 0;

    pthread_t encomendas[3];
    pthread_t id_main_thread = pthread_self();
    // Cria threads que simulam encomendas que chegam em tempos aleatórios
    pthread_create(&encomendas[0], NULL, encomenda_chassi, (void*) dados_encomenda);
    pthread_create(&encomendas[1], NULL, encomenda_motor, (void*) dados_encomenda);
    pthread_create(&encomendas[2], NULL, encomenda_roda, (void*) dados_encomenda);

    for (int i = 0; i < 3; i++) {
        pthread_join(encomendas[i], NULL);
    }

    int sobra_chassis = n_chassis - (dados_encomenda->dados_montagem->n_carros);
    int sobra_motores = n_motores - (dados_encomenda->dados_montagem->n_carros);
    int sobra_rodas = n_rodas - (4 * dados_encomenda->dados_montagem->n_carros);
    printf("Sobraram %d chassis\n", sobra_chassis);
    printf("Sobraram %d motores\n", sobra_motores);
    printf("Sobraram %d rodas\n", sobra_rodas);

    return 0;
}