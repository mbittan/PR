#include <stack.h>

void Init_stack(){
  key_t key_shm = ftok("/tmp",42);
  key_t key_sem = ftok("/tmp",43);
  union semun semunion;

  //Creation de la memoire partagee
  if((shmid=shmget(key_shm,sizeof(stack), 0666 | IPC_CREAT))==-1){
    perror("shmget");
    exit(EXIT_FAILURE);
  }
  if((t = (stack*)shmat(shmid,NULL, 0666)) == (void*)-1){
    perror("shmat");
    exit(EXIT_FAILURE);
  }

  t->stack_size=0;

  //Creation des semaphores
  //0 : Mutex
  //1 : Producteur
  //2 : Consommateur
  if((semid = semget(key_sem,3,0666 | IPC_CREAT))==-1){
    perror("semget");
    exit(EXIT_FAILURE);
  }

  //On met le mutex a 1
  semunion.val=1;
  if(semctl(semid,0,SETVAL,semunion)==-1){
    perror("semctl");
    exit(EXIT_FAILURE);
  } 

  //La semaphore des producteur a MAXSIZE
  semunion.val=MAXSIZE;
  if(semctl(semid,1,SETVAL,semunion)==-1){
    perror("semctl");
    exit(EXIT_FAILURE);
  }

  //Et celle des consommateurs a 0
  semunion.val=0;
  if(semctl(semid,2,SETVAL,semunion)==-1){
    perror("semctl");
    exit(EXIT_FAILURE);
  }
}


void Push(char c){
  struct sembuf bufsem[2];
  //On prend le producteur
  bufsem[1].sem_num =  1;
  bufsem[1].sem_op  = -1;
  bufsem[1].sem_flg =  0;

  //Et le mutex
  bufsem[0].sem_num =  0;
  bufsem[0].sem_op  = -1;
  bufsem[0].sem_flg =  0;
  semop(semid, bufsem, 2);

  t->stack[t->stack_size] = c;
  t->stack_size++;

  //On relache mutex et consommateur
  bufsem[0].sem_num =  0;
  bufsem[0].sem_op  =  1;
  bufsem[0].sem_flg =  0;
  bufsem[1].sem_num =  2;
  bufsem[1].sem_op  =  1;
  bufsem[1].sem_flg =  0;
  semop(semid, bufsem, 2);
}


char Pop(){
  char tmp;
  struct sembuf bufsem[3];
 //On prend le mutex
  bufsem[0].sem_num =  0;
  bufsem[0].sem_op  = -1;
  bufsem[0].sem_flg =  0;

  //Et consommateur
  bufsem[1].sem_num =  2;
  bufsem[1].sem_op  = -1;
  bufsem[1].sem_flg =  0;

  semop(semid, bufsem, 2);
  tmp = t->stack[--t->stack_size];

 //On relache mutex et producteur
  bufsem[0].sem_num =  0;
  bufsem[0].sem_op  =  1;
  bufsem[0].sem_flg =  0;
  bufsem[1].sem_num =  1;
  bufsem[1].sem_op  =  1;
  bufsem[1].sem_flg =  0;
  semop(semid, bufsem, 2);
  
  return tmp;
}

void Print_stack() {
  int i;
  printf("STACK[%d] = |", getpid());
  for(i=0; i<t->stack_size; i++)
    printf("%c|", t->stack[i]);
  printf(">\n");
}
