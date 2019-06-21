/***************
*	IOS - projekt c. 2
*	Roller Coaster
*	autor: Dominik Niederhofer
*	login: xniede03
*
****************/ 
#include "proj2.h"


Param_t param; // struktura parametru

FILE *out = NULL; //deklarace souboru

sem_t 	*load1, //deklarace semaforu
		*load2, 
		*unboard, 
		*unboard2,
		*citac, 
		*shotove, 
		*vypis, 
		*pas_citac;  

int car_pid = 0; // ulozeni id procesu voziku

//ukazatele na sdilenou pamet
int *A = NULL, //pocet akci 
	*cislo_pas = NULL, //cislo passengera
	*N = NULL,  //poradi vystupu
	*volne_misto = NULL, // != 0 volne misto ve voziku
	*hotove = NULL; // pocet hotovych procesu(passengeru)

int shm_pr = 0, //pomocne hodnoty pro sdilenou pamet
	shm_pas = 0,
	shm_n_id = 0,
	shm_vm_id = 0,
	shm_h_id = 0; 

int params(int argc, char *argv[])
{
	char *error;

	if (argc != 5)
		return 1;

	param.P = strtol(argv[1], &error, 10); if (*error) return 1;
	param.C = strtol(argv[2], &error, 10); if (*error) return 1;
	param.PT = strtol(argv[3], &error, 10); if (*error) return 1;
	param.RT = strtol(argv[4], &error, 10); if (*error) return 1;

	if 	(!(param.P > 0) ||
		!(param.C > 0) || 
		!(((param.P)%(param.C)) == 0) || 
		!(param.P > param.C) ||
		!(param.PT >= 0 && param.PT < 5001) ||
		!(param.RT >= 0 && param.RT < 5001)) 
			return 1;

	return 0;
}

int inc(int *cislo)
{
	int a = 0;

	sem_wait(citac);
	a = *cislo;
	(*cislo)++;
	sem_post(citac);
	
	return a;
}

//process car
void car()
{	
	int doba = 0;
	sem_wait(vypis);
	fprintf(out, "%i\t: C 1\t: started\n", inc(A));
	sem_post(vypis);

	for(int m = 0; m < ((param.P)/(param.C)); m++)
	{

		sem_wait(vypis);
		fprintf(out, "%i\t: C 1\t: load\n", inc(A));
		sem_post(vypis);

		//if ((*volne_misto) >= 0)
		sem_wait(citac);
		// spusteni operace load.. /nastupuje i pasazeru
			for (int i = 0; i < (*volne_misto); i++)
				sem_post(load1);
		sem_post(citac);

		//cekani dokud nenastoupi pasazeri
		sem_wait(load2);


		sem_wait(vypis);
		fprintf(out, "%i\t: C 1\t: run\n", inc(A));

		if (param.RT != 0)
		{
			doba = (random()%(param.RT));
			usleep(doba*1000);
		}
		sem_post(vypis);
		
		sem_wait(vypis);
		fprintf(out, "%i\t: C 1\t: unload\n", inc(A));
		sem_post(vypis);

		for (int i = 0; i < param.C; i++)
			sem_post(unboard);
		
		sem_wait(unboard2);
	}


	sem_wait(vypis);
	fprintf(out, "%i\t: C 1\t: finished\n", inc(A));
	sem_post(vypis);

}


void passenger()
{
	//inc(cislo_pas);
	int cis_pas = 0;// pro nahodnejsi start passengera
	sem_wait(pas_citac);
	cis_pas = *cislo_pas;
	(*cislo_pas)++;
	sem_post(pas_citac);

	sem_wait(vypis);
	fprintf(out, "%i\t: P %i\t: started\n", inc(A), cis_pas);
	sem_post(vypis);

	//ceka az se spusti operace load (pokyn k nastoupeni) 
	sem_wait(load1);


	sem_wait(vypis);
	fprintf(out, "%i\t: P %i\t: board\n", inc(A), cis_pas);
	sem_post(vypis);

	sem_wait(vypis);

	if (*N == param.C)
	{

		fprintf(out, "%i\t: P %i\t: board last\n", inc(A), cis_pas);
		*N = 1;

	} else
	{
		fprintf(out, "%i\t: P %i\t: board order %i\n", inc(A), cis_pas, (*N)++);
	}
	sem_post(vypis);

	sem_wait(citac);
	(*volne_misto)--;
	sem_post(citac);

	if ((*volne_misto) == 0)
		sem_post(load2);
	
	sem_wait(unboard);
	sem_wait(vypis);
	fprintf(out, "%i\t: P %i\t: unboard\n", inc(A), cis_pas);
	sem_post(vypis);


	sem_wait(vypis);
	if (*N == param.C)
	{ 
		fprintf(out, "%i\t: P %i\t: unboard last\n", inc(A), cis_pas);
		*N = 1;

	} else
	{
		fprintf(out, "%i\t: P %i\t: unboard order %i\n", inc(A), cis_pas, (*N)++);
	}
	sem_post(vypis);
	
	sem_wait(citac);
	(*volne_misto)++;
	sem_post(citac);

	if ((*volne_misto)==param.C)
		sem_post(unboard2);
	
	sem_wait(citac);
	(*hotove)++;
	sem_post(citac);

	if ((*hotove) == param.P) 
		for (int i = 0; i < param.P; i++)
			sem_post(shotove);
	//ukonceni
	sem_wait(shotove);
	sem_wait(vypis);
	fprintf(out, "%i\t: P %i\t: finished\n", inc(A), cis_pas);
	sem_post(vypis);

	
}


