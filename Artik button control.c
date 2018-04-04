#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#define buf 128
#define HIGH 1
#define LOW 0
#define INPUT 1
#define OUTPUT 0
FILE *fp;
FILE *p=NULL;
enum {
	__POWER_LED = 0,
	__WLAN_LED,
	__USR_LED,
	__USB_LED,
	__WIFI_LED,
	__USB_MODE,
	__END_END,
}
;
enum {
	__MODE_BTN = 0,
	__PORT_BTN,
}
;
unsigned char outPinNumber[] = {
	28, 38, 128, 129, 130, 14
}
;
unsigned char inputPinNumber[] = {
	30, 32
}
;
int usbPortSwitch = 1 ;
bool digitalPinMode(int pin, int dir) {
	FILE * fd;
	char fName[128];
	//Exporting the pin to be used
	if(( fd = fopen("/sys/class/gpio/export", "w")) == NULL) {
		printf("Error: unable to export pin\n");
		return false;
	}
	fprintf(fd, "%d\n", pin);
	fclose(fd);
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
void Inital_Pin( void ) {
	int i;
	for ( i = 0; i < __END_END; i ++ ) {
		digitalPinMode( outPinNumber[i], OUTPUT );
	}
	for ( i = 0; i <= __PORT_BTN; i ++ ) {
		digitalPinMode( inputPinNumber[i], INPUT );
	}
}
void wifimode( void ) {
	if (!digitalRead(inputPinNumber[__MODE_BTN])) {
		printf("AP mode reset");
		system( "rm /scanset/wpa_psk.conf" );
		system( "rm /scanset/wifi" );
		digitalWrite( outPinNumber[__POWER_LED], HIGH );
		digitalWrite( outPinNumber[__WLAN_LED], HIGH );
		digitalWrite( outPinNumber[__USR_LED], HIGH );
		digitalWrite( outPinNumber[__USB_LED], HIGH );
		digitalWrite( outPinNumber[__WIFI_LED], HIGH );
		sleep(1);
		system( "reboot" );
	}
}
void usbmode( void ) {
	if (usbPortSwitch == 0 ) {
		printf("Switch to Wi-fi mode\n");
		digitalWrite( outPinNumber[__USB_MODE], LOW );
		digitalWrite( outPinNumber[__WIFI_LED], HIGH );
		digitalWrite( outPinNumber[__USB_LED], LOW );
		usbPortSwitch = 1;
	} else {
		printf("Switch to USB mode");
		digitalWrite( outPinNumber[__USB_MODE], HIGH );
		digitalWrite( outPinNumber[__WIFI_LED], LOW );
		digitalWrite( outPinNumber[__USB_LED], HIGH );
		usbPortSwitch = 0;
	}
}
void wifistart( void ) {
	int nResult = access("wpa_psk.conf", 0 );
	if( nResult == 0 ) {
		printf("The wireless configuration file is verified \nEnable wlan0 ." );
		system("systemctl stop connman");
		system("ifconfig wlan0 up");
		printf("wlan0 connection.\n");
		system("wpa_supplicant -B -iwlan0 -c /scanset/wpa_psk.conf -Dwext");
		printf("dhclient wlan0.\n");
		system("dhclient wlan0");
		system("ifconfig");
		digitalWrite( outPinNumber[__WLAN_LED], HIGH );
	} else if( nResult == -1 ) {
		printf( "The wireless configuration file is not verified.\n" );
		printf( "Set to AP mode.\n" );
		printf( "MBoard S/N check.\n" );
		system( "lshw -C system > system.txt" );
		char str[buf];
		fp = fopen("system.txt","r");
		int i;
		for ( i = 0; i < 4; i++) {
			fgets(str, buf, fp);
		}
		fclose(fp);
		char *ptr;
		char cmd_buffer[256];
		char snCut[10];
		ptr = strtok(str,":");
		ptr = strtok(NULL, " ");
		memcpy( snCut,ptr+3,13);
		snCut[13]='\0';
		sprintf(cmd_buffer, "sed -i '3s/.*/ssid=voim-%s/g' hostapd.conf", snCut);
		system(cmd_buffer);
		printf( "AP mode change.\n" );
		system( "systemctl stop connman" );
		system("ifconfig wlan0 192.168.1.1 up" );
		system("dnsmasq -C /etc/dnsmasq.conf" );
		system("sysctl net.ipv4.ip_forward=1" );
		system("iptables --flush" );
		system("iptables -t nat --flush" );
		system("iptables --delete-chain" );
		system("iptables -t nat --delete-chain" );
		system("iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE" );
		system("iptables -A FORWARD -i wlan0 -j ACCEPT" );
		system("hostapd /scanset/hostapd.conf -B" );
	}
	digitalWrite( outPinNumber[__WIFI_LED], HIGH );
}
int main(void) {
	int i;
	Inital_Pin();
	wifimode();
	wifistart();
	digitalWrite( outPinNumber[__POWER_LED], HIGH );
	while(1) {
		usleep(1000*100);
		if (!digitalRead(inputPinNumber[__MODE_BTN])) {
			if (digitalRead( outPinNumber[__USR_LED] )) {
				printf("Session in use.\n");
				sleep(1);
			} else {
				time_t start, end;
				double diff;
				time (&start);
				while (1) {
					if (!digitalRead(inputPinNumber[__MODE_BTN])) {
						usleep(1000*100);
						time (&end);
						diff = difftime (end, start);
						if( diff >= 3 ) {
							wifimode();
						} else {
						}
					} else {
						printf( "Press the MODE button for 3 seconds or more.\n" );
						break;
					}
				}
			}
		}
		if (!digitalRead(inputPinNumber[__PORT_BTN])) {
			if (digitalRead( outPinNumber[__USR_LED] )) {
				printf("Session in use.\n");
				sleep(1);
			} else {
				time_t start, end;
				double diff;
				time (&start);
				while (1) {
					if (!digitalRead(inputPinNumber[__PORT_BTN])) {
						usleep(1000*100);
						time (&end);
						diff = difftime (end, start);
						if( diff >= 3 ) {
							usbmode();
							sleep(1);
							break;
						} else {
						}
					} else {
						printf( "Press the MODE button for 3 seconds or more.\n" );
						break;
					}
				}
			}
		}
	}
	return 0;
}
