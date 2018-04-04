
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


/*
 *
 * Print a greeting message on standard output and exit.
 *
 * On embedded platforms this might require semi-hosting or similar.
 *
 * For example, for toolchains derived from GNU Tools for Embedded,
 * to enable semi-hosting, the following was added to the linker:
 *
 * --specs=rdimon.specs -Wl,--start-group -lgcc -lc -lc -lm -lrdimon -Wl,--end-group
 *
 * Adjust it for other toolchains.
 *
 */


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
  printf("Hello ARM World!" "\n");

  //LED
  //– Red LED: sysfs GPIO 28 on ARTIK 530/710
  //– Blue LED: sysfs GPIO 38 on ARTIK 530/710
  //sysfs GPIO 30 for SW403 (next to board edge, alongside Red LED)
  //sysfs GPIO 32 for SW404 (alongside Blue LED)

  do {
      digitalPinMode(30,INPUT); //INPUT = 1
      printf("digitalPinMode(30,INPUT)!" "\n");
      if (!digitalRead(30))
      {
          digitalPinMode(28,OUTPUT); //OUTPUT = 0
          digitalWrite(28,HIGH); //HIGH = 1
      }

      printf("11111111111111111111111111" "\n");
      digitalPinMode(32,INPUT);
      if (!digitalRead(32))
      {
          digitalPinMode(28,OUTPUT); //OUTPUT = 0
          digitalWrite(28,LOW); //HIGH = 1
      }
      printf("22222222222222222222222222222" "\n");

  }
  while(true);

  printf("End1!!!" "\n");
  return 0;
}
