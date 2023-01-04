/**
  ******************************************************************************
  * @file    pi.c
  * @brief   Ce fichier sert à implémenter le correcteur PI mais il n'est pas terminé.
  ******************************************************************************
  */
#include "pi.h"


#define Kp 1
#define Ki 0.1
#define Te_courant 0.0000625

extern float retour_courant;
float erreur;

void Asserv_courant(float commande){
	erreur= commande-retour_courant;
    //erreur*(Ki+(Kp*(Te_courant/2)));


}
