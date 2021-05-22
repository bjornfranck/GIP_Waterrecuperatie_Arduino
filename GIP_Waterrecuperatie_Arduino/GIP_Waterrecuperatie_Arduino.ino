//--------------------------------------------------------------
/*
 * Project: GIP Waterrecuperatie
 * Versie: 1.0
 * Datum: 15/02/2021
 * Datum laatste verandering: 22/05/2021
 * Namen: Owen Nolis, Yenthe van Den Eynden & Björn Franck
 */
//--------------------------------------------------------------
// Bibliotheken:
#include <SPI.h>
#include <Controllino.h>
#include <Ethernet.h>
#include <DallasTemperature.h>
#include <OneWire.h>
//--------------------------------------------------------------
// Temperatuurmeting:
const int tempPin = CONTROLLINO_D3;
OneWire oneWire(tempPin);
DallasTemperature temp(&oneWire);
float tempResult;
//--------------------------------------------------------------
// Niveaumeting regenwater:
const int trigRain1 = CONTROLLINO_D6;
const int echoRain1 = CONTROLLINO_D7;
const int trigRain2 = CONTROLLINO_D8;
const int echoRain2 = CONTROLLINO_D9;
float distanceRain1;
float distanceRain2;
float distanceRain1Web;
float distanceRain2Web;
float distanceRain;
//--------------------------------------------------------------
// Niveaumeting leidingwater:
const int trigTap1 = CONTROLLINO_D10;
const int echoTap1 = CONTROLLINO_D11;
const int trigTap2 = CONTROLLINO_D12;
const int echoTap2 = CONTROLLINO_D13;
float distanceTap1;
float distanceTap2;
float distanceTap1Web;
float distanceTap2Web;
float distanceTap;
//--------------------------------------------------------------
// Niveaumeting bad:
const int trigBath = CONTROLLINO_D14;
const int echoBath = CONTROLLINO_D15;
float distanceBath;
float distanceBathWeb;
//--------------------------------------------------------------
// Niveaumetingen:
float levelResultRain;
float levelResultTap;
float levelResultBath;
float volumeResultRain;
float volumeResultTap;
float volumeResultBath;
float levelResultRainWeb;
float levelResultTapWeb;
float levelResultBathWeb;
float volumeResultRainWeb;
float volumeResultTapWeb;
float volumeResultBathWeb;
float volumeResultTap1;
float volumeResultTap1Web;
float volumeResultTap2;
float volumeResultTap2Web;
float levelResultTap1;
float levelResultTap1Web;
float levelResultTap2;
float levelResultTap2Web;
float volumeResultRain1;
float volumeResultRain1Web;
float volumeResultRain2;
float volumeResultRain2Web;
float levelResultRain1;
float levelResultRain1Web;
float levelResultRain2;
float levelResultRain2Web;
//--------------------------------------------------------------
// Solid State Relais:
const int pulse = CONTROLLINO_D1;
bool pulseState = false;
//--------------------------------------------------------------
// Consumption:
#define FLOWSENSORPIN CONTROLLINO_D2
float liters;
volatile uint16_t pulses = 0;
volatile uint8_t lastflowpinstate;
volatile uint32_t lastflowratetimer = 0;
volatile float flowrate;

SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return;
  }
  
  if (x == HIGH) {
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;
  lastflowratetimer = 0;
}
//--------------------------------------------------------------
// Webserver:
const float scale = 5.0/1024;
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
IPAddress ip(192,168,1,3); // IP Controllino
IPAddress serverTCP(192,168,1,2); // IP Computer
EthernetServer server(80);
//--------------------------------------------------------------
void setup() 
{
  // Temperatuurmeting:
  pinMode(tempPin, INPUT);
  temp.begin();

  // Niveaumeting regenwater:
  pinMode(trigRain1, OUTPUT);
  pinMode(echoRain1, INPUT);
  pinMode(trigRain2, OUTPUT);
  pinMode(echoRain2, INPUT);
  
  // Niveaumeting leidingwater:
  pinMode(trigTap1, OUTPUT);
  pinMode(echoTap1, INPUT);
  pinMode(trigTap2, OUTPUT);
  pinMode(echoTap2, INPUT);

  // Niveaumeting bad:
  pinMode(trigBath, OUTPUT);
  pinMode(echoBath, INPUT);

  // SSR (Solid State Relais):
  pinMode(pulse, OUTPUT);

  // Consumption:
  pinMode(FLOWSENSORPIN, INPUT);
  digitalWrite(FLOWSENSORPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSORPIN);
  useInterrupt(true);

  // Webserver:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server online. IP: ");
  Serial.println(Ethernet.localIP());

  Serial.begin(9600);
}
//--------------------------------------------------------------
void loop() 
{
  // Het uitvoeren van de verschillende voids.
  
  temperature();
  levelRain();
  levelTap();
  levelBath();
  consumption();
  ssr();
  networkcommunication();
  webserver();

  Serial.println();
  
  delay(1000);
}
//--------------------------------------------------------------
void temperature()
{
  // Het vragen van de temperatuur en omvormen tot graden Celcius.
  // Dan printen naar de serieële monitor.
  
  temp.requestTemperatures();
  tempResult = temp.getTempCByIndex(0);

  Serial.print("Temperature: ");
  Serial.print(tempResult, 2);
  Serial.println(" *C");
  
  delay(100);
}
//--------------------------------------------------------------
void levelRain()
{
  // Het uitlezen en berekenen van de 2 sensorwaarde in een nuttige
  // grootheid. Daarna al de juiste waarde in de correcte variabelen
  // zetten. Ook wordt er naar de seriële monitor geprint om te zien
  // wat de waarden precies zijn en of er eventuele fouten zijn. 
  
  digitalWrite(trigRain1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigRain1, LOW);
 
  float distanceRain1 = pulseIn(echoRain1, HIGH);
  distanceRain1 = distanceRain1 / 58;

  distanceRain1Web = distanceRain1;

  volumeResultRain1 = 62 - distanceRain1;
  volumeResultRain1 = volumeResultRain1 * 0.80;
  levelResultRain1 = (volumeResultRain1 / 176) * 100;

  levelResultRain1Web = levelResultRain1;
  volumeResultRain1Web = volumeResultRain1;

  digitalWrite(trigRain2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigRain2, LOW);
 
  float distanceRain2 = pulseIn(echoRain2, HIGH);
  distanceRain2 = distanceRain2 / 58;

  distanceRain2Web = distanceRain2;

  volumeResultRain2 = 62 - distanceRain2;
  volumeResultRain2 = volumeResultRain2 * 0.80;
  levelResultRain2 = (volumeResultRain2 / 176) * 100;

  levelResultRain2Web = levelResultRain2;
  volumeResultRain2Web = volumeResultRain2;

  distanceRain = (distanceRain1 + distanceRain2) / 2;
  volumeResultRain = 62 - distanceRain; 
  volumeResultRain = volumeResultRain * 0.80;
  levelResultRain = (volumeResultRain / 32) * 100;

  levelResultRainWeb = levelResultRain;
  volumeResultRainWeb = volumeResultRain;

  if(distanceRain1 > 0 && distanceRain2 > 0)
  {
    
    Serial.print("Level rainwater 1: ");
    Serial.print(distanceRain1);
    Serial.println(" cm");

    Serial.print("Level rainwater 2: ");
    Serial.print(distanceRain2);
    Serial.println(" cm");

    Serial.print("Average level rainwater: ");
    Serial.print(distanceRain);
    Serial.println(" cm"); 
  }
  
  if(distanceRain1 == 0 && distanceRain2 == 0)
  {
    Serial.print("Level rainwater 1: ");
    Serial.println(" ERROR");

    Serial.print("Level rainwater 2: ");
    Serial.println(" ERROR");
  }
  
  if(distanceRain1 == 0 && distanceRain2 > 0)
  {
    levelResultRainWeb = levelResultRain2Web;
    volumeResultRainWeb = volumeResultRain2Web;
    
    Serial.print("Level rainwater 1: ");
    Serial.println(" ERROR");

    Serial.print("Level rainwater 2: ");
    Serial.print(distanceRain2);
    Serial.println(" cm");
  }

  if(distanceRain2 == 0 && distanceRain1 > 0)
  {
    levelResultRainWeb = levelResultRain1Web;
    volumeResultRainWeb = volumeResultRain1Web;
    
    Serial.print("Level rainwater 1: ");
    Serial.print(distanceRain1);
    Serial.println(" cm");

    Serial.print("Level rainwater 2: ");
    Serial.println(" ERROR");
  }

  delay(100);
}
//--------------------------------------------------------------
void levelTap()
{
  // Het uitlezen en berekenen van de 2 sensorwaarde in een nuttige
  // grootheid. Daarna al de juiste waarde in de correcte variabelen
  // zetten. Ook wordt er naar de seriële monitor geprint om te zien
  // wat de waarden precies zijn en of er eventuele fouten zijn. 
  
  digitalWrite(trigTap1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigTap1, LOW);
 
  float distanceTap1 = pulseIn(echoTap1, HIGH);
  distanceTap1 = distanceTap1 / 58;

  distanceTap1Web = distanceTap1;

  volumeResultTap1 = 62 - distanceTap1;
  volumeResultTap1 = volumeResultTap1 * 4.40;
  levelResultTap1 = (volumeResultTap1 / 176) * 100;

  levelResultTap1Web = levelResultTap1;
  volumeResultTap1Web = volumeResultTap1;

  digitalWrite(trigTap2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigTap2, LOW);
 
  float distanceTap2 = pulseIn(echoTap2, HIGH);
  distanceTap2 = distanceTap2 / 58;

  distanceTap2Web = distanceTap2;

  volumeResultTap2 = 62 - distanceTap2;
  volumeResultTap2 = volumeResultTap2 * 4.40;
  levelResultTap2 = (volumeResultTap2 / 176) * 100;

  levelResultTap2Web = levelResultTap2;
  volumeResultTap2Web = volumeResultTap2;

  distanceTap = (distanceTap1 + distanceTap2) / 2;
  volumeResultTap = 62 - distanceTap;
  volumeResultTap = volumeResultTap * 4.40;
  levelResultTap = (volumeResultTap / 176) * 100;

  levelResultTapWeb = levelResultTap;
  volumeResultTapWeb = volumeResultTap;

  if(distanceTap1 > 0 && distanceTap2 > 0)
  {
    Serial.print("Level tapwater 1: ");
    Serial.print(distanceTap1);
    Serial.println(" cm");

    Serial.print("Level tapwater 2: ");
    Serial.print(distanceTap2);
    Serial.println(" cm");

    Serial.print("Average level tapwater: ");
    Serial.print(distanceTap);
    Serial.println(" cm");
  }
  
  if(distanceTap1 == 0 && distanceTap2 == 0)
  {
    Serial.print("Level tapwater 1: ");
    Serial.println(" ERROR");

    Serial.print("Level tapwater 2: ");
    Serial.println(" ERROR");
  }
  
  if(distanceTap1 == 0 && distanceTap2 > 0)
  {
    levelResultTapWeb = levelResultTap2Web;
    volumeResultTapWeb = volumeResultTap2Web;
  
    Serial.print("Level tapwater 1: ");
    Serial.println(" ERROR");

    Serial.print("Level tapwater 2: ");
    Serial.print(distanceTap2);
    Serial.println(" cm");
  }

  if(distanceTap2 == 0 && distanceTap1 > 0)
  {
    levelResultTapWeb = levelResultTap1Web;
    volumeResultTapWeb = volumeResultTap1Web;
    
    Serial.print("Level tapwater 1: ");
    Serial.print(distanceTap1);
    Serial.println(" cm");

    Serial.print("Level tapwater 2: ");
    Serial.println(" ERROR");
  }

  delay(100);
}
//--------------------------------------------------------------
void levelBath()
{
  // Het uitlezen en berekenen van de sensorwaarde in een nuttige
  // grootheid. Daarna al de juiste waarde in de correcte variabelen
  // zetten. Ook wordt er naar de seriële monitor geprint om te zien
  // wat de waarden precies zijn en of er eventuele fouten zijn. 
  
  digitalWrite(trigBath, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigBath, LOW);
 
  float distanceBath = pulseIn(echoBath, HIGH);
  distanceBath = distanceBath / 58;

  distanceBathWeb = distanceBath;
  
  volumeResultBath = 101.5 - distanceBath;
  volumeResultBath = volumeResultBath * 4.90;                  // LITERS
  levelResultBath = (volumeResultBath / 232.85) * 100;         // PERCENTAGE

  levelResultBathWeb = levelResultBath;
  volumeResultBathWeb = volumeResultBath;

  if(distanceBath > 0)
  {
    Serial.print("Level Bath: ");
    Serial.print(distanceBath);
    Serial.println(" cm");
  }

  if(distanceBath == 0)
  {
    Serial.print("Level Bath: ");
    Serial.println(" ERROR");
  }

  if(distanceBath >= 30 && pulseState == false)
  {
     digitalWrite(pulse, HIGH);
     pulseState = true;

     delay(100);
  }

  if(distanceBath <= 15 && pulseState == true)
  {
    digitalWrite(pulse, LOW);
    pulseState = false;

    delay(100);
  }

  Serial.print("SSR: ");
  Serial.println(pulseState);

  delay(100);
}
//--------------------------------------------------------------
void ssr()
{
  // Vergelijken van de gemeten afstand en bepalen of de SSR
  // geschakeld mag worden of niet. Er is voorzien dat de SSR
  // niet elke keer aan- en uitgaat bij kleine verschillen in
  // de afstand.
  
  if(distanceBath >= 30 && pulseState == false)
  {
     digitalWrite(pulse, HIGH);
     pulseState = true;

     delay(100);
  }

  if(distanceBath <= 15 && pulseState == true)
  {
    digitalWrite(pulse, LOW);
    pulseState = false;

    delay(100);
  }

  Serial.print("SSR: ");
  Serial.println(pulseState);
}
//--------------------------------------------------------------
void consumption()
{
  // Het printen van de frequentie en pulsen. Daarna worden er
  // berekeningen gedaan voor het aantal liters te berekenen.
  // Ook wordt er een interrupt gebruikt zodat er geen gegevens
  // verloren gaan.
  
  Serial.print("Freq: "); Serial.println(flowrate);
  Serial.print("Pulses: "); Serial.println(pulses, DEC);

  liters = pulses;
  liters /= 7.5;
  liters /= 60.0;

  Serial.print(liters); Serial.println(" Liters");
  delay(100);
}

