#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/**
 * Process Control Block.
 *
 * PCB::label The process ID.
 * PCB::responded Whether the process has responded to the CPU.
 * PCB::at Arrival time.
 * PCB::bt Burst time.
 * PCB::ct Completion time.
 * PCB::rbt Remaining burst time.
 * PCB::rt Response time.
 * PCB::wt Waiting time.
 * PCB::tat Turnaround time.
 * PCB::st Start time.
 * PCB::et End time.
 */
struct PCB {
    int label, responded;
    float at, bt, ct, rbt, rt, wt, tat, st, et;
};

/**
 * A process node in the process queue.
 */
struct ProcessNode {
    struct PCB current;
    struct ProcessNode* next;
};

/**
 * A queue of processes.
 */
struct Queue {
    struct ProcessNode* front;
    struct ProcessNode* rear;
};

/**
 * A block in the Gantt Chart.
 *
 * The Gantt Chart is formatted in such a way where each block leading column will be the start time of the process,
 * and the trailing column will be the start time of the next process block. Only the trailing column of the final block
 * will be the end time of the final process.
 */
struct GanttNode {
    int label;
    float st, et;
    char* queueLevel;
    struct GanttNode* next;
};

/**
 * The result of the scheduling algorithm.
 */
struct SolutionResult {
    struct Queue* queue;
    struct GanttNode* ganttChart;
    int ganttChartSize;
};

/**
 * Manipulates the process and its nodes.
 */
struct ProcessNode* createProcess(int label, float arrivalTime, float burstTime);
void updateProcessTime(struct Queue* q, float runtime);

/**
 * Solves the scheduling problem using the given algorithm.
 */
struct SolutionResult solveFCFS(struct Queue* waitingQueue, float* runtime);
struct SolutionResult solveRR(struct Queue* waitingQueue, float quantum, float* runtime);
struct SolutionResult solveMLFQ(struct Queue* waitingQueue, float quantum, float* runtime);

/**
 * Manipulates the queue of processes.
 */
struct Queue* createQueue();
struct Queue* duplicateQueue(struct Queue* queue);
void sortQueueByArrivalTime(struct Queue* queue, int processCount);
void enqueue(struct Queue* q, struct PCB data);
struct ProcessNode* peek(struct Queue* q);
struct PCB dequeue(struct Queue* q);
void freeQueue(struct Queue* q);
void emptyQueue(struct Queue* q);
int isEmpty(struct Queue* q);

/**
 * Used for dynamic table formatting and other micro console visual enhancements.
 */
int intLength(int value);
int getInput(char* input, const char* prompt, int (*validateFunc)(char*), const char* errorMsg);
int isValidAlgorithm(char* input);
int isValidProcessCount(char* input);
int isValidTime(char* input);
int isValidQuantum(char* input);

struct Queue* createQueue() {
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    if (q) {
        q->front = q->rear = NULL;
    }
    return q;
}

struct Queue* duplicateQueue(struct Queue* queue) {
    struct Queue* newQueue = createQueue();
    struct ProcessNode* current = queue->front;

    while (current != NULL) {
        struct PCB newPCB = current->current;
        enqueue(newQueue, newPCB);
        current = current->next;
    }

    return newQueue;
}

void sortQueueByArrivalTime(struct Queue* queue, int processCount) {
    if (queue == NULL || isEmpty(queue)) return;

    struct ProcessNode** tempArray = (struct ProcessNode**) calloc(processCount, sizeof(struct ProcessNode*));
    if (tempArray == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Dequeue all elements into an array
    for (int i = 0; i < processCount; i++) {
        tempArray[i] = (struct ProcessNode*) malloc(sizeof(struct ProcessNode));
        if (tempArray[i] == NULL) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }

        tempArray[i]->current = dequeue(queue);
        tempArray[i]->next = NULL;
    }

    // Perform bubble sort on the array
    for (int i = 0; i < processCount - 1; i++) {
        for (int j = 0; j < processCount - i - 1; j++) {
            if (tempArray[j]->current.at > tempArray[j + 1]->current.at) {
                // Swap tempArray[j] and tempArray[j + 1]
                struct ProcessNode* temp = tempArray[j];
                tempArray[j] = tempArray[j + 1];
                tempArray[j + 1] = temp;
            }
        }
    }

    // Re-enqueue the elements into the queue in sorted order
    for (int i = 0; i < processCount; i++) {
        enqueue(queue, tempArray[i]->current);
        free(tempArray[i]);
    }

    free(tempArray);
}

