#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#define ERRCODE 1

#define logerr(msg) do { fprintf(stderr, "Error: line %d: %s: "msg"\n", __LINE__, __func__); } while(0);

#define CRITICAL_SECTION_LOCK sem_wait(sem.write)
#define CRITICAL_SECTION_UNLOCK sem_post(sem.write)

//Sleep random amount of milliseconds in range <0, amount>
#define sleepmill(amount) (usleep((rand() % (amount+1))*1000));

//Allocates memory that will be shared between processes.
int shmem(void** ptr)
{
    void* tmp = mmap(NULL, sizeof(*ptr), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (tmp == MAP_FAILED)
    {
        perror("mmap failed");
        return 0;
    }
    *ptr = tmp;
    return 1;
}

//Frees shared memory.
int shunmap(void* ptr)
{
    if ((munmap(ptr, sizeof(ptr))) == -1)
    {
        perror("munmap failed");
        return 0;
    }
    return 1;
}

//Structure type that hold all the shared variables.
typedef struct
{
    int log_index;
    int mol_index;
    int create_q;
    int totalO;
    int totalH;
} shinfo_t;

//Structure type that holds all semaphores.
typedef struct{
    sem_t* write;
    sem_t* oxy_q;
    sem_t* hydro_q;
    sem_t* create;
    sem_t* final;
} semaphores;

semaphores sem = {NULL,};
shinfo_t* info = NULL;
FILE* filep = NULL;

//Initialize all semaphores.
int open_semaphores()
{
    //Try to unlink all semaphores in case previous invokation
    //ended badly and they are still linked.
    sem_unlink("writing");
    sem_unlink("oxy_queue_sem");
    sem_unlink("hydro_queue_sem");
    sem_unlink("creation_sem");
    sem_unlink("final_sem");

//Open all semaphores and set their initial values according to their role.

    //This semaphore is always used to access a critical section to avoid race condition.
    //It shouldn't be used for any other purpose. Unlocked by default.
    //Initial value: 1.
    if ((sem.write = sem_open("writing", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
        perror("sem_open failed");
        return 0;
    }
    
    //This semaphore is used to simulate the queue of oxygen.
    //Unlocked by default but then is only unlocked after previous "1 oxygen and 2 hydrogens"
    //have fully created their molecule.
    //Initial value: 1.
    if ((sem.oxy_q = sem_open("oxy_queue_sem", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
        perror("sem_open failed");
        return 0;
    }
    
    //This semaphore is used to simulate the queue of hydrogen.
    //Unlocked by default but then is only unlocked after previous "1 oxygen and 2 hydrogens"
    //have fully created their molecule.
    //Initial value: 2.
    if ((sem.hydro_q = sem_open("hydro_queue_sem", O_CREAT | O_EXCL, 0666, 2)) == SEM_FAILED)
    {
        perror("sem_open failed");
        return 0;
    }
    
    //This semaphore is used to indicate the start of molecule creation.
    //Locked by default and is only unlocked after "1 oxygen and 2 hydrogens"
    //have printed their "...creating a molecule..." message.
    //Initial value: 0.
    if ((sem.create = sem_open("creation_sem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        perror("sem_open failed");
        return 0;
    }
    
    //This semaphore is used to indicate the end of molecule creation.
    //Locked by default and is only unlocked after "1 oxygen and 2 hydrogens"
    //have printed their "...molecule created..." message.
    //Initial value: 0.
    if ((sem.final = sem_open("final_sem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
        perror("sem_open failed");
        return 0;
    }

    return 1;
}

//Close and unlink all semaphores.
void close_semaphores()
{
    sem_close(sem.write);
    sem_close(sem.oxy_q);
    sem_close(sem.hydro_q);
    sem_close(sem.create);
    sem_close(sem.final);
   
    sem_unlink("writing");
    sem_unlink("oxy_queue_sem");
    sem_unlink("hydro_queue_sem");
    sem_unlink("creation_sem");
    sem_unlink("final_sem");
}

//Open semaphores, allocate and initialize memory, open "proj2.out" file, set random seed.
int init()
{    
    open_semaphores();

    if ((filep = fopen("proj2.out", "w")) == NULL)
    {
        perror("fopen failed");
        return 0;
    }
        
    setlinebuf(filep);

    if (!shmem((void**)(&info)))
        return 0;

    info->log_index = 0;
    info->mol_index = 1;
    info->create_q = 0;
    info->totalH = 0;
    info->totalO = 0;
    
    srand(time(0));
    return 1;
}

//Unmap all shared memory, close semaphores, close file descriptor.
void cleanup()
{
    shunmap(info);

    close_semaphores();

    if (filep != NULL)
        fclose(filep);
}

//Get current value of a semophore for debug purposes.
int semgetv(sem_t* sem)
{
    int sval = 0;
    if (sem_getvalue(sem, &sval) == -1)
    {
        perror("se_getvalue failed");
        exit(1);
    }

    return sval;
}

//Print out a "...starting..." message.
void log_start(const int id, const char who)
{
    assert(who == 'O' || who == 'H');
    CRITICAL_SECTION_LOCK;

        info->log_index++;
        fprintf(filep, "%d: %c %d: started\n", info->log_index, who, id);
        
    CRITICAL_SECTION_UNLOCK;
}

//Print out a "...going to a queue..." message.
void enter_queue(const int id, const char who)
{
    assert(who == 'O' || who == 'H');
    CRITICAL_SECTION_LOCK;

        info->log_index++;
        fprintf(filep, "%d: %c %d: going to queue\n", info->log_index, who, id);
    
    CRITICAL_SECTION_UNLOCK;
}

//Check if there is enough oxygen and hydrogen to create a molecule.
//If there isn't, print out a "...not enough..." message and end the process.
void check_amount(const int id, const char who)
{
    assert(who == 'O' || who == 'H');
    CRITICAL_SECTION_LOCK;
        
        if (info->totalH < 2 || info->totalO < 1)
        {
            info->log_index++;
            
            if (who == 'O')
                fprintf(filep, "%d: O %d: not enough H\n", info->log_index, id);
            else if (who == 'H')
                fprintf(filep, "%d: H %d: not enough O or H\n", info->log_index, id);
            
            who == 'O' ? info->totalO-- : info->totalH--;
            CRITICAL_SECTION_UNLOCK;
            exit(0);
        }
            
    CRITICAL_SECTION_UNLOCK;
}

//Print out a "...creating a molecule..." message.
//If that message was already printed by other 2 atoms,
//open the creation semaphore.
void create_molecule(const int id, const char who)
{
    assert(who == 'O' || who == 'H');
    CRITICAL_SECTION_LOCK;

        info->log_index++;

        fprintf(filep, "%d: %c %d: creating molecule %d\n", info->log_index, who, id, info->mol_index);
        
        info->create_q++;
        if (info->create_q >= 3)
        {
            sem_post(sem.create);
            sem_post(sem.create);
            sem_post(sem.create);
        }

    CRITICAL_SECTION_UNLOCK;
}

//Print out a "...molecule 'noM' created..." message.
//If that message was already printed by other 2 atoms,
//open the final semaphore.
void molecule_created(const int id, const char who)
{
    assert(who == 'O' || who == 'H');
    CRITICAL_SECTION_LOCK;
        
        info->log_index++;
        fprintf(filep, "%d: %c %d: molecule %d created\n", info->log_index, who, id, info->mol_index);
        
        info->create_q--;
        if (info->create_q <= 0)
        {
            sem_post(sem.final);
            sem_post(sem.final);
            sem_post(sem.final);  
        }

    CRITICAL_SECTION_UNLOCK;
}

//If other 2 atoms have also reached this stage,
//open the oxygen_queue and 2 hydrogen_queue semaphores.
//If there is not enough oxygen or hydrogen for the next molecule,
//open queue semaphores for all the remaining atoms,
//for them to end their processes with a corresponding message.
void finalize(const char who)
{
    assert(who == 'O' || who == 'H');
    CRITICAL_SECTION_LOCK;

        if (who == 'O') info->mol_index++;

        who == 'O' ? info->totalO-- : info->totalH--;
        info->create_q++;

        if (info->create_q >= 3)
        {
            if (info->totalO >= 1 && info->totalH >= 2)
            {
                sem_post(sem.oxy_q);
                sem_post(sem.hydro_q);
                sem_post(sem.hydro_q);
            }
            else if (info->totalO < 1 || info->totalH < 2)
            {
                for (int i = 0; i < info->totalO; i++)
                    sem_post(sem.oxy_q);
                for (int i = 0; i < info->totalH; i++)
                    sem_post(sem.hydro_q);
            }
            info->create_q -= 3;
        }
        
    CRITICAL_SECTION_UNLOCK;
}

//Function that will be run on a child process, representing a hydrogen atom.
void process_hydrogen(const int id, int TI)
{
    log_start(id, 'H');
    
    sleepmill(TI);

    enter_queue(id, 'H');

    check_amount(id, 'H');

    sem_wait(sem.hydro_q);
    
    check_amount(id, 'H');

    create_molecule(id, 'H');

    sem_wait(sem.create);

    molecule_created(id, 'H');

    sem_wait(sem.final);

    finalize('H');

    exit(0);
}

//Function that will be run on a child process, representing a oxygen atom.
void process_oxygen(const int id, int TI, int TB)
{
    log_start(id, 'O');

    sleepmill(TI);

    enter_queue(id, 'O');

    check_amount(id, 'O');
    
    sem_wait(sem.oxy_q);

    check_amount(id, 'O');

    create_molecule(id, 'O');

    sleepmill(TB);

    sem_wait(sem.create);

    molecule_created(id, 'O');

    sem_wait(sem.final);

    finalize('O');
     
    exit(0);
}

//Function responsible for argument parsing.
int getargs(int argc, char** argv, int (*args_int)[4])
{
    if (argc != 5)
    {
        logerr("Wrong number of arguments.");
        return 0;
    }
    
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '\0')
        {
            logerr("Invalid argument.");
            return 0;
        }
        char* endp = NULL;
        long tmp = strtol(argv[i], &endp, 10);
        if (endp[0] != '\0')
        {
            logerr("Invalid argument.");
            return 0;
        }
        if (tmp < 0)
        {
            logerr("Invalid argument.");
            return 0;
        }

        int this = i-1;
        (*args_int)[this] = (int)tmp;

        if (this == 2 || this == 3)
        {
            if ((*args_int)[this] < 0 || (*args_int)[this] > 1000)
            {
                logerr("Argument out of range(0<=...<=1000).");
                return 0;
            }
        }
    }

    return 1;
}

//MAIN FUNCTION
int main(int argc, char** argv)
{
    int args_int[4] = {0,}; //NO NH TI TB.
    if ( !getargs(argc, argv, &args_int) )
        return ERRCODE;

    if (!init())
    {
        cleanup();
        return ERRCODE;
    }
    
    info->totalO = args_int[0];
    info->totalH = args_int[1];

    //Create all the oxygen.
    for (int i = 0; i < args_int[0]; i++)
    {
        pid_t pid = fork();
        if ( pid == 0 )
        {
            //Child process.
            srand(getpid() * time(0));
            process_oxygen(i, args_int[2], args_int[3]);
        }
        
    }

    //Create all the hydrogen.
    for (int i = 0; i < args_int[1]; i++)
    {
        pid_t pid = fork();
        if ( pid == 0 )
        {
            //Child process.
            srand(getpid() * time(0));
            process_hydrogen(i, args_int[2]);
        }
        
    }
    //Wait for all child processes to end.
    //wait(NULL);
    while(wait(NULL) > 0);
    //Cleanup all the allocated resources.
    cleanup();
    return 0;
}