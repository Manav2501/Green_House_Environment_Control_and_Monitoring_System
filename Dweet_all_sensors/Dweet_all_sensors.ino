#include <SPI.h>
#include <Ethernet.h>
#include <dht11.h>


byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(10,20,24,37);

// initialize the library instance:
EthernetClient client;

char srvr1[] = "www.dweet.io";

unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 1000L; // delay between updates, in milliseconds
// the "L" is needed to use long type numbers


const size_t maxSize = 512;
char response[512];
int LDRpin = A0;        // input pin A0 to LDR
#define DHT11PIN A1  // input pin A1 to DHT11
int SoilMoisturepin = A3;  // input pin A3 to Soil moisture

int LDRValue = 0;// variable to store the value coming from the LDR
int AirQualitySensorValue = 0;

dht11 DHT11;

void setup() {
  // start serial port:
  Serial.begin(9600);
  
  Serial.println("--- Start ---");
  
  // give the ethernet module time to boot up:
  delay(1000);
  // start the Ethernet connection using a fixed IP address and DNS server:
  //Ethernet.begin(mac, ip); 
  Serial.println(Ethernet.begin(mac));
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
 
}

void loop() {
  //sensorValue = analogRead(pin);
  //Serial.println(sensorValue);
 httpRequest();
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  /* LDR */
  LDRValue = analogRead(LDRpin); // read analog input of LDR *need to map between 0-255*
  Serial.print("LDRValue : ");
  Serial.println(LDRValue);   //// printing value of LDR
  /* DHT11 */
  int chk = DHT11.read(DHT11PIN); // read analog input of temp and humidity
  Serial.print("Humidity (%): ");
  Serial.println((float)DHT11.humidity, 2);

  Serial.print("Temperature (C): ");
  Serial.println((float)DHT11.temperature, 2);  // printing value of temp and humidity
  /* MQ135 */
  AirQualitySensorValue = analogRead(A2);       // read analog input pin A2
  Serial.print("AirQua= ");
  Serial.print(AirQualitySensorValue, DEC);               // prints the value read
  Serial.println(" PPM");
  /* Soil Moisture Sensor */
  int moisture_percentage;
  int SoilMoistAnalog;
  SoilMoistAnalog = analogRead(SoilMoisturepin);   // read analog input of Soil moisture
  moisture_percentage = ( 100 - ( (SoilMoistAnalog/1023.00) * 100 ) );
  Serial.print("Moisture Percentage = ");
  Serial.print(moisture_percentage);
  Serial.print("%\n\n");
  Serial.print(SoilMoistAnalog);    // prints the value read
  Serial.print("\n\n");

  /* HTTP Request */
    client.stop();
  if (client.connect(srvr1, 80)) {
    Serial.println("connected");
    String s ="POST /dweet/for/arduinotest?temp="+String((int)DHT11.temperature)+"&ldr="+String(LDRValue)+"&humid="+
              String((int)DHT11.humidity)+"&soil="+String(moisture_percentage)+"&aqi="+String(AirQualitySensorValue);

    Serial.println(s);
    client.println(s);
    
    client.println("Host: www.dweet.io");
    delay(5000);
    size_t len = client.readBytes(response, 512);
    response[len] = 0;
    client.println("Connection: close");
    client.println();
    Serial.println(response);
  }
  else
    Serial.println("connection failed");
}
