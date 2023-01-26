/*
 * Envío de información de Arduino por MQTT
 * por: Jesús Adrián
 * Fecha:25/01/2023
 * 
 * Este código realiza una lectura de un arduino comunicado 
 * por Rx y Tx de forma serial, acumula byte por byte hasta 
 * un salto de linea, posteriormente envia estos datos por MQTT 
 * a un tema el cuál será procesado en node-red y serán 
 * representados gráficamente.
 * 
 * 
 * Componente     PinESP32CAM     Estados lógicos
 * ledStatus------GPIO 33---------On=>LOW, Off=>HIGH
 * ledFlash-------GPIO 4----------On=>HIGH, Off=>LOW
 * 
 * Arduino   ESP32CAM
 * Rx--------GPIO 12
 * Tx--------GPIO 13
 * GND-------GND
 */

//Bibliotecas
#include <WiFi.h>  // Biblioteca para el control de WiFi
#include <PubSubClient.h> //Biblioteca para conexion MQTT
//Constantes
#define RXp2 13
#define TXp2 12

//Datos de WiFi
const char* ssid = "Totalplay-8CA5";  // Aquí debes poner el nombre de tu red
const char* password = "8CA549F8V2WtX8M3";  // Aquí debes poner la contraseña de tu red

//Datos del broker MQTT son los datos de mi IP, porque es un broker local (localhost) 
const char* mqtt_server = "18.198.34.5"; // Si estas en una red local, coloca la IP asignada, en caso contrario, coloca la IP publica
IPAddress server(18,198,34,5);

// Objetos
WiFiClient espClient; // Este objeto maneja los datos de conexion WiFi
PubSubClient client(espClient); // Este objeto maneja los datos de conexion al broker

// Variables
int flashLedPin = 4;  // Para indicar el estatus de conexión
int statusLedPin = 33; // Para ser controlado por MQTT
long timeNow, timeLast; // Variables de control de tiempo no bloqueante
int data = 0; // Contador
int wait = 5000;  // Indica la espera cada 5 segundos para envío de mensajes MQTT
char charBuf[100];
String inputString = "";        // a String to hold incoming data
bool stringComplete = false;  


void serialEvent2() {
                while (Serial2.available()) {
                  // get the new byte:
                  char inChar = (char)Serial2.read();
                  // add it to the inputString:
                  inputString += inChar;
                  //Serial.print(inputString);
                  // if the incoming character is a newline, set a flag so the main loop can
                  // do something about it:
                  if (inChar == '\n') {
                    stringComplete = true;
                  }}}


// Inicialización del programa
void setup() {
  // Iniciar comunicación serial
  Serial.begin (115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

  Serial.println ("Programa iniciado");

  //
  pinMode (flashLedPin, OUTPUT);
  pinMode (statusLedPin, OUTPUT);
  digitalWrite (flashLedPin, LOW);
  digitalWrite (statusLedPin, HIGH);

  Serial.println();
  Serial.println();
  Serial.print("Conectar a ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password); // Esta es la función que realiz la conexión a WiFi
 
  while (WiFi.status() != WL_CONNECTED) { // Este bucle espera a que se realice la conexión
    digitalWrite (statusLedPin, HIGH);
    delay(500); //dado que es de suma importancia esperar a la conexión, debe usarse espera bloqueante
    digitalWrite (statusLedPin, LOW);
    Serial.print(".");  // Indicador de progreso
    delay (5);
  }
  
  // Cuando se haya logrado la conexión, el programa avanzará, por lo tanto, puede informarse lo siguiente
  Serial.println();
  Serial.println("WiFi conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());

  // Si se logro la conexión, encender led
  if (WiFi.status () > 0){
  digitalWrite (statusLedPin, LOW);
  }
  
  delay (1000); // Esta espera es solo una formalidad antes de iniciar la comunicación con el broker

  // Conexión con el broker MQTT
  client.setServer(server, 1883); // Conectarse a la IP del broker en el puerto indicado
//  client.setCallback(callback); // Activar función de CallBack, permite recibir mensajes MQTT y ejecutar funciones a partir de ellos
  delay(1500);  // Esta espera es preventiva, espera a la conexión para no perder información
  
  timeLast = millis (); // Inicia el control de tiempo
}// fin del void setup ()


                  
// Cuerpo del programa, bucle principal
void loop() {

    //Verificar siempre que haya conexión al broker
  if (!client.connected()) {
    reconnect();  // En caso de que no haya conexión, ejecutar la función de reconexión, definida despues del void setup ()
  }// fin del if (!client.connected())
  
  client.loop(); // Esta función es muy importante, ejecuta de manera no bloqueante las funciones necesarias para la comunicación con el broker
         
  if (stringComplete) {
    //Serial.println(inputString);
    String json = inputString;
    delay(1000);
    Serial.println(json); // Se imprime en monitor solo para poder visualizar que el string esta correctamente creado
    json.toCharArray(charBuf, 100);
    client.publish("codigoIoT/colector/sensores", charBuf);
    // clear the string:
    inputString = "";
    stringComplete = false;}
   
}// fin del void loop ()

// Función para reconectarse
void reconnect() {
  // Bucle hasta lograr conexión
  while (!client.connected()) { // Pregunta si hay conexión
    Serial.print("Tratando de contectarse...");
    // Intentar reconexión
    if (client.connect("ESP32CAMClient")) { //Pregunta por el resultado del intento de conexión
      Serial.println("Conectado");
      client.subscribe("codigoIoT/ejemplo/mqttin"); // Esta función realiza la suscripción al tema
    }// fin del  if (client.connect("ESP32CAMClient"))
    else {  //en caso de que la conexión no se logre
      Serial.print("Conexion fallida, Error rc=");
      Serial.print(client.state()); // Muestra el codigo de error
      Serial.println(" Volviendo a intentar en 5 segundos");
      // Espera de 5 segundos bloqueante
      delay(5000);
      Serial.println (client.connected ()); // Muestra estatus de conexión
    }// fin del else
  }// fin del bucle while (!client.connected())
}// fin de void reconnect(