// Enqueue a PCB into a queue
void enqueue(struct Queue* q, struct PCB data) {
    struct ProcessNode* newNode = (struct ProcessNode*)malloc(sizeof(struct ProcessNode));
    if (newNode) {
        newNode->current = data;
        newNode->next = NULL;

        if (q->rear == NULL) { // If the queue is empty
            q->front = q->rear = newNode;
        } else {
            q->rear->next = newNode;
            q->rear = newNode;
        }
    }
}

struct ProcessNode* peek(struct Queue* q) {
    return q->front;
}

struct PCB dequeue(struct Queue* q) {
    if (q->front == NULL) {
        return (struct PCB){0}; // Return an empty PCB if the queue is empty
    }

    struct ProcessNode* temp = q->front;
    struct PCB data = temp->current;

    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return data;
}

// Check if a queue is empty
int isEmpty(struct Queue* q) {
    return q->front == NULL;
}

void freeQueue(struct Queue* q) {
    emptyQueue(q);
    free(q);
}

void emptyQueue(struct Queue* q) {
    while (!isEmpty(q)) {
        dequeue(q);
    }
}

struct ProcessNode* createProcess(int label, float arrivalTime, float burstTime) {
    struct PCB pcb;
    pcb.label = label;
    pcb.at = arrivalTime;
    pcb.bt = burstTime;
    pcb.ct = 0;
    pcb.rbt = burstTime;
    pcb.rt = 0;
    pcb.wt = 0;
    pcb.tat = 0;
    pcb.responded = 0;

    struct ProcessNode* node = (struct ProcessNode*)malloc(sizeof(struct ProcessNode));
    if (!node) {
        fprintf(stderr, "Failed to allocate memory for new process node.\n");
        exit(EXIT_FAILURE);
    }

    node->current = pcb;
    node->next = NULL;
    return node;
}

void updateProcessTime(struct Queue* q, float runtime) {
    // Check if the q is empty
    if (q == NULL || q->front == NULL) {
        return;
    }

    // Iterate through each node in the q
    struct ProcessNode* node = q->front;
    while (node != NULL) {
        struct PCB* current = &(node->current);

        // Only increment waiting time for processes that have arrived
        if (runtime >= current->at) {
            if (current->responded == 0) {
                current->rt += 1;
            }
            current->wt += 1;
        }

        node = node->next;
    }
}

struct SolutionResult solveFCFS(struct Queue* waitingQueue, float* runtime) {
    struct Queue* runningQueue = createQueue();
    struct Queue* completedQueue = createQueue();

    struct GanttNode* ganttChart = NULL;
    int ganttChartSize = 0;
    int begin = 0; // Track whether the processing has started;

    float lastRuntime = *runtime - 1;

    while (!isEmpty(waitingQueue) || !isEmpty(runningQueue)) {
        // Check for CPU idle time
        if (begin == 1 && isEmpty(runningQueue) && !isEmpty(waitingQueue) && lastRuntime >= 0) {
            float arrivalTime = peek(waitingQueue)->current.at;

            if (ganttChartSize > 0 && ganttChart[ganttChartSize - 1].label == -1) {
                // Update the previous block's et, since it's also an idle block
                ganttChart[ganttChartSize - 1].et = arrivalTime;
            } else {
                // Create a new idle block as CPU is idle until the next process arrives.
                ganttChart = realloc(ganttChart, sizeof(struct GanttNode) * (ganttChartSize + 1));
                ganttChart[ganttChartSize].label = -1; // Label for idle time
                ganttChart[ganttChartSize].st = lastRuntime;
                ganttChart[ganttChartSize].et = arrivalTime;
                ganttChart[ganttChartSize].queueLevel = NULL;
                ganttChartSize++;
            }
        }

        // If a process is running, continue
        if (!isEmpty(runningQueue)) {
            struct PCB* current = &peek(runningQueue)->current;

            // Process starts
            if (current->rbt == current->bt) {
                ganttChart = realloc(ganttChart, sizeof(struct GanttNode) * (ganttChartSize + 1));
                ganttChart[ganttChartSize].label = current->label;
                ganttChart[ganttChartSize].st = lastRuntime;
                ganttChart[ganttChartSize].queueLevel = NULL;
            }

            // Process starts or continues
            if (current->responded == 0) {
                current->responded = 1; // Mark as responded
                current->rt = lastRuntime - current->at;

                updateProcessTime(runningQueue, *runtime);

                current->st = lastRuntime;
            }

            // Update the process time
            current->rbt -= 1;

            // Process ends
            if (current->rbt == 0) {
                current->et = *runtime;
                ganttChart[ganttChartSize].et = *runtime;
                ganttChartSize++;

                struct PCB completedProcess = dequeue(runningQueue);
                completedProcess.ct = *runtime;
                completedProcess.tat = completedProcess.ct - completedProcess.at;
                completedProcess.wt = completedProcess.tat - completedProcess.bt;

                enqueue(completedQueue, completedProcess);
            }
        }

        // Check for new arrivals
        while (!isEmpty(waitingQueue) && peek(waitingQueue)->current.at == *runtime) {
            struct PCB arrivedProcess = dequeue(waitingQueue);
            arrivedProcess.st = lastRuntime;
            enqueue(runningQueue, arrivedProcess);
            begin = 1;
        }

        // Increment time
        (*runtime)++;
        lastRuntime++;

        if (isEmpty(waitingQueue) && isEmpty(runningQueue)) {
            break;
        }
    }

