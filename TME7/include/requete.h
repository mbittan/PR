#ifndef _REQUETE_H_
#define _REQUETE_H_

#define SETENV 1
#define GETENV 2

#define OK 0
#define EXISTEPAS 1
#define VALINCORRECTE 2

#define TAILLE_MAX 128

typedef struct _req {
  char type;
  char variable[TAILLE_MAX];
  char valeur[TAILLE_MAX];
} req_t;

typedef struct _resultat {
  char res;
  char valeur[TAILLE_MAX];
} result_t;

#endif
