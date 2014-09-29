#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <time.h>

#define N 3

void handler(int sig) {
  printf("%d : j'ai reçu ", getpid());
  switch(sig){
  case SIGUSR1:
    printf("SIGUSR1 !\n");
    break;
  case SIGCHLD:
    printf("SIGCHLD !\n");
    break;
  }
}

int main(int argc, char* argv[]){
  pid_t pids[N+1], son;
  int i;
  sigset_t mask, maskusr, maskchld;
  struct sigaction act;

  sigemptyset(&mask);
  sigfillset(&maskusr);
  sigfillset(&maskchld);
  sigdelset(&maskusr, SIGUSR1);
  sigdelset(&maskchld, SIGCHLD);
  sigaddset(&mask, SIGCHLD);
  sigaddset(&mask, SIGCONT);
  sigaddset(&mask, SIGUSR1);
  sigprocmask(SIG_SETMASK, &mask, NULL);

  act.sa_handler=handler;
  sigaction(SIGCHLD, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);

  pids[0] = getpid();
	
  for(i=1; i<= N; i++){
    if((son = fork()) != 0)
      break;
    pids[i] = getpid();
  }
  if(i == N+1){
    //si le processus courant est le dernier de la chaine
    //on affiche les pids
    for(i=0; i<N; i++)
      printf("%d ; ", pids[i]);
    printf("\n");
    //On s'envoi en SIGSTOP
    kill(getpid(), SIGSTOP);

    //on reprend l'execution (on a recu un SIGCONT)
    //On attends que notre pere nous envoie un SIGUSR1 qui indique que l'on
    //peut se terminer
    sigsuspend(&maskusr);
    exit(EXIT_SUCCESS);
  }
  else {
    //On attend dans tout les cas un premier SIGCHLD
    sigsuspend(&maskchld);
    printf("Me : %d ; Father : %d ; Son : %d\n", pids[i-1], getppid(), son);
    if(i==1){
      //Si on est dans le processus principal, cela veut dire que tout 
      //les processus se sont stoppé
      printf("Tous les processus sont arretes.\n");

      //On donne l'ordre au fils de continuer son execution
      kill(son, SIGCONT);

      //On recupere le SIGCHLD qui provient du fait que le fils a change 
      //d'etat.
      sigsuspend(&maskchld);

      //On indique au fils qu'il peut se terminer en lui envoyant un SIGUSR1
      kill(son, SIGUSR1);

      //On attends un SIGCHLD qui nous indique que celui ci s'est terminé
      sigsuspend(&maskchld);
      //on peut ensuite quitter le programme car tout les processus se sont
      //terminé
    }
    else{
      //On s'arrette
      kill(pids[i-1], SIGSTOP);

      //On a repris l'execution, donc on indique a son fils de reprendre
      //son execution
      kill(son, SIGCONT);

      //On attends un SIGCHLD qui atteste que le fils a repris son execution
      sigsuspend(&maskchld);

      //On envoie un SIGUSR1 au fils pour lui indiquer qu'il peut terminer
      //son exeuction
      kill(son, SIGUSR1);

      //on recupere le SIGCHLD qui indique que le fils a fini son execution
      sigsuspend(&maskchld);

      //On attends un SIGUSR1 du pere qui nous indique qu'on peut terminer
      //notre execution
      sigsuspend(&maskusr);
      exit(EXIT_SUCCESS);
    }
  }
  printf("Fin du programme !\n");
  return 0;
}


