#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <RF24/RF24.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);

typedef struct
{
  uint32_t id;
  float temperature;
} SPacket;

uint8_t addr[6] = {0xC6, 0x2F, 0x6B, 0xA5, 0xC2, 0xD3};
char cmd[128];

int main(void)
{
  radio.begin();
  radio.setChannel(0x70);
  radio.setPayloadSize(sizeof(SPacket));
  radio.openReadingPipe(1, addr);
  radio.startListening();
  radio.printDetails();

  signal(SIGCHLD, SIG_IGN);

  for(;;)
  {
    if (radio.available())
    {
      SPacket packet;
      radio.read(&packet, sizeof(SPacket));
      if (packet.id != 0)
      {
        if(fork() == 0)
	{
		sprintf(cmd, "rrdtool update temp%d.rrd N:%f", packet.id, packet.temperature);
	        printf("%s\n", cmd);
        	system(cmd);
		exit(0);
	}
	if(fork() == 0)
	{
        	sprintf(cmd, "echo \"eschenbach.temperature.%d %f `date +%%s`\" | nc -q0 graphite.draradech.de 2003", packet.id, packet.temperature);
	        printf("%s\n", cmd);
        	system(cmd);
		exit(0);
	}
      }
    }
    else
    {
      usleep(10000);
    }
  }
  return 0;
}