int init()
{
	// mapovani semaforu
	if (((load1 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) ||
		((load2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) ||
		((citac = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) ||
		((pas_citac = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) ||
		((vypis = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) 	||
		((shotove = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED)	||
		((unboard = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) ||
		((unboard2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED))
		return 2;

	// inicializace semaforu
	if 	((sem_init(load1, 1, 0) == -1) ||
		(sem_init(load2, 1, 0) == -1) ||
		(sem_init(citac, 1, 1) == -1) ||
		(sem_init(pas_citac, 1, 1) == -1) ||
		(sem_init(vypis, 1, 1) == -1) ||
		(sem_init(shotove, 1, 0) == -1) || 
		(sem_init(unboard, 1, 0) == -1) ||
		(sem_init(unboard2, 1, 0) == -1)) 
			return 2;

	//--alokoce pameti pro sdilenou pamet
	shm_pr = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
	shm_pas = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
	shm_n_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
	shm_vm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
	shm_h_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);

	// inicializace sdilene pameti 
	A = (int *) shmat(shm_pr, NULL, 0);
	cislo_pas = (int *) shmat(shm_pas, NULL, 0);
	N = (int *) shmat(shm_n_id, NULL, 0);
	volne_misto = (int *) shmat(shm_vm_id, NULL, 0);
	hotove = (int *) shmat(shm_h_id, NULL, 0);

	return 0;
}

void deinit()
{
	fclose(out);
	
	if 	((sem_destroy(load1) == -1) ||
		(sem_destroy(load2) == -1) ||
		(sem_destroy(citac) == -1) ||
		(sem_destroy(pas_citac) == -1) ||
		(sem_destroy(vypis) == -1) ||
		(sem_destroy(shotove) == -1) ||
		(sem_destroy(unboard) == -1) ||
		(sem_destroy(unboard2) == -1))
			{fprintf(stderr, "chyba, nepovedlo se smazat semafor/y");
			exit(2);}
	if 	((shmctl(shm_pr, IPC_RMID, NULL) == -1) ||
		(shmctl(shm_pas, IPC_RMID, NULL) == -1) ||
		(shmctl(shm_n_id, IPC_RMID, NULL) == -1) ||
		(shmctl(shm_vm_id, IPC_RMID, NULL) == -1) ||
		(shmctl(shm_h_id, IPC_RMID, NULL) == -1))
			fprintf(stderr, "chyba, nepovedlo se uvolnit pamet");
	exit(2);
}


int main(int argc, char *argv[])
{
	//deklarace promennych
	pid_t PIDcar;
	pid_t PIDpassenger;

	int doba = 0;

	//signaly pro ukonceni
	signal(SIGTERM, deinit);
	signal(SIGINT, deinit);

	// zpracovani parametru
	if(params(argc, argv) == 1)
	{
		fprintf(stderr, "Spatne zadane argumenty!\n");
		exit(1);
	}

	int childs[param.P];

	//otevreni vystupniho souboru
	if((out = fopen("proj2.out", "w")) == NULL)
	{
		fprintf(stderr, "chyba otevreni souboru pro vypis (proj2.out)");
		exit(2);
	}

	// pro spravny vypis
	setbuf(out, NULL);
	setbuf(stderr, NULL);

	//alokace a inicializace semaforu a sdilene pameti
	int ini = init();
	if (ini == 2)
	{
		fprintf(stderr, "chyba inicializace semaforu\n");
		exit(2);
	}

	//nastaveni citacu
	*A = 1;
	*cislo_pas = 1;
	*N = 1;
	*volne_misto = param.C;
	*hotove = 0;


	//vytvoreni procesu
	PIDcar = fork();


	if (PIDcar == 0)
	{	
		//vytvoreni procesu car
		PIDcar = fork();

		if (PIDcar == 0)
		{
			car(&param);
			exit(0);
		} else if (PIDcar > 0)
		{	
			car_pid = PIDcar;
		} else
		{

			fprintf(stderr, "chyba, nepodarilo se vytvorit proces car\n");
			deinit();
			exit(2);
		}
		
	} else if (PIDcar > 0)
	{	
		childs[0] = PIDcar;
		// vytvareni procesu passenger
		for(int m = 0; m < param.P; m++)
		{

			if (param.PT != 0)
			{
				doba = (random()%(param.RT));
				usleep(doba * 1000);
			}

			PIDpassenger = fork();
			
			if (PIDpassenger == 0)
			{
				passenger(&param);
				exit(0);
			} else if (PIDpassenger > 0)
			{
				childs[m+1] = PIDpassenger;
			} else
			{
				fprintf(stderr, "chyba, nepodarilo se vytvorit passengera\n");
				deinit();
				exit(2);
			}
		}
	} else

	{
		fprintf(stderr, "chyba, nepodarilo se vytvorit potomky\n");
		deinit();
		exit(2);
	}


	//cekani na potomky
	waitpid(car_pid, NULL,0);
	for(int i = 0; i <= param.P; i++)
		waitpid(childs[i], NULL, 0);
	

	deinit();
	kill(PIDcar, SIGTERM);
	kill(PIDpassenger, SIGTERM);

return 0;
}