    return (struct SolutionResult){completedQueue, ganttChart, ganttChartSize};
}

struct SolutionResult solveRR(struct Queue* waitingQueue, float quantum, float* runtime) {
    struct Queue* runningQueue = createQueue();
    struct Queue* completedQueue = createQueue();
    float quantumLeft = quantum;

    struct GanttNode* ganttChart = NULL;
    int ganttChartSize = 0;
    int begin = 0; // Track whether the processing has started;

    float lastRuntime = *runtime - 1;

    while (!isEmpty(waitingQueue) || !isEmpty(runningQueue)) {
        // Check for CPU idle time
        if (begin == 1 && isEmpty(runningQueue) && !isEmpty(waitingQueue) && lastRuntime >= 0) {
            float arrivalTime = peek(waitingQueue)->current.at;

            if (ganttChartSize > 0 && ganttChart[ganttChartSize - 1].label == -1) {
                // Update the previous block's et, since it's also an idle block
                ganttChart[ganttChartSize - 1].et = arrivalTime;
            } else {
                // Create a new idle block as CPU is idle until the next process arrives.
                ganttChart = realloc(ganttChart, sizeof(struct GanttNode) * (ganttChartSize + 1));
                ganttChart[ganttChartSize].label = -1; // Label for idle time
                ganttChart[ganttChartSize].st = lastRuntime;
                ganttChart[ganttChartSize].et = arrivalTime;
                ganttChart[ganttChartSize].queueLevel = NULL;
                ganttChartSize++;
            }
        }

        if (!isEmpty(runningQueue)) {
            struct PCB* current = &peek(runningQueue)->current;

            // Processing restarts its time slice from the allocated quantum.
            if (quantumLeft == quantum) {
                ganttChart = realloc(ganttChart, sizeof(struct GanttNode) * (ganttChartSize + 1));
                ganttChart[ganttChartSize].label = current->label;
                ganttChart[ganttChartSize].st = lastRuntime;
                ganttChart[ganttChartSize].queueLevel = NULL;
                ganttChartSize++;
            }

            if (current->responded == 0) {
                current->responded = 1;
                current->st = lastRuntime;
            }

            current->rbt -= 1;
            quantumLeft -= 1;

            updateProcessTime(runningQueue, *runtime);

            // Process ends
            if (current->rbt == 0) {
                current->et = *runtime;
                current->ct = *runtime;
                current->tat = current->ct - current->at;
                current->wt = current->tat - current->bt;

                enqueue(completedQueue, *current);
                dequeue(runningQueue);

                // Reset quantum for the next process
                quantumLeft = quantum;

                ganttChart[ganttChartSize - 1].et = *runtime;
            }
        }

        while (!isEmpty(waitingQueue) && peek(waitingQueue)->current.at == *runtime) {
            struct PCB arrived = dequeue(waitingQueue);

            // Set the start time for the process if it's the first time being processed
            if (arrived.responded == 0) {
                arrived.st = *runtime;
            }
            enqueue(runningQueue, arrived);
            begin = 1;
        }

        if (quantumLeft == 0) {
            struct PCB* current = &peek(runningQueue)->current;
            current->et = *runtime;

            struct PCB incomplete = dequeue(runningQueue);
            enqueue(runningQueue, incomplete);

            quantumLeft = quantum;

            ganttChart[ganttChartSize - 1].et = *runtime;
        }

        (*runtime)++;
        lastRuntime++;

        if (isEmpty(waitingQueue) && isEmpty(runningQueue)) {
            break;
        }
    }

