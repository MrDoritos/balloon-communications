#include "/home/ian/cpp-projects/balloon-communications/serialClient.h"
#include "/home/ian/cpp-projects/balloon-communications/packets.h"

serialClient sClient;

timeSync tSync(sClient);
tempSensor tSense[3] = {tempSensor(sClient, 1, tSync),
                        tempSensor(sClient, 2, tSync),
                        tempSensor(sClient, 3, tSync)};
uvSensor uvSense(sClient, 1, tSync);



void setup() {
  while (!Serial);
  
  Serial.begin(9600);

  tSync.send(millis());
}

void loop() {
  tSense[0].send(0);
  tSense[1].send(1);
  tSense[2].send(2);
  uvSense.send(3);
  delay(1000);
}
