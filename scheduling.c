#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// 변수명에 대하여
// 1. 줄임말은 반드시 대문자로 작성
// 2. Struct(Class)의 경우 첫글자만 대문자로 작성
// 3. 함수의 경우 단어 단위로 첫글자만 대문자 작성 (예시 : Happy_Birthday)
// 4. 그 외에 일반 변수는 모두 소문자로 작성

typedef struct Process Process;
Process *Create_Process(int PID, int CPU_burst_time, int IO_burst_time, int arrival_time, int priority);
typedef struct Node Node;
Node *Create_Node();
typedef struct Queue Queue;
Queue *Create_Queue();
void Push_Queue(Queue *queue, Process *process);
Process *Pop_Queue(Queue *queue);
void Empty_Queue(Queue *queue);
Process *Copy_Process(Process *process);
Queue *Copy_Queue(Queue *queue);
void Sort_Queue(Queue *queue, int type);

typedef struct Limit Limit;
Limit *limit;
void Set_Limit(int cnt_process, int CPU_burst_time, int IO_burst_time, int arrival_time, int IO_percentage, int quantum);
void Set_Limit_By_Input();

typedef struct History History;
History *Create_History(int time, int PID);
void Add_History(History *history, int time);

int recorded_max_time;
History *history_IO;
int isIO_Happen(int time);

int by_PID, by_priority, by_arrival_time, by_remain_burst_time, nonpreemtive, preemtive_RR, preemtive_SJF, preemtive_Priority;
Process *running;
Queue *process_queue, *job_queue, *ready_queue, *waiting_queue, *terminated_queue, *preready_queue;
Queue *result_FCFS, *result_SJF, *result_priority, *result_RR, *result_preemptive_SJF, *result_preemptive_priority;
History *history_FCFS, *history_SJF, *history_priority, *history_RR, *history_preemptive_SJF, *history_preemptive_priority;
void Config();
Queue *Save_And_Reset_Queue();

void Update_Process_Time();
void Update_Process_State(int time, int type, int sort_by);
void Update_Ready_Queue(int sort_by);
void Update_Running();

void Schedule(int type, int sort_by, History *history);
void FCFS();
void SJF();
void Priority();
void RR();
void Preemtive_SJF();
void Preemtive_Priority();

void Show();
void Show_Process();
void Show_Gantt_Chart();
void Show_Result();
void Print_Center(int content, int space);
void Print_Right(int content, int space);
History *Print_Time_And_PID(History *h, int front_blank, int *n);

typedef struct Process{
    int PID;                    // Process ID
    int CPU_burst_time;         // CPU burst time
    int IO_burst_time;          // IO burst time
    int arrival_time;           // Arrival time
    int priority;               // Priority
    int turnaround_time;        // Turnaround Time
    int waiting_time;           // Waiting Time
    int current_waiting_time;   // Waiting Time in Waiting Queue
    int burst_time;             // Burst Time;
    int current_burst_time;     // Burst Time in the state Running
    int completion_time;        // Completion Time
}Process;

Process *Create_Process(int PID, int CPU_burst_time, int IO_burst_time, int arrival_time, int priority){
    Process *process = (struct Process *)malloc(sizeof(struct Process));
    process->PID = PID;
    process->CPU_burst_time = CPU_burst_time;
    process->IO_burst_time = IO_burst_time;
    process->arrival_time = arrival_time;
    process->priority = priority;
    process->turnaround_time = 0;
    process->waiting_time = 0;
    process->current_waiting_time = 0;
    process->burst_time = 0;
    process->current_burst_time = 0;
    process->completion_time = 0;
    return process;
};

typedef struct Node{
    Process *process;
    struct Node *next;
}Node;

Node *Create_Node(){
    Node *node = (struct Node *)malloc(sizeof(struct Node));
    node->process = NULL;
    node->next = NULL;
    return node;
}

typedef struct Queue{
    int cnt_node;
    Node *head;
}Queue;

Queue *Create_Queue(){
    Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
    queue->cnt_node = 0;
    queue->head = Create_Node();
    queue->head->next = NULL;
    return queue;
}

