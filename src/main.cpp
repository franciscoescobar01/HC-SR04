#import <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Ethernet.h>
#include <SPI.h>


//Constantes de coneccion a WiFi
const char* ssid = "CEISUFRO";
const char* password = "DCI.2016";

//Constantes de servidor MQTT
const char* mqtt_server = "ipame.cl";
const char* mqtt_topic = "ultrasonido";


#define echoPin D7 // Echo Pin
#define trigPin D6 // Trigger Pin

long duration, distance; // Duration used to calculate distance

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  delay(100);
  Serial.print("Conectando a: ");
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("Coneccion WiFi realizada");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("Command is : [");
  Serial.print(topic);
  int p =(char)payload[0]-'0';
} //end callback

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
     //once connected to MQTT broker, subscribe command if any
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 6 seconds before retrying
      delay(6000);
    }
  }
} //end reconnect()

void setup()
{
Serial.begin (115200);
pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT);

setup_wifi();
client.setServer(mqtt_server, 1883);
client.setCallback(callback);
Serial.println("Setup!");
}

void loop()
{
  char message[58];
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();

/* The following trigPin/echoPin cycle is used to determine the
distance of the nearest object by bouncing soundwaves off of it. */
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
duration = pulseIn(echoPin, HIGH);
//Calculate the distance (in cm) based on the speed of sound.
distance = duration/58.2;
Serial.println(distance);
//Delay 50ms before next reading.

// read DHT11 sensor every 6 seconds
if (now - lastMsg > 6000) {



  lastMsg = now;

  String msg=""+distance;


  msg.toCharArray(message,58);
  Serial.println(message);
   //publish sensor data to MQTT broker
  client.publish(mqtt_topic, message);
  //webservice datos unidad_medida_1=Temperatura+C&dato_1=90&unidad_medida_2=Humedad++%25&dato_2=55&codigo_sensor=Humedad+y+Temperatura&accion=agregar
  String data = "unidad_medida_1=Distancia+C&dato_1=";
  data = data + distance;
  data = data + "&codigo_sensor=Ultrasonido&accion=agregar";

  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status

  HTTPClient http;    //Declare object of class HTTPClient

  http.begin("http://praga.ceisufro.cl/ipame_dev/index.php?eID=obtenerDatos");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(data);   //Send the request
  String payload = http.getString();                  //Get the response payload

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();  //Close connection

  }else{

  Serial.println("Error in WiFi connection");
  }

  Serial.println(data);
  Serial.print("Response body from server: ");
  Serial.println(message);
}


}
