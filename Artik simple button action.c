#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#define HIGH 1
#define LOW 0
#define INPUT 1
#define OUTPUT 0

FILE *fp;
FILE *p=NULL;
int pin_input = 38;


bool digitalPinMode(int pin, int dir){
  FILE * fd;
  char fName[128];

  //Exporting the pin to be used
  if(( fd = fopen("/sys/class/gpio/export", "w")) == NULL) {
    printf("Error: unable to export pin\n");
    return false;
  }

  fprintf(fd, "%d\n", pin);
  fclose(fd);   // Setting direction of the pin
  sprintf(fName, "/sys/class/gpio/gpio%d/direction", pin);
  if((fd = fopen(fName, "w")) == NULL) {
    printf("Error: can't open pin direction\n");
    return false;
  }

  if(dir == OUTPUT) {
    fprintf(fd, "out\n");
  } else {
    fprintf(fd, "in\n");
  }

  fclose(fd);
  return true;
}


int digitalRead(int pin) {
  FILE * fd;
  char fName[128];
  char val[2];

  //Open pin value file
  sprintf(fName, "/sys/class/gpio/gpio%d/value", pin);
  if((fd = fopen(fName, "r")) == NULL) {
     printf("Error: can't open pin value\n");
     return false;
  }

  fgets(val, 2, fd);
  fclose(fd);

  return atoi(val);
}


bool digitalWrite(int pin, int val) {
  FILE * fd;
  char fName[128];

  // Open pin value file
  sprintf(fName, "/sys/class/gpio/gpio%d/value", pin);
  if((fd = fopen(fName, "w")) == NULL) {
    printf("Error: can't open pin value\n");
    return false;
  }

  if(val == HIGH) {
    fprintf(fd, "1\n");
  } else {
    fprintf(fd, "0\n");
  }

  fclose(fd);
  return true;
}


int
main(void)
{
  printf("button" "\n");

  do {
      usleep(1000*100);
      digitalPinMode(30,INPUT); //INPUT = 1
      if (!digitalRead(30))
      {
                system( "/scanset/apmode.sh" );
		sleep(4);
      }

      digitalPinMode(32,INPUT);
      if (!digitalRead(32))
      {
                system( "/scanset/usbmode.sh" );
		sleep(4);
      }

  }
  while(true);

  printf("End1!!!" "\n");
  return 0;
}

