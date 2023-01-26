/*Colector pluvial inteligente
Equipo 23
Joel Reyna
Jose Juan
Jesús Adrián
  Este código de arduino lee 3 sensores, un sensor ultrasónico para medir el nivel 
del tanque de agua, un sensor de potencial de hidrógeno para medir la acidez y 
un sensor de flujo.
  A partir de estos se modela un sistema de control con 2 actuadores, una electrovalvula 
y una bomba.
  Posteriormente se prepara un mensaje que será transmitido de forma serial para su 
envío por MQTT por un esp32-CAM.
*/


const int ph = A3;  //Pin analogico 3 para el sensor de ph
const int Echo = 8;   //Pin digital 3 para el Echo del sensor
const int Trigger = 9;   //Pin digital 2 para el Trigger del sensor
int valvula = 10;
int bomba = 11;
volatile int Fan;  
int Calc;
int flujo = 2;  //Pin digital 2 para el sensor de flujo
const int interruptor = 4;

const unsigned long INTERVALO_json = 5000UL;
unsigned long evento_json;

void rpm ()
{
Fan++;
}

void setup() {
   Serial.begin(9600);
pinMode(flujo, INPUT);
pinMode(valvula, OUTPUT);  
pinMode(bomba, OUTPUT);  
pinMode(interruptor, INPUT);
pinMode(Trigger, OUTPUT); //pin como salida
pinMode(Echo, INPUT);  //pin como entrada
digitalWrite(valvula, HIGH);
digitalWrite(bomba, HIGH); 
digitalWrite(Trigger, LOW);//Inicializamos el pin con 0  
attachInterrupt(0, rpm, RISING); // inicializamos las interupciones            
}


void loop() {    
  unsigned long actual = millis(); 
long t; //timepo que demora en llegar el eco
long d; //distancia en centimetros 
long p;
float prom_ph = 0;

Fan = 0;
sei();
delay (1000);
cli();
Calc = (Fan * 60 / 5.5);
// Serial.print (" L/m: ");
// Serial.print (Calc, DEC);

digitalWrite(Trigger, HIGH);
delayMicroseconds(10);          //Enviamos un pulso de 10us
digitalWrite(Trigger, LOW);
t = pulseIn(Echo, HIGH); //obtenemos el ancho del pulso
d = t / 59;//escalamosel tiempo a una distancia en cm
p= 100 - ((d-2)*6.66);
// Serial.print(" d: ");
// Serial.print(d);
// Serial.print(" %: ");
// Serial.print(p);

for ( int i = 0 ; i < 100 ; i++ ){
prom_ph += analogRead(ph);}
prom_ph = prom_ph / 100;
float phVol=(float)prom_ph*5.0/1024/6;
float phValue = -5.70 * phVol + 9.77;
// Serial.print(" ph: ");
// Serial.println(phValue);

int modo=digitalRead(interruptor);
//Serial.println(modo);
//**************json*************//
if( actual > evento_json ){
Serial.print("{\"ph\":");
Serial.print(phValue);
Serial.print(",");
Serial.print("\"caudal\":");
Serial.print(Calc, DEC);
Serial.print(",");
Serial.print("\"control\":");
Serial.print(modo);
Serial.print(",");
Serial.print("\"nivel\":");
Serial.print(p);
Serial.println("}");
evento_json += INTERVALO_json;
}

if(modo==HIGH){
if(phValue>6.5){
      if(d>3){
        digitalWrite(valvula,LOW);
        digitalWrite(bomba,LOW);
        }
      else{
        digitalWrite(valvula,HIGH);
        digitalWrite(bomba,HIGH);}
      }
  else{
      digitalWrite(valvula,HIGH);
      digitalWrite(bomba,HIGH);}
}
  else{
      digitalWrite(valvula,HIGH);
      digitalWrite(bomba,HIGH);} 

}