/***************
*	IOS - projekt c. 2
*	Roller Coaster
*	autor: Dominik Niederhofer
*	login: xniede03
*
****************/ 


#include <stdio.h> 
#include <stdlib.h> 
#include <sys/shm.h> 
#include <sys/wait.h> 
#include <semaphore.h>  
#include <unistd.h>  
#include <sys/mman.h> 


typedef struct param {
	int P; //pocet procesu passenger (pasazeru)
	int C; //kapacita voziku
	int PT; //maximalni doba generovani pasazera
	int RT; //maximalni doba prujezdu trati
} Param_t;


//deklarace funkci
int main(int argc, char *argv[]);
int params(int argc, char *argv[]);
int inc(int *cislo);
void car();
void passenger();
int init(); 
void deinit();
