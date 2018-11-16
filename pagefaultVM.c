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

extern struct SYSTEMFRAMETABLE *systemframetable;
extern struct PROCESSPAGETABLE *ptbr;

//funciones que tuve que traer para evitar declaración implicita
extern int copyframe(int sframe,int dframe);
extern int writeblock(char *buffer, int dblock);
extern int readblock(char *buffer, int sblock);
extern int loadframe(int frame);
extern int saveframe(int frame);

int getfreeframe();
int searchvirtualframe();
int OldestPage();

int pagefault(char *vaddress)
{
    int i;
    int frame,vframe;
    long pag_a_expulsar;
    int fd;
    char buffer[PAGESIZE];
    int pag_del_proceso;
    int oldest_page;

    // A partir de la dirección que provocó el fallo, calculamos la página
    pag_del_proceso=(long) vaddress>>12;

    //printf("estoy fallando aqui");
    if((ptbr+pag_del_proceso)->presente != 1 && (ptbr+pag_del_proceso)->framenumber != NINGUNO)// Si la página del proceso está en un marco virtual del disco
    {
      frame = (ptbr + pag_del_proceso)->framenumber;
	    // Lee el marco virtual al buffer
      readblock(buffer, frame);
      // Libera el frame virtual
      systemframetable[frame].assigned = 0;

      // Cuenta los marcos asignados al proceso
      i=countframesassigned();
      // Si ya ocupó todos sus marcos, expulsa una página
      if(i >= RESIDENTSETSIZE)
      {
        oldest_page = OldestPage(); //busca la página más vieja
        if(oldest_page == NINGUNO)
        {
			       printf("Error encontrando página más vieja");// Escribe el frame de la página en el archivo de respaldo y pon en 0 el bit de modificado
        }
        if((ptbr + oldest_page)->modificado == 1)// Si la página ya fue modificada, grábala en disco
        {
          (ptbr + oldest_page)->presente = 0; //expulsar página más vieja
          // Escribe el frame de la página en el archivo de respaldo y pon en 0 el bit de modificado
          (ptbr + oldest_page)->modificado = 0;
          frame = (ptbr + oldest_page)->framenumber;
          saveframe(frame);
        }
        vframe = searchvirtualframe(); // Busca un frame virtual en memoria secundaria
    	  if(vframe == NINGUNO)// Si no hay frames virtuales en memoria secundaria regresa error
    		{
          printf("Error No se encuentran MV libres");
          return(-1);
        }
          // Copia el frame a memoria secundaria, actualiza la tabla de páginas y libera el marco de la memoria principal
          copyframe(frame, vframe); //copio el MF (source) a un MV libre (destination)
          (ptbr + oldest_page)->presente = 0; //expulsar página más vieja
          (ptbr + oldest_page)->framenumber = vframe; //el numero de marco es ahora el virtual
          systemframetable[frame].assigned = 0;//el marco en memoria principal ahora esta libres
          systemframetable[vframe].assigned = 1;//el marco virtual esta ocupado ahora
      }
    }

    // Busca un marco físico libre en el sistema
    frame = getfreeframe();
	   // Si no hay marcos físicos libres en el sistema regresa error
     if(frame == NINGUNO)
    {
      printf("Error no hay marco físico libre");
        return(-1); // Regresar indicando error de memoria insuficiente
    }
    else// Si la página estaba en memoria secundaria
    {
        // Cópialo al frame libre encontrado en memoria principal y transfiérelo a la memoria física
        writeblock(buffer, frame);//copia a un buffer lo que hay en ese frame source
    }
	// Poner el bit de presente en 1 en la tabla de páginas y el frame
    (ptbr + pag_del_proceso)->presente = 1;
    (ptbr + pag_del_proceso)->framenumber = 1;
    return(1); // Regresar todo bien
}

int getfreeframe()
{
  /*int counter, flag;
  flag = 0;
  //Searches for a free frame in physical memory
  for(counter = framesbegin; counter < systemframetablesize + framesbegin ; counter++)
  {
    if(systemframetable[counter].assigned == 0)
    {
      systemframetable[counter].assigned = 1;
      flag = 1;
    }
      break;
  }
  if(flag == 0)
    counter = NINGUNO;
  return (counter);*/
  int i;
  // Busca un marco libre en el sistema
  for(i=framesbegin;i<systemframetablesize+framesbegin;i++)
      if(!systemframetable[i].assigned)
      {
          systemframetable[i].assigned=1;
          break;
      }
  if(i<systemframetablesize+framesbegin)
      systemframetable[i].assigned=1;
  else
      i=-1;
  return(i);
}

int searchvirtualframe()
{
  int counter;
  //searches for a free frame in virtual memory
  for(counter = (systemframetablesize + framesbegin); counter < 2*(systemframetablesize + framesbegin); counter++)
  {
    if(systemframetable[counter].assigned == 0)
    {
      systemframetable[counter].assigned = 1;
      break;
    }
  }
  if(counter<2*(systemframetablesize + framesbegin))
  {
    systemframetable[counter].assigned = 1;
  } else
      counter = NINGUNO;
    return (counter);
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
