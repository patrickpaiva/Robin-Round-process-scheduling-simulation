#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.C"
#include "process.C"


// Constantes definidas pelo grupo
#define NUMBER_OF_PROCESSES 3
#define MAX_COLUMNS 5
#define DISC_TIME 2
#define MAGNETIC_TAPE_TIME 3
#define PRINTER_TIME 4
#define TIME_SLICE 2




// Define filas a serem utilizadas
queue cpu_running_queue;
queue process_high_priority_queue;
queue process_low_priority_queue;
queue IO_disc_queue;
queue IO_magnetic_tape_queue;
queue IO_printer_queue;

// Array dos processos
process processArray[NUMBER_OF_PROCESSES]; 

// Variavel com o atual numero de processos e I/Os concluidos
int completed = 0;                              

// Armazena processos da simulacao em um array
void initializeAllProcesses() {
    FILE *arquivo;
    arquivo = fopen("pconfig.txt", "r");

    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    char linha[100];
    int processesLoadedCounter = 0;

    // Ler e ignorar a primeira linha (cabeçalho)
    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        printf("Erro ao ler a primeira linha (cabeçalho).\n");
        fclose(arquivo);
        return;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        int valores[MAX_COLUMNS];
        int lidos = sscanf(linha, "%d %d %d %d %d %d", 
                           &valores[0], &valores[1], &valores[2],
                           &valores[3], &valores[4], &valores[5]);

        if (lidos == MAX_COLUMNS) {
            processArray[processesLoadedCounter] = initializeProcess(processesLoadedCounter, valores);
        } else {
            printf("Erro ao ler a linha do arquivo.\n");
            return;
        }
        processesLoadedCounter++;
    }

    fclose(arquivo);
}
int checkNewIO(int PID){
    if(processArray[PID].hasIO == 1){
        if(processArray[PID].IORequestTime == processArray[PID].CPURunTime){
            int type = processArray[PID].IOType;
            printf("Processo %d requisitou IO\n", PID+1);
            switch(type){
                case 1:
                    push(&IO_disc_queue, PID);
                    printf("    adicionado na fila de IO: Disco\n");
                    return 1;
                case 2:
                    push(&IO_magnetic_tape_queue, PID);
                    printf("    adicionado na fila de IO: Fita Magnetica\n");
                    return 1;
                case 3:
                    push(&IO_printer_queue, PID);
                    printf("    adicionado na fila de IO: Impressora\n");
                    return 1;
            }
        }
    }
    return 0;
}

void checkNewProcesses(int clock){
    for(int i=0; i<NUMBER_OF_PROCESSES; i++){
        if(processArray[i].arrivalTime == clock){
            int PID = processArray[i].PID;
            printf("Processo %d chegou\n", PID+1);
            if(checkNewIO(PID)==1){
                return;
            }
            push(&process_high_priority_queue, PID);
            printf("    adicionado na fila de alta prioridade\n");
            processArray[i].priority=1;
        }
    }
}

int checkIORequest(int PID, queue *q){
    if(processArray[PID].hasIO == 1){
        if(processArray[PID].IORequestTime == processArray[PID].CPURunTime){
            int type = processArray[PID].IOType;
            printf("Processo %d requisitou IO\n", PID+1);
            switch(type){
                case 1:
                    push(&IO_disc_queue, pop(q));
                    printf("    adicionado na fila de IO: Disco\n");
                    return 1;
                case 2:
                    push(&IO_magnetic_tape_queue, pop(q));
                    printf("    adicionado na fila de IO: Fita Magnetica\n");
                    return 1;
                case 3:
                    push(&IO_printer_queue, pop(q));
                    printf("    adicionado na fila de IO: Impressora\n");
                    return 1;
            }
        }
    }
    return 0;
}