    return (struct SolutionResult){completedQueue, ganttChart, ganttChartSize};
}

struct SolutionResult solveMLFQ(struct Queue* waitingQueue, float quantum, float* runtime) {
    struct Queue* runningRRQueue = createQueue(); // high priority queue
    struct Queue* runningFCFSQueue = createQueue(); // low priority queue
    struct Queue* completedQueue = createQueue();
    float quantumLeft = quantum;

    struct GanttNode* ganttChart = NULL;
    int ganttChartSize = 0;
    int begin = 0; // Track whether the processing has started;

    float lastRuntime = *runtime - 1;

    while (!isEmpty(waitingQueue) || !isEmpty(runningRRQueue) || !isEmpty(runningFCFSQueue)) {
        // Check for CPU idle time
        if (begin == 1 && isEmpty(runningRRQueue) && isEmpty(runningFCFSQueue) && !isEmpty(waitingQueue) && lastRuntime >= 0) {
            float at = peek(waitingQueue)->current.at;

            if (ganttChartSize > 0 && ganttChart[ganttChartSize - 1].label == -1) {
                // Update the previous block's end time, since it's also an idle block.
                ganttChart[ganttChartSize - 1].et = at;
            } else {
                // Create a new idle block as CPU is idle until the next process arrives.
                ganttChart = realloc(ganttChart, sizeof(struct GanttNode) * (ganttChartSize + 1));
                ganttChart[ganttChartSize].label = -1; // Label for idle time
                ganttChart[ganttChartSize].st = lastRuntime;
                ganttChart[ganttChartSize].et = at;
                ganttChart[ganttChartSize].queueLevel = NULL;
                ganttChartSize++;
            }
        }

        if (!isEmpty(runningRRQueue)) {
            struct PCB* current = &peek(runningRRQueue)->current;

            // Processing restarts its time slice from the allocated quantum.
            if (current->responded == 0 || quantumLeft == quantum) {
                current->responded = 1;
                current->st = lastRuntime;

                ganttChart = realloc(ganttChart, sizeof(struct GanttNode) * (ganttChartSize + 1));
                ganttChart[ganttChartSize].label = current->label;
                ganttChart[ganttChartSize].st = lastRuntime;
                ganttChart[ganttChartSize].queueLevel = "RR";
                ganttChartSize++;
            }

            current->rbt -= 1;
            quantumLeft -= 1;

            updateProcessTime(runningRRQueue, *runtime);
            updateProcessTime(runningFCFSQueue, *runtime);

            if (current->rbt == 0) {
                current->et = *runtime;
                current->ct = *runtime;
                current->tat = current->ct - current->at;
                current->wt = current->tat - current->bt;

                enqueue(completedQueue, *current);
                dequeue(runningRRQueue);

                quantumLeft = quantum;

                ganttChart[ganttChartSize - 1].et = *runtime;
            } else if (quantumLeft == 0) {
                struct PCB incomplete = dequeue(runningRRQueue);
                enqueue (runningFCFSQueue, incomplete);

                quantumLeft = quantum;

                ganttChart[ganttChartSize - 1].et = *runtime;

                // Marks the demotion to FCFS.
                ganttChart = realloc(ganttChart, sizeof(struct GanttNode) * (ganttChartSize + 1));
                ganttChart[ganttChartSize].label = incomplete.label;
                ganttChart[ganttChartSize].st = *runtime;
                ganttChart[ganttChartSize].et = *runtime;
                ganttChart[ganttChartSize].queueLevel = "FCFS";
                ganttChartSize++;
            }
        } else if (!isEmpty(runningFCFSQueue)) {
            struct PCB* current = &peek(runningFCFSQueue)->current;

            // Processing restarts its time slice from the allocated quantum.
            if (current->responded == 0) {
                current->responded = 1;
                current->st = lastRuntime;

                ganttChart[ganttChartSize - 1].et = *runtime;

                ganttChart = realloc(ganttChart, sizeof(struct GanttNode) * (ganttChartSize + 1));
                ganttChart[ganttChartSize].label = current->label;
                ganttChart[ganttChartSize].st = lastRuntime;
                ganttChart[ganttChartSize].queueLevel = "FCFS";
                ganttChartSize++;
            }

            current->rbt -= 1;

            updateProcessTime(runningFCFSQueue, *runtime);

            if (current->rbt == 0) {
                current->et = *runtime;
                current->ct = *runtime;
                current->tat = current->ct - current->at;
                current->wt = current->tat - current->bt;

                enqueue(completedQueue, *current);
                dequeue (runningFCFSQueue);

                ganttChart[ganttChartSize - 1].et = *runtime;
            }
        }

        while (!isEmpty(waitingQueue) && peek(waitingQueue)->current.at == *runtime) {
            struct PCB arrived = dequeue(waitingQueue);

            // Set the start time for the process if it's the first time being processed
            if (arrived.responded == 0) {
                arrived.st = *runtime;
            }
            enqueue(runningRRQueue, arrived);
            begin = 1;
        }

        (*runtime)++;
        lastRuntime++;

        if (isEmpty(waitingQueue) && isEmpty(runningRRQueue) && isEmpty(runningFCFSQueue)) {
            break;
        }
    }

