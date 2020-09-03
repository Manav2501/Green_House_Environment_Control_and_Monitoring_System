#include <SPI.h>
#include <Ethernet.h>
#include <dht11.h>


byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(10,20,12,47);

// initialize the library instance:
EthernetClient client;

char server[] = "10.1.22.74";
const size_t maxSize = 512;
char response[512];
int LDRpin = A0;        // input pin A0 to LDR
#define DHT11PIN A1  // input pin A1 to DHT11
int SoilMoisturepin = A3;  // input pin A3 to Soil moisture

int LDRValue = 0;// variable to store the value coming from the LDR
int AirQualitySensorValue = 0;
int HumidValue = 0;
int TempValue = 0;
int moisture_percentage = 0;
int SoilMoistAnalog = 0;

dht11 DHT11;

String rcv="";
int Rcvflg = 5; 

int TempRcv = 30;
int HumidRcv = 75;
int LDRRcv = 300;
int SoilRcv = 78;
int AqiRcv = 700; 

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

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
 
}

void loop() {
  
  /* LDR */
  LDRValue = analogRead(LDRpin); // read analog input of LDR 
  
  /* DHT11 */
  int chk = DHT11.read(DHT11PIN); // read analog input of temp and humidity
  HumidValue = (int)DHT11.humidity;
  
  TempValue = (int)DHT11.temperature;
   
  /* MQ135 */
  AirQualitySensorValue = analogRead(A2);       // read analog input pin A2
  
  /* Soil Moisture Sensor */
  SoilMoistAnalog = analogRead(SoilMoisturepin);   // read analog input of Soil moisture
  moisture_percentage = ( 100 - ( (SoilMoistAnalog/1023.00) * 100 ) );

  httpSendRequest();
  
  if(Rcvflg == 5){
    //httpReceiveRequest();
    Rcvflg = 0;
  }
  if(Rcvflg>9){
    Rcvflg = 0;
  }
  Rcvflg++;

  if(TempValue > TempRcv||1){  // with temprature data cooling system(motor) will On/Off
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
    Serial.println("Cooling System(motor) is ON");
    delay(2000);
  }
  else{
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
    Serial.println("Cooling System(motor) is OFF");
  }

  if(LDRValue > LDRRcv){  // With LDR Value Lighting System (LED) Will Operate
    digitalWrite(13, HIGH);
    Serial.println("Lighting System (LED) is ON");
    delay(2000);
  }
  else{
    digitalWrite(13, LOW);
    Serial.println("Lighting System (LED) is OFF");
  }

  if(moisture_percentage < SoilRcv || 1){  // with Soil Moisture data Water pump(motor) will On/Off
    digitalWrite(6, HIGH);
    digitalWrite(7, LOW);
    Serial.println("Water pump(motor) is ON");
    delay(2000);
  }
  else{
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    Serial.println("Water pump(motor) is OFF");
  }

  if(HumidValue > HumidRcv || 1){  // with Humidity Sensor data Dehumidifier(motor) will On/Off
    digitalWrite(8, HIGH);
    digitalWrite(9, LOW);
    Serial.println("Dehumidifier(motor) is ON");
    delay(2000);
  }
  else{
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
    Serial.println("Dehumidifier(motor) is OFF");
  }

  if(AirQualitySensorValue > AqiRcv){  // With MQ135 sensor Value Air purifier System (LED) Will Operate
    digitalWrite(12, HIGH);
    Serial.println("Air purifier System (LED) is ON");
    delay(2000);
  }
  else{
    digitalWrite(12, LOW);
    Serial.println("Air purifier System (LED) is OFF");
  }
}

// this function Send data to HTTP connection to the server:
void httpSendRequest() {

  /* HTTP Request */
    client.stop();
  if (client.connect(server, 80)) {
    Serial.println("connected");
    String s ="GET /send.php?temp="+String(TempValue)+"&ldr="+String(LDRValue)+"&humid="+
              String(HumidValue)+"&soil="+String(moisture_percentage)+"&aqi="+String(AirQualitySensorValue);
    Serial.println(s);
    client.println(s);
    
    client.println("Host: 10.1.22.74");
    delay(1000);
    size_t len = client.readBytes(response, 512);
    response[len] = 0;
    client.println("Connection: close");
    client.println();
    Serial.println(response);
  }
  else
    Serial.println("connection failed");
}

// this function Receive data to HTTP connection from the server:
void httpReceiveRequest()
{
  if (client.connect(server, 80)) 
  {
    Serial.println("Connection established 1");
    client.println("GET /receive.php");
    
    client.println("Host: 10.1.22.74");
    delay(1000);
    unsigned long timeout = millis();
    while (client.available() == 0) 
    {
      if (millis() - timeout > 2500) //If nothing is available on server for 25 seconds, close the connection.
      { 
        return;
      }
    }
    while(client.available())
    {
      String line = client.readStringUntil('\r'); //Read the server response line by line..
      rcv+=line; //And store it in rcv.
    }
    client.println("\r\nConnection: close\r\n\r\n"); //GET request for server response.
    client.stop(); // Close the connection.
  }
  else
  {
    Serial.println("Connection failed 1");
  }
    
  String s2=rcv.substring((rcv.indexOf('[')),rcv.indexOf(']')); // Extract the line returned by JSON object.

  int l=s2.length();
  String s="";
  String s1="";
  int m,i=0;
  while(i<l)
  {
    m=s2.indexOf('"',i+1);
    if(m!=-1)
    {
      for(int j=m+1;j<l;j++)
      {
        if(s2.charAt(j)!='"')
        {
          s+=s2.charAt(j);
        }
        else
        {
          i=j;
          break;
        }
      }
      if(s=="temp" || s=="humid" || s=="soil" || s=="aqi" || s=="ldr") {
        s1 = s;
        s = "";
        continue;
      }
      if(s1=="temp") {
        Serial.print("temp : " + s + " ");
        TempRcv = s.toInt();
        Serial.println(TempRcv);
      }
      else if(s1=="humid") {
        Serial.print("humid : " + s + " ");  
        HumidRcv = s.toInt(); 
        Serial.println(HumidRcv);     
      }
      else if(s1=="soil") {
        Serial.print("soil : " + s + " "); 
        SoilRcv = s.toInt();  
        Serial.println(SoilRcv);     
      }
      else if(s1=="aqi") {
        Serial.print("aqi : " + s + " "); 
        AqiRcv = s.toInt();
        Serial.println(AqiRcv);       
      }
      else if(s1=="ldr") {
        Serial.print("ldr : " + s + " ");  
        LDRRcv = s.toInt();
        Serial.println(LDRRcv);      
      }      
      s="";
    }
    else {
      break;
    }
  }
}