void useInterrupt(boolean v) {
  if (v) {
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } 
  else {
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

//--------------------------------------------------------------
void networkcommunication()
{
  // Het doorsturen van alle verzamelde data naar de ASP.NET website.
  
  Serial.println("Send my data");
  EthernetClient client;
  
  if(client.connect(serverTCP, 5320))
  {
    Serial.println("Client connected");
    
    String split = ";";
    client.println(levelResultRainWeb + split + levelResultTapWeb 
    + split + levelResultBathWeb + split + pulseState + split 
    + tempResult + split + liters + split + volumeResultRainWeb 
    + split + volumeResultTapWeb + split + volumeResultBathWeb);
    
    delay(10);
  }
  else{
    Serial.println("No transfering of data");
  }
  delay(10);
    
  client.stop();
  delay(1000);   
}
//-------------------------------------------------------------
void webserver()
{
  // De inhoud die op de lokale webserver van de Controllino wordt
  // gezet en toegankelijk is via WiFi of Ethernet.
  
  EthernetClient client = server.available();

  if (client) 
  { 
    while (client.connected()) 
    {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 5");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head><title>GIP Waterrecuperatie</title><style>body{font-family: Arial;}</style></head>");
          client.println("<body>");
          client.println("<h3 style='color: blue;'>GIP Waterrecuperatie</h3>");

          client.println("</br>");
          
          // Temperatuurmeting:
          client.print("<p>Temperature: ");
          client.print(tempResult, 2);
          client.println(" *C</p>");

          // Niveaumeting regenwater:
          if(distanceRain1Web > 0 && distanceRain2Web > 0)
          {
            client.print("<p>Niveau regenwater: ");
            client.print(levelResultRainWeb);
            client.println(" %</p>");

            client.print("<p>Volume regenwater: ");
            client.print(volumeResultRainWeb);
            client.println(" L</p>");
          }
          
          if(distanceRain1Web == 0 && distanceRain2Web == 0)
          {
            client.print("<p>Niveau regenwater: ");
            client.println(" FOUT (Beide sensoren zijn niet actief)</p>");

            client.print("<p>Volume regenwater: ");
            client.println(" FOUT (Beide sensoren zijn niet actief)</p>");
          }
          
          if(distanceRain1Web == 0 && distanceRain2Web > 0)
          {
            client.print("<p>Niveau regenwater: ");
            client.print(levelResultRain2Web);
            client.println(" % - FOUT (Sensor 1 is niet actief)</p>");

            client.print("<p>Volume regenwater: ");
            client.print(volumeResultRain2Web);
            client.println(" L - FOUT (Sensor 1 is niet actief)</p>");
          }
        
          if(distanceRain2Web == 0 && distanceRain1Web > 0)
          {
            client.print("<p>Niveau regenwater: ");
            client.print(levelResultRain1Web);
            client.println(" % - FOUT (Sensor 2 is niet actief)</p>");
            
            client.print("<p>Volume regenwater: ");
            client.print(volumeResultRain1Web);
            client.println(" L - FOUT (Sensor 2 is niet actief)</p>");
          }

          // Niveaumeting leidingwater:
          if(distanceTap1Web > 0 && distanceTap2Web > 0)
          {
            client.print("<p>Niveau leidingwater: ");
            client.print(levelResultTapWeb);
            client.println(" %</p>");

            client.print("<p>Volume leidingwater: ");
            client.print(volumeResultTapWeb);
            client.println(" L</p>");
          }
          
          if(distanceTap1Web == 0 && distanceTap2Web == 0)
          {
            client.print("<p>Niveau leidingwater: ");
            client.println(" FOUT (Beide sensoren zijn niet actief)</p>");

            client.print("<p>Volume leidingwater: ");
            client.println(" FOUT (Beide sensoren zijn niet actief)</p>");
          }
          
          if(distanceTap1Web == 0 && distanceTap2Web > 0)
          {
            client.print("<p>Niveau leidingwater: ");
            client.print(levelResultTap2Web);
            client.println(" % - FOUT (Sensor 1 is niet actief)</p>");

            client.print("<p>Volume leidingwater: ");
            client.print(volumeResultTap2Web);
            client.println(" L -  FOUT (Sensor 1 is niet actief)</p>");
          }
        
          if(distanceTap2Web == 0 && distanceTap1Web > 0)
          {
            client.print("<p>Niveau leidingwater: ");
            client.print(levelResultTap1Web);
            client.println(" % - FOUT (Sensor 2 is niet actief)</p>");

            client.print("<p>Volume leidingwater: ");
            client.print(volumeResultTap1Web);
            client.println(" L - FOUT (Sensor 2 is niet actief)</p>");
          }

          // Niveaumeting bad:
          if(distanceBathWeb > 0)
          {
            client.print("<p>Niveau bad: ");
            client.print(levelResultBathWeb);
            client.println(" %</p>");

            client.print("<p>Volume bad: ");
            client.print(volumeResultBathWeb);
            client.println(" L</p>");
          }
        
          if(distanceBathWeb == 0)
          {
            client.print("<p>Niveau bad: ");
            client.println(" FOUT (Sensor is niet actief)</p>");

            client.print("<p>Volume bad: ");
            client.println(" FOUT (Sensor is niet actief)</p>");
          }
          
          // SSR:
          client.print("<p>SSR: ");
          client.print(pulseState);
          client.println("</p>");

          // Consumption:
          client.print("<p>Consumtpion: "); 
          client.println(liters);
          client.print(" L</p>");
          
          client.println("</body>");
          client.println("</html>");
          break;
        }
      }
    delay(100);
    client.stop();
}
//--------------------------------------------------------------