void Push_Queue(Queue *queue, Process *process){
    Node *new_node = Create_Node();
    new_node->process = process;
    Node *node = queue->head;
    while(node->next != NULL){
        node = node->next;
    }
    node->next = new_node;
    queue->cnt_node++;
}

Process *Pop_Queue(Queue *queue){
    if(queue->cnt_node > 0){
        Node *node = queue->head->next;
        Process *process = node->process;
        queue->head->next = node->next;
        queue->cnt_node--;
        return process;
    }else{
        return NULL;
    }
}

void Empty_Queue(Queue *queue){
    int cnt = queue->cnt_node;
    for(int i=0; i<cnt; i++){
        Pop_Queue(queue);
    }
}

Process *Copy_Process(Process *process){
    Process *new_process = Create_Process(process->PID, process->CPU_burst_time, process->IO_burst_time, process->arrival_time, process->priority);
    new_process->turnaround_time = process->turnaround_time;
    new_process->waiting_time = process->waiting_time;
    new_process->current_waiting_time = process->current_waiting_time;
    new_process->burst_time = process->burst_time;
    new_process->current_burst_time = process->current_burst_time;
    new_process->completion_time = process->completion_time;
    return new_process;
}

Queue *Copy_Queue(Queue *queue){
    Queue *new_queue = Create_Queue();
    Node *node = queue->head;
    while(node->next != NULL){
        node = node->next;
        Push_Queue(new_queue, Copy_Process(node->process));
    }
    return new_queue;
}

void Sort_Queue(Queue *queue, int type){
    Node *node;
    Process *temp;
    if(type == by_PID){
        for(int i=0; i<queue->cnt_node; i++){
            node = queue->head;
            for(int j=0; j<queue->cnt_node - 1; j++){
                node = node->next;
                if(node->process->PID > node->next->process->PID){
                    temp = node->next->process;
                    node->next->process = node->process;
                    node->process = temp;
                }   
            }
        }
    }else if(type == by_priority){
        for(int i=0; i<queue->cnt_node; i++){
            node = queue->head;
            for(int j=0; j<queue->cnt_node - 1; j++){
                node = node->next;
                if(node->process->priority > node->next->process->priority){
                    temp = node->next->process;
                    node->next->process = node->process;
                    node->process = temp;
                }
            }
        }
    }else if(type == by_arrival_time){
        for(int i=0; i<queue->cnt_node; i++){
            node = queue->head;
            for(int j=0; j<queue->cnt_node - 1; j++){
                node = node->next;
                if(node->process->arrival_time > node->next->process->arrival_time){
                    temp = node->next->process;
                    node->next->process = node->process;
                    node->process = temp;
                }
            }
        }
    } else if(type == by_remain_burst_time){
        for(int i=0; i<queue->cnt_node; i++){
            node = queue->head;
            for(int j=0; j<queue->cnt_node - 1; j++){
                node = node->next;
                if(node->process->CPU_burst_time - node->process->burst_time > node->next->process->CPU_burst_time - node->next->process->burst_time){
                    temp = node->next->process;
                    node->next->process = node->process;
                    node->process = temp;
                }
            }
        }
    }
}

typedef struct Limit{
    int cnt_process;        // The number of Process
    int CPU_burst_time;     // Limit of the max CPU burst time
    int IO_burst_time;      // Limit of the max IO burst time
    int arrival_time;       // Limit of the max Arrival time
    int IO_percentage;      // Percentage of I/O
    int quantum;            // Time Quantum for RR
}Limit;

void Set_Limit(int cnt_process, int CPU_burst_time, int IO_burst_time, int arrival_time, int IO_percentage, int quantum){
    limit = (struct Limit *)malloc(sizeof(struct Limit));
    limit->cnt_process = cnt_process;
    limit->CPU_burst_time = CPU_burst_time;
    limit->IO_burst_time = IO_burst_time;
    limit->arrival_time = arrival_time;
    limit->IO_percentage = IO_percentage;
    limit->quantum = quantum;
};

