// Arquivo que implementa a lógica de fila

// Tamanho máximo da fila
#define MAX_SIZE 10

// Estrutura para representar a fila
typedef struct {
    int *array;   // Array para armazenar elementos
    int front;     // Índice do elemento frontal
    int rear;      // Índice do elemento traseiro
    int size;      // Tamanho atual da fila
    int lenght;      // Tamanho total que a fila pode ter
} queue;

// Função para inicializar a fila
void initQueue(queue *queue, int maxSize) {
    queue->array = (int *)malloc(maxSize * sizeof(int));
    queue->front = -1;
    queue->rear = -1;
    queue->size = 0;
    queue->lenght = maxSize;
}

// Função para verificar se a fila está vazia
int isEmpty(const queue *queue) {
    return queue->size == 0;
}

// Função para verificar se a fila está cheia
int isFull(const queue *queue) {
    //printf("size atual: %d, length total: %d\n", queue->size, queue->lenght);
    return queue->size == queue->lenght;
}

// Função para verificar o tamanho atual
int getSize(const queue *queue) {
    return queue->size;
}

// Função para adicionar um elemento à fila
void push(queue *queue, int value) {
    if (isFull(queue)) {
        //printf("A fila está cheia. Não é possível adicionar %d.\n", value+1);
        return;
    }

    if (isEmpty(queue)) {
        queue->front = 0;  // Se a fila estiver vazia, define o front para 0
        queue->rear = 0;  // Se a fila estiver vazia, define o rear para 0
    } else {
        // Incrementa o rear e faz wrap-around se necessário
        queue->rear = (queue->rear + 1) % queue->lenght;
    }

    queue->array[queue->rear] = value;
    queue->size++;
    //printf("rear: %d front: %d\n", queue->array[queue->rear], queue->array[queue->front]);

    //printf("%d adicionado à fila.\n", value+1);
}

// Função para remover um elemento da fila
int pop(queue *queue) {
    if (isEmpty(queue)) {
        //printf("A fila está vazia. Não é possível remover.\n");
        return -1;  // Retorna um valor padrão indicando falha
    }

    int value = queue->array[queue->front];

    if (queue->front == queue->rear) {
        queue->front = -1;
        queue->rear = -1;
    } else {
        queue->front = (queue->front + 1) % queue->lenght;
    }
    
    queue->size--;

    //printf("%d removido da fila.\n", value+1);
    return value;
}

// função para pegar o primeiro da fila
int peek(queue *queue) {
    if (isEmpty(queue)) {
        //printf("A fila está vazia. Não há elementos para acessar.\n");
        return -1;  // Retorna um valor padrão indicando falha
    }

    return queue->array[queue->front];
}

// Função para liberar a memória alocada para a fila
void freeQueue(queue *queue) {
    free(queue->array);
    queue->front = -1;
    queue->rear = -1;
    queue->size = 0;
}