void runProcess(queue *q){
    if (isEmpty(q)){
        return;
    }
    int PID = peek(q);
    
    int newTime = processArray[PID].CPURunTime + 1;
    int targetTime = processArray[PID].CPUTime;
    printf("Executando processo %d\n", PID+1);
    if(newTime==targetTime){
        printf("    processo %d finalizado\n", PID+1);
        pop(q);
        completed++;
        return;
    }
    processArray[PID].CPURunTime = newTime;
    processArray[PID].timeSliceCounter = processArray[PID].timeSliceCounter+1;
    if(checkIORequest(PID, q)==1){
        processArray[PID].timeSliceCounter = 0;
        return;
    }
    if(processArray[PID].timeSliceCounter==TIME_SLICE){
        push(&process_low_priority_queue, PID);
        pop(q);
        printf("    processo %d movido para fila de baixa prioridade\n", PID+1);
        processArray[PID].priority=0;
        return;
    }
    pop(q);
    push(&cpu_running_queue, PID);
    return;
}

void runIO(queue q, int type){
    if(isEmpty(&q)) { return; }
    int PID = peek(&q);
    if(processArray[PID].IOIsRunning==1) { return; }
    printf("Marca para iniciar IO %d\n", PID+1);
    processArray[PID].IOIsRunning = 1;
}

void schedule(int clock){
    // Verifica se alguma operacao de IO esta sendo executada
    for(int i=0; i<NUMBER_OF_PROCESSES; i++){
        if(processArray[i].IOIsRunning == 1){
            int PID = processArray[i].PID;
            printf("Executando IO %d\n", PID+1);
            int newTime = processArray[i].IOTimeLeft - 1;
            processArray[i].IOTimeLeft = newTime; 
            if(processArray[i].IOTimeLeft == 0){
                int type = processArray[i].IOType;
                printf("    IO do processo %d terminou\n", PID+1);
                processArray[i].IOIsRunning = 0;
                processArray[i].hasIO = 0;
                switch(type){
                    case 1:
                        push(&process_low_priority_queue, pop(&IO_disc_queue));
                        printf("        processo %d movido para fila de baixa prioridade\n", PID+1);
                        processArray[PID].priority=0;
                    case 2:
                        push(&process_high_priority_queue, pop(&IO_magnetic_tape_queue));
                        printf("        processo %d movido para fila de alta prioridade\n", PID+1);
                        processArray[PID].priority=1;
                    case 3:
                        push(&process_high_priority_queue, pop(&IO_printer_queue));
                        printf("        processo %d movido para fila de alta prioridade\n", PID+1);
                        processArray[PID].priority=1;
                }
            }
        }
    }
    // Verifica qual processo será executado neste instante de tempo
    if(isFull(&cpu_running_queue)){ // CENÁRIO 1 (CPU AINDA OCUPADA POIS PROCESSO NAO COMPLETOU DENTRO DO TIME SLICE)
        runProcess(&cpu_running_queue);
    } else { // CENÁRIO 2 - ESCOLHE DAS DUAS FILAS DE PRIORIDADE
        if(isEmpty(&process_high_priority_queue)){
            runProcess(&process_low_priority_queue); 
        } else {
            runProcess(&process_high_priority_queue); 
        }
    }
    // Executa IOs e aloca nas filas apropriadas
    runIO(IO_disc_queue ,1);
    runIO(IO_magnetic_tape_queue ,2);
    runIO(IO_printer_queue ,3);
}


int main (){
    int clock = 0;                                     // Inicializa clock

    // Inicializa filas
    initQueue(&cpu_running_queue, 1);
    initQueue(&process_high_priority_queue, MAX_SIZE);
    initQueue(&process_low_priority_queue, MAX_SIZE);
    initQueue(&IO_disc_queue, MAX_SIZE);
    initQueue(&IO_magnetic_tape_queue, MAX_SIZE);
    initQueue(&IO_printer_queue, MAX_SIZE);
    

    initializeAllProcesses();                          // Inicializa processos

    // Inicia simulacao de processameto
    while(completed != NUMBER_OF_PROCESSES){
        printf("ROUND %d ------------------------------------\n", clock+1);
        // Busca por novos processos
        checkNewProcesses(clock);
        // Chama o scheduler
        schedule(clock);
        clock++;
    }

    return 0;
}