void Set_Limit_By_Input(){
    limit = (struct Limit *)malloc(sizeof(struct Limit));
    int value;
    printf(" How many processes : ");
    scanf("%d", &value);
    limit->cnt_process = value;
    printf(" Limit of CPU burst time : ");
    scanf("%d", &value);
    limit->CPU_burst_time = value;
    printf(" Limit of I/O burst time : ");
    scanf("%d", &value);
    limit->IO_burst_time = value;
    printf(" Limit of arrival time : ");
    scanf("%d", &value);
    limit->arrival_time = value;
    printf(" Percent of I/O: ");
    scanf("%d", &value);
    limit->IO_percentage = value;
    printf(" Quantum of RR(Round Robin) : ");
    scanf("%d", &value);
    limit->quantum = value;
}

typedef struct History{
    int time;               // Start time of Process Running
    int PID;                // Process ID
    struct History *next;   // Address of next History
}History;

History *Create_History(int time, int PID){
    History *new_history = (struct History *)malloc(sizeof(struct History));
    new_history->time = time;
    new_history->PID = PID;
    new_history->next = NULL;
    return new_history;
}

void Add_History(History *history, int time){
    int PID;
    if(running == NULL){
        PID = 0;
    }else{
        PID = running->PID;
    }
    History *new_history = Create_History(time, PID);

    History *ptr_history = history;
    while(ptr_history->next != NULL){
        ptr_history = ptr_history->next;
    }
    ptr_history->next = new_history;
}

int isIO_Happen(int time){
    if(time > recorded_max_time){
        recorded_max_time = time;
        int random_number = rand()%100 + 1;
        if(random_number <= limit->IO_percentage){
            Add_History(history_IO, time);
            return 1;
        } else {
            return 0;
        }
    }else{
        History *ptr_history = history_IO;
        while(ptr_history->next != NULL){
            ptr_history = ptr_history->next;
            if(ptr_history->time == time){
                return 1;
            }
        }
        return 0;
    }
}

void Config(){
    by_PID = 0;
    by_priority = 1;
    by_arrival_time = 2;
    by_remain_burst_time = 3;

    nonpreemtive = 0;
    preemtive_RR = 1;
    preemtive_SJF = 2;
    preemtive_Priority = 3;

    history_FCFS = Create_History(0, 0);
    history_SJF = Create_History(0, 0);
    history_priority = Create_History(0, 0);
    history_RR = Create_History(0, 0);
    history_preemptive_SJF = Create_History(0, 0);
    history_preemptive_priority = Create_History(0, 0);

    recorded_max_time = 0;
    history_IO = Create_History(0, 0);

    process_queue = Create_Queue();
    for(int i=0; i<limit->cnt_process; i++){
        Push_Queue(process_queue, Create_Process(i+1, rand()%limit->CPU_burst_time + 1, rand()%limit->IO_burst_time + 1, rand()%(limit->arrival_time + 1), rand()%limit->cnt_process + 1));
    }
    job_queue = Copy_Queue(process_queue);
    ready_queue = Create_Queue();
    waiting_queue = Create_Queue();
    terminated_queue = Create_Queue();
    preready_queue = Create_Queue();
}

Queue *Save_And_Reset_Queue(){
    Queue *queue = Copy_Queue(terminated_queue);
    Sort_Queue(queue, by_PID);
    Empty_Queue(terminated_queue);
    job_queue = Copy_Queue(process_queue);
    return queue;
}

void Update_Process_Time(){
    Node *ptr_node;
    Process *ptr_process;

    // add 1 sec to burst time if there is a running process
    if(running != NULL){
        running->current_burst_time++;
    }
    // add 1 sec to waiting time if there is a process in ready queue
    ptr_node = ready_queue->head;
    for(int i=0; i<ready_queue->cnt_node; i++){
        ptr_node = ptr_node->next;
        ptr_node->process->waiting_time++;
    }
    // add 1 sec to waiting time if there is a process in waiting queue
    ptr_node = waiting_queue->head;
    for(int i=0; i<waiting_queue->cnt_node; i++){
        ptr_node = ptr_node->next;
        ptr_node->process->current_waiting_time++;
    }
}

