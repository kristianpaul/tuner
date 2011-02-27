#include <ncurses.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/types.h>
#include <fcntl.h>

 void send(int x);
 void trans(void);
 void Delay( unsigned x);

 unsigned char bits[27]={0};
 int j,lim1,lim2,k,reset;
 int i,CeDelay;
 unsigned int divisor;
 float freq, oldfreq, offset;
 unsigned char data,loop,stat,lock;
 int band;

 #define BASEPORT 0x378 /* lp1 */


 int main()
 {
 reset=2000;
 CeDelay=800;
 lim1=400;
 lim2=300;
 freq=200.00;

printf("\nSoftware control for Panasonic NTSC Tuner from MTM Scientific, Inc. ");
printf("\nNote: The tuner requires +30V, +12V and +5V to operate. ");
printf("\nVisit http:\\www.mtmscientific.com for more information. \n\n");

bits[26]=0;  // bit 26 sets tuning voltage control on
bits[25]=2;  // bits 24 and 25 set divider ratio to 1024
bits[24]=0;
bits[23]=2;  // bits 21, 22 and 23 set test to normal mode
bits[22]=0;
bits[21]=0;
bits[20]=0;  // bit 20 sets charge pump current to 60 uA
bits[19]=2;  // bit 19 is don't care

// printf("\nEnter 0 to ignore lock signal, or 1 to sense lock signal. \n");
// scanf("%c", &lock);

// FM audio reception requires offsetting frequency by IF frequency
printf("\nEnter 0 for Television Receiver Mode, or 1 for FM Audio Receiver Mode. \n");
scanf("%f", &offset);
if(offset = 0) offset=0;
else offset=41.25;

while(1)

{
oldfreq=freq;
printf("\nEnter Frequency in MHz, 0 to exit, 1 for up, 2 for down\n");
scanf("%f",&freq);

if(freq > 860.00)
{
printf("Frequency should be less than 860 MHz\n");
// exit(0);
}

if(freq ==1)
{
freq = oldfreq + .03125;
printf("\nNew Frequency is %f \n", freq);
}

if(freq ==2)
{
freq = oldfreq - .03125;
printf("\nNew Frequency is %f \n", freq);
}

if(freq ==0)
exit(0);

if(freq>400.00)
{
bits[0]=0;
bits[1]=0;
bits[2]=0;  // bits 0, 1, 2, 3 are the band switch data
bits[3]=0;
band=0;
}


if((freq >180.00) && (freq <=400.00))
{
bits[0]=0;
bits[1]=0;
bits[2]=2;  // bits 0, 1, 2, 3 are the band switch data
bits[3]=0;
band=2;
}


if (freq <=180.00)
{
bits[0]=0;
bits[1]=2;
bits[2]=0;  // bits 0, 1, 2, 3 are the band switch data
bits[3]=0;
band=3;
}


divisor=(int)((freq + offset) / 0.03125); // FM audio uses offset of 41.25 MHZ


	for(k=18;k>3;k--)
	{
	bits[k]= ((divisor >>(18-k)) & (0x01));
	bits[k] <<=1; // bits 4 through 18 set the frequency
	}

	trans();

	Delay(1); // provides a 1 second delay for PLL lock

	stat= inb(0x379);
	stat = 0; // remove this line to monitor status from tuner

	if ((stat & 0x10) ==0)
	{
	 printf("LOCKED\n");
	}

	if((stat & 0x10))
	{

		printf("Did Not Lock. Will Try Lower Band\n");

		if (band==0)
		{
	    bits[0]=0;
		bits[1]=0;
		bits[2]=2;
		bits[3]=0;

	trans();
	Delay(1);
		stat= inb(0x379);

			if ((stat & 0x10) ==0)
					printf("LOCKED\n");
			if((stat & 0x10))
				{
				printf("Tried Lower Band. The Tuner did NOT LOCK. Check supply and wiring.\n"); 
			exit(0);
			}
		}

		if (band==2)
		{
	    bits[0]=0;
		bits[1]=2;
		bits[2]=0;
		bits[3]=0;
	trans();
	Delay(1);
		stat= inb(0x379);

			if ((stat & 0x10) ==0)
					printf("LOCKED\n");
			if((stat & 0x10))
				{
				printf("Tried Lower Band. The Tuner did NOT LOCK. Check supply and wiring.\n"); 
			exit(0);
		     }
       }

      if(band==3)
      {
	printf("This is the LOWEST BAND. The Tuner did NOT LOCK. Check supply and wiring.\n"); 
			exit(0);
	  }

    }
   }


}

void send(int delay)
 {
 int tem;
	 	
 //set permissions to access port
 if (ioperm(BASEPORT, 3, 1)) {perror("ioperm"); exit(1);}
	
 tem = fcntl(0, F_GETFL, 0);
 fcntl (0, F_SETFL, (tem | O_NDELAY));

 int j;
 outb(data,BASEPORT);
 for(j=0;j<delay;j++);
 return;
 }


 void trans(void)
{
data=0;
send(reset);
data=4;        // set the enable
send(CeDelay);
for (k=0;k<27;k++)
{
	for(i=0;i<4;i++) // this divides the clock in 4 sections
	{
	if(i==0)
	{
	data &=0x06;    // enable and data unchanged clock low
	send(lim1);
	}

	if(i==2)        // clock high for second part of the
	{
	data |=0x01;
	send(lim1/2);
	}

	if(i==3)
	{
	data &=0x05;
	data|=bits[k];
	send(lim1/2);
	}
   }
}
data=4;
send(CeDelay);
send(lim1);
data=0;
send(reset);
}

void Delay( unsigned x)
{
long tused;
time_t tstart, tstop;
time(&tstart);
	do
	{
	time(&tstop);
	}
	while( ( tused = (long)(difftime(tstop,tstart))) < x);
}
