#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <mmu.h>

#define NUMPROCS 4
#define PAGESIZE 4096
#define PHISICALMEMORY 12*PAGESIZE
#define TOTFRAMES PHISICALMEMORY/PAGESIZE //12 marcos de 4kb
#define RESIDENTSETSIZE PHISICALMEMORY/(PAGESIZE*NUMPROCS) //3 Cantidad de frames que puede tener un proceso en MF
#define PROCESS_PAGES 6

extern char *base;
extern int framesbegin;
extern int idproc;
extern int systemframetablesize;
extern int ptlr;

char buffer[PAGESIZE];
int framesVM[24] = {0};

extern struct SYSTEMFRAMETABLE *systemframetable;
extern struct PROCESSPAGETABLE *ptbr;


int getfreeframe();
int searchvirtualframe();
int getfifo();
int OldestPage();

int pagefault(char *vaddress)
{
    int i;
    int frame,vframe;
    long pag_a_expulsar;
    int fd;
    char buffer[PAGESIZE];
    int pag_del_proceso;

    // A partir de la dirección que provocó el fallo, calculamos la página
    pag_del_proceso=(long) vaddress>>12;
    // Cuenta los marcos asignados al proceso
    i=countframesassigned();

    if((ptbr + pag_del_proceso)->framenumber == NINGUNO)// Si la página del proceso está en un marco virtual del disco
    {

		// Lee el marco virtual al buffer

        // Libera el frame virtual
    }

    // Si ya ocupó todos sus marcos, expulsa una página
    if(i>=RESIDENTSETSIZE)
    {
		// Buscar una página a expulsar

		// Poner el bitde presente en 0 en la tabla de páginas

        // Si la página ya fue modificada, grábala en disco
        {
			// Escribe el frame de la página en el archivo de respaldo y pon en 0 el bit de modificado
        }

        // Busca un frame virtual en memoria secundaria
		// Si no hay frames virtuales en memoria secundaria regresa error
		{
            return(-1);
        }
        // Copia el frame a memoria secundaria, actualiza la tabla de páginas y libera el marco de la memoria principal
    }

    // Busca un marco físico libre en el sistema
	// Si no hay marcos físicos libres en el sistema regresa error
    {
        return(-1); // Regresar indicando error de memoria insuficiente
    }

    // Si la página estaba en memoria secundaria
    {
        // Cópialo al frame libre encontrado en memoria principal y transfiérelo a la memoria física
    }

	// Poner el bit de presente en 1 en la tabla de páginas y el frame


    return(1); // Regresar todo bien
}

int getfreeframe()
{
  int counter, flag;
  flag = 0;
  //Searches for a free frame in physical memory
  for(counter = framesbegin; counter < systemframetablesize + framesbegin ; counter++)
  {
    if(systemframetable[counter].assigned == 0)
    {
      systemframetable[counter] = 1;
      flag = 1;
    }
      break;
  }
  if(flag == 0)
    counter = NINGUNO;
  else
    return (counter);
  }
}

int searchvirtualframe()
{
  int counter;
  //searches for a free frame in virtual memory


}

int OldestPage()
{
  int counter;
  int oldest_page = 0;
  unsigned long smallest_time = 0, time_dif = 0;
  //Searches for the oldest accessed page by selecting the one with the shortest time between it arrived and when it was last accessed
  for(counter = 0; counter < PROCESS_PAGES; counter++)
  {
    if((ptbr + counter)->presente == 1)
    {//time calculation of the amount of time
      time_dif = (ptbr + counter)-> tlastaccess - (ptbr + counter)->tarrived;
      if(smallest_time <= time_dif)
      {
        oldest_page = counter;
        smallest_time = time_dif;
      }
    }
  }
  return oldest_page;
}