void Update_Process_State(int time, int type, int sort_by){
    // if running process finish running, push that process to terminated queue
    if((running != NULL) && (running->burst_time + running->current_burst_time == running->CPU_burst_time)){
        running->burst_time += running->current_burst_time;
        running->current_burst_time = 0;
        running->completion_time = time;
        running->turnaround_time = running->completion_time - running->arrival_time;
        Push_Queue(terminated_queue, running);
        running = NULL;
    }

    if((running != NULL) && (isIO_Happen(time))){
        // if there is a running process and I/O is happened, then push running process to waiting queue
        running->burst_time += running->current_burst_time;
        running->current_burst_time = 0;
        Push_Queue(waiting_queue, running);
        running = NULL;
    }

    if((running != NULL) && (((type == preemtive_RR) && (running->current_burst_time == limit->quantum)) 
    || (type == preemtive_SJF) || (type == preemtive_Priority))){
        running->burst_time += running->current_burst_time;
        running->current_burst_time = 0;
        Push_Queue(preready_queue, running);
        running = NULL;
    }

    Node *ptr_node;
    int cnt;

    // if some processes arrive, then copy them to preready queue
    ptr_node = job_queue->head;
    while(ptr_node->next!=NULL){
        if(ptr_node->next->process->arrival_time == time){
            Push_Queue(preready_queue, ptr_node->next->process);
            ptr_node->next = ptr_node->next->next;
            job_queue->cnt_node -= 1;
        }else{
            ptr_node = ptr_node->next;
        }
    }

    // if some processes finish waiting, then push them to preready queue
    ptr_node = waiting_queue->head;
    while(ptr_node->next!=NULL){
        if(ptr_node->next->process->current_waiting_time == ptr_node->next->process->IO_burst_time){
            ptr_node->next->process->waiting_time += ptr_node->next->process->current_waiting_time;
            ptr_node->next->process->current_waiting_time = 0;
            Push_Queue(preready_queue, ptr_node->next->process);
            ptr_node->next = ptr_node->next->next;
            waiting_queue->cnt_node -= 1;
        }else{
            ptr_node = ptr_node->next;
        }
    }
}

void Update_Ready_Queue(int sort_by){
    // After gathering processes(which arrive or finish waiting) in sort process, Sort them by priority and Push to ready queue
    if(sort_by == by_priority){
        while(preready_queue->cnt_node > 0){
            Push_Queue(ready_queue, Pop_Queue(preready_queue));
        }
        Sort_Queue(ready_queue, by_arrival_time);
        Sort_Queue(ready_queue, by_priority);
    }else if(sort_by == by_arrival_time){
        Sort_Queue(preready_queue, by_arrival_time);
        while(preready_queue->cnt_node > 0){
            Push_Queue(ready_queue, Pop_Queue(preready_queue));
        }
    }else if(sort_by == by_remain_burst_time){
        while(preready_queue->cnt_node > 0){
            Push_Queue(ready_queue, Pop_Queue(preready_queue));
        }
        Sort_Queue(ready_queue, by_arrival_time);
        Sort_Queue(ready_queue, by_remain_burst_time);
    }
}

void Update_Running(){
    // if there is no running process and there is a process in ready queue, then dispatch that process
    if((running == NULL) && (ready_queue->cnt_node != 0)){
        running = Pop_Queue(ready_queue);
    }
}

void Schedule(int type, int sort_by, History *history){
    int time = -1;
    running = NULL;
    Node *ptr_node;
    Process *ptr_process;
    while(terminated_queue->cnt_node != process_queue->cnt_node){
        time++;
        Update_Process_Time();      // running, ready_queue, waiting_queue의 time ++
        Update_Process_State(time, type, sort_by);
        Update_Ready_Queue(sort_by);
        Update_Running();
        Add_History(history, time);
    }
}

void FCFS(){
    Schedule(nonpreemtive, by_arrival_time, history_FCFS);
    result_FCFS = Save_And_Reset_Queue();
}

