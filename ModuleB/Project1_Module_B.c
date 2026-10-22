#include "Project1_Module_B.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_processes>\n", argv[0]);
        return 1;
    }

    int num_processes = atoi(argv[1]);
    if (num_processes <= 0) {
        printf("Invalid number of processes: %d\n", num_processes);
        return 1;
    }


    struct timespec start, end;
    long long seconds, nano_seconds;


    /* ToDo1
     * Create and attach shared memory region 1 to store the matrix
     * 
     * Hint: Google shmget() and shmat()
     */
    size_t matrix_bytes = (size_t)ROWS_COUNT * COLUMN_COUNT * sizeof(int);
    int shmid_matrix = shmget(IPC_PRIVATE, matrix_bytes, IPC_CREAT | 0666);
    if (shmid_matrix < 0) {
        perror("shmget matrix");
        return 1;
    }

    int *matrix = (int *)shmat(shmid_matrix, NULL, 0);
    if (matrix == (int *)-1) {
        perror("shmat matrix");
        return 1;
    }

    /* ToDo2
     * create and attach shared memory region 2 to store the partial sums that
     * are returned by children processes.
     */
    size_t sums_bytes = (size_t)num_processes * sizeof(long);
    int shmid_sums = shmget(IPC_PRIVATE, sums_bytes, IPC_CREAT | 0666);
    if (shmid_sums < 0) {
        perror("shmget sums");
        return 1;
    }

    long *partial_sums = (long *)shmat(shmid_sums, NULL, 0);
    if (partial_sums == (long *)-1) {
        perror("shmat sums");
        return 1;
    }

    /* ToDo3
     * Initialize the shared memory region 1 to hold
     * the matrix of size ROW_COUNT x COLUMN_COUNT.
     * Value of each element is set to i+j, where i is index of row
     * and j is index of column.
     * 
     * Hint: Treat starting address of shared memory region as first
     * elemnt of a 2D matrix i.e.,  matrix[0][0]
    */
    for (int i = 0; i < ROWS_COUNT; i++) {
        int *row = matrix + (size_t)i * COLUMN_COUNT;
        for (int j = 0; j < COLUMN_COUNT; j++) {
            row[j] = i + j;
        }
    }

    clock_gettime(CLOCK_REALTIME, &start);


    /* ToDo4:
     * 1) Create num_processes count of children processes. (using fork). Note that each child process runs a new binary 
     * (i.e., binary of child_proc.c) using execlp()
     * 2) Specify for each process the start_row, end_row, and
     * index in shared memory region 2 where a child process is supposed to store its
     * calculated partial sum.
     *
     *
     * Hints:
     *
     * Google "man fork" and "man execlp"
     *
    */
    int rows_per_proc = ROWS_COUNT / num_processes;

    for (int p = 0; p < num_processes; p++) {
        int start_row = p * rows_per_proc;
        int end_row   = (p == num_processes - 1)
                        ? (ROWS_COUNT - 1)              /* last one mops up remainder */
                        : (start_row + rows_per_proc - 1);

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            char a_start[16], a_end[16], a_shm1[16], a_shm2[16], a_idx[16];
            snprintf(a_start, sizeof(a_start), "%d", start_row);
            snprintf(a_end,   sizeof(a_end),   "%d", end_row);
            snprintf(a_shm1,  sizeof(a_shm1),  "%d", shmid_matrix);
            snprintf(a_shm2,  sizeof(a_shm2),  "%d", shmid_sums);
            snprintf(a_idx,   sizeof(a_idx),   "%d", p);

            execlp("./child", "child", a_start, a_end, a_shm1, a_shm2, a_idx, (char *)NULL);
            perror("execlp");   /* only reached if exec failed */
            _exit(1);
        }
    }

    /* ToDo5
     * Wait for all children processes to complete
    */
    for (int p = 0; p < num_processes; p++) {
        if (wait(NULL) < 0) {
            perror("wait");
            break;
        }
    }

    /* ToDo6:
     * Sum all the partial-sums stored in sharred memory region 2 (by children processes)
     * and print the sum.
    */
    long total_sum = 0;
    for (int p = 0; p < num_processes; p++) {
        total_sum += partial_sums[p];
    }
    printf("Total sum of matrix elements: %ld\n", total_sum);

    clock_gettime(CLOCK_REALTIME, &end);
    // Calculate execution time
    seconds=end.tv_sec - start.tv_sec;
    nano_seconds=end.tv_nsec - start.tv_nsec;
    long double executio_time=(long double)seconds+((long double)nano_seconds/1000000000);
    printf("Execution Time with %d process(es): %Lf\n",num_processes,executio_time);

    /* ToDo7:
     * Detach and destroy both shared memory regions.
     * Hint:
     * Google "man shmdt" and "man shmctl"
    */
    if (shmdt(matrix) < 0)       perror("shmdt matrix");
    if (shmdt(partial_sums) < 0) perror("shmdt sums");
    if (shmctl(shmid_matrix, IPC_RMID, NULL) < 0) perror("shmctl matrix");
    if (shmctl(shmid_sums,   IPC_RMID, NULL) < 0) perror("shmctl sums");

    return 0;
}