    return (struct SolutionResult){completedQueue, ganttChart, ganttChartSize};
}

int intLength(int value) {
    if (value == 0) return 1;
    value = abs(value);
    return floor(log10(value)) + 1;
}

int getInput(char* input, const char* prompt, int (*validateFunc)(char*), const char* errorMsg) {
    while (1) {
        printf("%s", prompt);
        scanf("%s", input);

        if (strcmp(input, "q") == 0) {
            printf("\nProgram terminated.\n");
            return 0; // Quit signal
        } else if (strcmp(input, "r") == 0) {
            return 1; // Restart signal
        }

        if (!validateFunc(input)) {
            printf("%s\n\n", errorMsg);
            continue;
        }

        return 2; // Valid input signal
    }
}

int isValidAlgorithm(char* input) {
    int algorithm = atoi(input);
    return (algorithm == 1 || algorithm == 2 || algorithm == 3);
}

int isValidProcessCount(char* input) {
    int count = atoi(input);
    return (count > 0);
}

int isValidTime(char* input) {
    float time = atof(input);
    return (time >= 0);
}

int isValidQuantum(char* input) {
    float quantum = atof(input);
    return (quantum > 0);
}

int main() {
    printf("\n\n");
    printf("                    ---------- [ CPU SCHEDULING ALGORITHMS ] ----------                    \n\n");
    printf("[Enter \"q\" to quit or \"r\" to restart the program at any time.]\n");

    char input[100];

    while (1) {
        struct Queue* waitingQueue = createQueue();

        int algorithm, processCounter = 0;
        float quantum = 0;
        float runtime = -1;

        printf("\n");
        printf("                    -------------------- [ Input ] --------------------                    \n\n");
        printf("Choose the scheduling algorithm:\n");
        printf("1. First Come First Served (FCFS)\n");
        printf("2. Round Robin (RR)\n");
        printf("3. Multi-Level Feedback Queue (MLFQ)\n");

        int inputSignal = getInput(input, "Enter your choice of algorithm: ", isValidAlgorithm, "Invalid choice of algorithm.");
        if (inputSignal == 1) {
            continue;
        } else if (inputSignal == 0) {
            return inputSignal;
        }
        algorithm = atoi(input);

        printf("\n");

        inputSignal = getInput(input, "Enter the total number of processes: ", isValidProcessCount, "Invalid number of processes.");
        if (inputSignal == 1) {
            continue;
        } else if (inputSignal == 0) {
            return inputSignal;
        }
        processCounter = atoi(input);

        printf ("\n");

        for (int i = 0; i < processCounter; i++) {
            float arrivalTime, burstTime;

            printf("Process No. %d: \n", i + 1);

            inputSignal = getInput(input, "Enter arrival time: ", isValidTime, "Invalid arrival time.");
            if (inputSignal == 1) {
                break;
            } else if (inputSignal == 0) {
                return inputSignal;
            }
            arrivalTime = atof(input);

            inputSignal = getInput(input, "Enter burst time: ", isValidTime, "Invalid burst time.");
            if (inputSignal == 1) {
                break;
            } else if (inputSignal == 0) {
                return inputSignal;
            }
            burstTime = atof(input);

            printf("\n");

            struct ProcessNode* node = createProcess(i + 1, arrivalTime, burstTime);
            enqueue(waitingQueue, node->current);
            free(node);
        }

        if (inputSignal == 1) {
            continue;
        }

        if (algorithm == 2 || algorithm == 3) {
            inputSignal = getInput(input, "Enter time quantum: ", isValidQuantum, "Invalid time quantum.");
            if (inputSignal == 1) {
                continue;
            } else if (inputSignal == 0) {
                return inputSignal;
            }
            quantum = atof(input);
        }

        printf("\n");

        struct Queue* initialWaitingQueue = duplicateQueue(waitingQueue);
        sortQueueByArrivalTime(waitingQueue, processCounter);

        struct SolutionResult result;

        switch (algorithm) {
            case 1:
                result = solveFCFS(waitingQueue, &runtime);
                break;
            case 2:
                result = solveRR(waitingQueue, quantum, &runtime);
                break;
            case 3:
                result = solveMLFQ(waitingQueue, quantum, &runtime);
                break;
        }

        printf("                    -------------------- [ Output ] --------------------                    \n\n");
        switch (algorithm) {
            case 1:
                printf("                    ------------ [ First Come First Served ] ------------                    \n\n");
                break;
            case 2:
                printf("                    ----------------- [ Round Robin ] -----------------                    \n\n");
                break;
            case 3:
                printf("                    ---------- [ Multi-Level Feedback Queue ] ----------                    \n\n");
                break;
        }

        printf("Initial Queue");
        printf(algorithm == 1 ? ": \n" : " (with time quantum of %.0f): \n", quantum);
        printf("+------------+--------------+------------+\n");
        printf("| Process ID | Arrival Time | Burst Time |\n");
        printf("+------------+--------------+------------+\n");
        while (!isEmpty(initialWaitingQueue)) {
            struct PCB current = dequeue(initialWaitingQueue);
            printf("| %-10d | %-12.0f | %-10.0f |\n", current.label, current.at, current.bt);
        }
        printf("+------------+--------------+------------+\n\n");
        freeQueue(initialWaitingQueue);

        printf("Process Start and End Times (in ascending order of completion):\n");
        printf("+------------+------------+----------+\n");
        printf("| Process ID | Start Time | End Time |\n");
        printf("+------------+------------+----------+\n");
        struct Queue* completedQueue = result.queue;
        struct ProcessNode* process = completedQueue->front;
        while (process != NULL) {
            printf("| %-10d | %-10.0f | %-8.0f |\n", process->current.label, process->current.st, process->current.et);
            process = process->next;
        }
        printf("+------------+------------+----------+\n\n");

        struct GanttNode* ganttChart = result.ganttChart;
        int ganttChartSize = result.ganttChartSize;
        int ganttBlocksPerLine = 10;
        int ganttBlocksLeftOnLine = ganttBlocksPerLine;
        int totalIdleBlocks = 0;
        int totalExecutionBlocks = 0;

        printf("Gantt Chart (with time units");
        printf(algorithm == 3 ? " and queue levels):\n" : "):\n");
        int ganttBlocksInCurrentLine = 0;

        for (int i = 0; i < ganttChartSize; ++i) {
            if (ganttBlocksInCurrentLine == 0) {
                printf("+");
            }

            int label = ganttChart[i].label;
            if (label == -1) {
                totalIdleBlocks++;
            } else {
                totalExecutionBlocks++;
            }

            int timeUnitLen = intLength((int) ganttChart[i].et - (int) ganttChart[i].st);

            int totalDashes = label == -1 ? 9 : 7;
            if (ganttChart[i].queueLevel != NULL) {
                totalDashes += strlen(ganttChart[i].queueLevel) + 1;
            }
            totalDashes += timeUnitLen;

            for (int k = 0; k < totalDashes; ++k) {
                printf("-");
            }
            printf("+");
            ganttBlocksInCurrentLine++;

            if (ganttBlocksInCurrentLine >= ganttBlocksPerLine || i == ganttChartSize - 1) {
                printf("\n|");

                // Process labels for the current line
                for (int j = i - ganttBlocksInCurrentLine + 1; j <= i; ++j) {
                    int label = ganttChart[j].label;
                    int timeUnit = (int) ganttChart[j].et - (int) ganttChart[j].st;
                    char* queueLevel = ganttChart[j].queueLevel;

                    if (label == -1) {
                        printf(" Idle [%d] |", timeUnit);
                    } else {
                        if (queueLevel == NULL) {
                            printf(" P%-d [%d] |", label, timeUnit);
                        } else {
                            printf(" %s P%-d [%d] |", queueLevel, label, timeUnit);
                        }
                    }
                }

                printf("\n+");

                for (int j = i - ganttBlocksInCurrentLine + 1; j <= i; ++j) {
                    int label = ganttChart[j].label;
                    char* queueLevel = ganttChart[j].queueLevel;

                    if (label == -1) {
                        printf("---------");
                    } else {
                        printf("-------");
                        if (queueLevel != NULL) {
                            for (int k = 0; k < strlen(queueLevel) + 1; ++k) {
                                printf("-");
                            }
                        }
                    }

                    int timeUnitLen = intLength((int) ganttChart[j].et - (int) ganttChart[j].st);
                    for (int k = 0; k < timeUnitLen; ++k) {
                        printf("-");
                    }
                    printf("+");
                }

                // Time units for the current line of blocks
                printf("\n");
                for (int j = i - ganttBlocksInCurrentLine + 1; j <= i; ++j) {
                    int label = ganttChart[j].label;
                    int timeUnitLen = intLength((int) ganttChart[j].et - (int) ganttChart[j].st);
                    char* queueLevel = ganttChart[j].queueLevel;
                    int cellSize = (label == -1 ? 10 : 8) + timeUnitLen;
                    if (queueLevel != NULL) {
                        cellSize += strlen(queueLevel) + 1;
                    }

                    printf("%-*d", cellSize, (int) ganttChart[j].st);
                }

                if (ganttBlocksInCurrentLine == ganttBlocksPerLine || i == ganttChartSize - 1) {
                    printf("%-8d", (int) ganttChart[i].et);
                }

                printf("\n");

                ganttBlocksInCurrentLine = 0;
            }
        }

        printf("\n");

        printf("+------------------+-------------+\n");
        printf("| Execution Blocks | Idle Blocks |\n");
        printf("+------------------+-------------+\n");
        printf("| %-16d | %-11d |\n", totalExecutionBlocks, totalIdleBlocks);
        printf("+------------------+-------------+\n\n");

        float averageRT = 0, averageWT = 0, averageTAT = 0;

        printf("Scheduling Calculation Information (in ascending order of completion): \n");
        printf("+------------+--------------+------------+-----------------+-----------------+--------------+---------------+\n");
        printf("| Process ID | Arrival Time | Burst Time | Completion Time | Turnaround Time | Waiting Time | Response Time |\n");
        printf("+------------+--------------+------------+-----------------+-----------------+--------------+---------------+\n");

        struct Queue* tempQueue = duplicateQueue(completedQueue);
        while (!isEmpty(tempQueue)) {
            struct PCB current = dequeue(tempQueue);
            averageRT += current.rt;
            averageWT += current.wt;
            averageTAT += current.tat;

            printf("| %-10d | %-12.0f | %-10.0f | %-15.0f | %-15.0f | %-12.0f | %-13.0f |\n",
                   current.label, current.at, current.bt,
                   current.ct, current.tat, current.wt, current.rt);
        }

        printf("+------------+--------------+------------+-----------------+-----------------+--------------+---------------+\n\n");

        averageRT /= (float) processCounter;
        averageWT /= (float) processCounter;
        averageTAT /= (float) processCounter;
        float throughput = (float) processCounter / runtime;

        printf("Scheduling Calculation Performance (in milliseconds): \n");
        printf("+-----------------------+----------------------+-------------------------+------------+\n");
        printf("| Average Response Time | Average Waiting Time | Average Turnaround Time | Throughput |\n");
        printf("+-----------------------+----------------------+-------------------------+------------+\n");
        printf("| %-21.3f | %-20.3f | %-23.3f | %-10.3f |\n",
               averageRT, averageWT, averageTAT, throughput);
        printf("+-----------------------+----------------------+-------------------------+------------+\n");

        freeQueue(waitingQueue);
        freeQueue(completedQueue);
    }
}