void SJF(){
    Schedule(nonpreemtive, by_remain_burst_time, history_SJF);
    result_SJF = Save_And_Reset_Queue();
}

void Priority(){
    Schedule(nonpreemtive, by_priority, history_priority);
    result_priority = Save_And_Reset_Queue();
}

void RR(){
    Schedule(preemtive_RR, by_arrival_time, history_RR);
    result_RR = Save_And_Reset_Queue();
}

void Preemtive_SJF(){
    Schedule(preemtive_SJF, by_remain_burst_time, history_preemptive_SJF);
    result_preemptive_SJF = Save_And_Reset_Queue();
}

void Preemtive_Priority(){
    Schedule(preemtive_Priority, by_priority, history_preemptive_priority);
    result_preemptive_priority = Save_And_Reset_Queue();
}

void Show(){
    printf("\n\n[1] Process Information\n");
    Show_Process();

    printf("\n\n[2] Gantt Chart\n");
    Show_Gantt_Chart();

    printf("\n\n[3] Evaluation\n");
    Show_Result();
}

void Show_Process(){
    Node *node = process_queue->head;
    Process *p;
    for(int i=0; i<process_queue->cnt_node; i++){
        node = node->next;
        p = node->process;
        printf(" PID : %d, CPU_burst : %d, IO_burst = %d, arrvial : %d, priority : %d\n", p->PID, p->CPU_burst_time, p->IO_burst_time, p->arrival_time, p->priority);
    }
}

void Show_Gantt_Chart(){
    int n = 6;
    History *h1 = history_FCFS;
    History *h2 = history_SJF;
    History *h3 = history_priority;
    History *h4 = history_RR;
    History *h5 = history_preemptive_SJF;
    History *h6 = history_preemptive_priority;

    History *h_io = history_IO;
    if(h_io->next != NULL){
        h_io = h_io->next;
    }
    printf("\n * I/O :");
    while(h_io->next != NULL){
        printf(" %d,", h_io->time);
        h_io = h_io->next;
    }
    printf(" %d\n\n", h_io->time);

    printf(" <1> FCFS      <2> SJF       <3> Priority     <4> RR        <5> Preemptive SJF     <6> Preemptive Priority\n");
    printf(" Time | PID   Time | PID      Time | PID     Time | PID          Time | PID                Time | PID\n");
    while(n > 0){
        for(int i=0; i<6; i++){
            switch(i){
                case 0:
                    h1 = Print_Time_And_PID(h1, 1, &n);
                    break;
                case 1:
                    h2 = Print_Time_And_PID(h2, 3, &n);
                    break;
                case 2:
                    h3 = Print_Time_And_PID(h3, 6, &n);
                    break;
                case 3:
                    h4 = Print_Time_And_PID(h4, 5, &n);
                    break;
                case 4:
                    h5 = Print_Time_And_PID(h5, 10, &n);
                    break;
                case 5:
                    h6 = Print_Time_And_PID(h6, 16, &n);
                    printf("\n");
                    break;
                default:
                    break;
            }

        }
    }
}

void Show_Result(){
    char schedule_type[6][20] = {"FCFS", "SJF", "Priority", "RR", "Preemptive SJF", "Preemptive Priority"};
    Queue *queue;
    Node *node;
    char avg_turnaround_time[6][7], avg_waiting_time[6][7];
    int sum_turnaround_time, sum_waiting_time;
    for(int i=0; i<6; i++){
        switch(i){
            case 0:
                queue = result_FCFS;
                break;
            case 1:
                queue = result_SJF;
                break;
            case 2:
                queue = result_priority;
                break;
            case 3:
                queue = result_RR;
                break;
            case 4:
                queue = result_preemptive_SJF;
                break;
            case 5:
                queue = result_preemptive_priority;
                break;
            default:
                break;
        }
        sum_turnaround_time = 0;
        sum_waiting_time = 0;
        printf("\n < %s >\n", schedule_type[i]);
        printf("   PID | arrival time | completion time | turnaround time | burst time | waiting time \n");
        printf(" -------------------------------------------------------------------------------------\n");
        node = queue->head;
        while(node->next != NULL){
            node = node->next;
            sum_turnaround_time += node->process->turnaround_time;
            sum_waiting_time += node->process->waiting_time;
            printf("  ");
            Print_Center(node->process->PID ,5);
            printf("|     ");
            Print_Center(node->process->arrival_time, 5);
            printf("    |      ");
            Print_Center(node->process->completion_time, 5);
            printf("      |      ");
            Print_Center(node->process->turnaround_time, 5);
            printf("      |    ");
            Print_Center(node->process->burst_time, 5);
            printf("   |     ");
            Print_Center(node->process->waiting_time, 5);
            printf("    \n");
        }
        sprintf(avg_turnaround_time[i], "%.2f", (double)(sum_turnaround_time) / (double)(queue->cnt_node));
        sprintf(avg_waiting_time[i], "%.2f", (double)(sum_waiting_time) / (double)(queue->cnt_node));
    }
    printf("\n < Evaluation >\n");
    printf("                      | Average waiting time | Average turnaround time\n");
    printf(" ----------------------------------------------------------------------\n");
    printf("  FCFS                |         %s         |          %s\n", avg_waiting_time[0], avg_turnaround_time[0]);
    printf("  SJF                 |         %s         |          %s\n", avg_waiting_time[1], avg_turnaround_time[1]);
    printf("  Priority            |         %s         |          %s\n", avg_waiting_time[2], avg_turnaround_time[2]);
    printf("  RR                  |         %s         |          %s\n", avg_waiting_time[3], avg_turnaround_time[3]);
    printf("  preemptive SJF      |         %s         |          %s\n", avg_waiting_time[4], avg_turnaround_time[4]);
    printf("  preemptive Priority |         %s         |          %s\n", avg_waiting_time[5], avg_turnaround_time[5]);
}

void Print_Center(int content, int space){
    int len = 1;
    if(content != 0){
        len = (int)(log10(content)) + 1;
    }
    int front = (space - len) / 2;
    char *str_content = (char *)malloc(sizeof(char) * (len + 1));
    sprintf(str_content, "%d", content);
    static char str[20] = {0};
    for(int i=0; i<space; i++){
        str[i] = ' ';
    }
    for(int i=front; i<front+len; i++){
        str[i] = str_content[i-front];
    }
    printf("%s", str);
}

void Print_Right(int content, int space){
    int len = 1;
    if(content != 0){
        len = (int)(log10(content)) + 1;
    }
    for(int i=0; i<space-len; i++){
        printf(" ");
    }
    printf("%d", content);
}

History *Print_Time_And_PID(History *h, int front_blank, int *n){
    if(h == NULL){
        for(int i=0; i<front_blank + 10; i++){
            printf(" ");
        }
    }else if((h->next != NULL) && (h->next->time == 0) && (h->next->PID == 0)){
        for(int i=0; i<front_blank; i++){
            printf(" ");
        }
        printf("   0 |   X");
        h = h->next;
    }else if((h->next != NULL) && (h->PID != h->next->PID)){
        for(int i=0; i<front_blank; i++){
            printf(" ");
        }
        Print_Right(h->next->time, 4);
        printf(" | ");
        if(h->next->PID == 0){
            printf("  X");
        }else{
            Print_Right(h->next->PID, 3);
        }
        h = h->next;
    }else if(h->next != NULL){
        for(int i=0; i<front_blank+5; i++){
            printf(" ");
        }
        printf("|    ");
        h = h->next;
    }else{
        (*n)--;
        h = h->next;
        for(int i=0; i<front_blank + 10; i++){
            printf(" ");
        }
    }
    return h;
}

int main(void){
    srand(100);
    //Set_Limit(5, 10, 10, 10, 10, 3);
    Set_Limit_By_Input();
    Config();

    FCFS();
    SJF();
    Priority();
    RR();
    Preemtive_SJF();
    Preemtive_Priority();

    Show();
    return 0;
}
