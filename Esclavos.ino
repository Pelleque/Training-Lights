
/*Montaje ESP32 + sensor MPU6050 + Anillo LED
 * Sensor   ESP32
 * VCC      3.3v
 * GND      GND   Anillo LED (GND)
 * SLC      D22
 * SDA      D21
 *          D15   Anillo LED (IN)
 *          Vin   Anillo LED (VCC)
 * instalar resitencias 330 Ohms entre Vcc y canales RX y TX
 */

#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>

const int MPU = 0x68; // MPU6050 I2C address
float AccX, AccY, AccZ;
unsigned long tiempo1; // tiempo de entrada en bucle
unsigned long tiempo2; //definicion de tiempo para comparar con el tiempo de entrada en bucle
unsigned long deltaTiempo; // diferencia entre tiempo 2 y tiempo 1 para gestionar bucle de leds

#define PIN            14  // define el pin de coxeion del anillo LED
#define NUMPIXELS      24 // numero de pixel del anillo
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int delayval = 250; // delay for half a second
int numero =0 ; // variable para recorrer los LEDs
int tiempoCiclo = 10; // tiempo de ciclo en segundos
int tiempoLed = tiempoCiclo*1000/NUMPIXELS;

int inicio = 0; // Definimos variable para ver estado del ciclo 0 parado, 1 en marcha, 2 conseguido, 3 tiempo agotado.


uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0x15, 0xC1, 0x18};
typedef struct test_struct { // estructura igual que la del maestro
  int tiempo;
  int R;
  int G;
  int B;
} test_struct;

test_struct enviar;
test_struct recibir;


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nUltimo envio de mensaje:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "NOK");
  digitalWrite(LED_BUILTIN, LOW);
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recibir, incomingData, sizeof(test_struct));
  Serial.print("Recibido: ");
  // Serial.println(recibir.texto);
  digitalWrite(LED_BUILTIN, HIGH);
}


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Peer NOK");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.begin(115200);
  Wire.begin();                      // Initialize comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission
   delay(20);

  pinMode(LED_BUILTIN, OUTPUT);

  recibir.tiempo = 0;
  
  int tiempo = 5;
  unsigned long t = millis();
  float medida_sensor = 0;
  for(int i = 0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0,0,255));
    pixels.show();
  }
  do {
    medida_sensor = sensorAc();
  } while((millis()-t)/1000 < tiempo && medida_sensor < 20);
  if (medida_sensor < 20) { 
    for(int i = 0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255,0,0));
      pixels.show();
    }
    delay(400);
  } else { 
    for(int i = 0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0,255,0));
      pixels.show();
    }
    delay(400);
  }
  for(int i = 0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    pixels.show();
  }
}


   
float sensorAc() {
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  AccX = (Wire.read() << 8 | Wire.read());
  AccY = (Wire.read() << 8 | Wire.read());
  AccZ = (Wire.read() << 8 | Wire.read());
 
  //double arg =AccX*AccY*AccZ/10000000; // Multiplico los valores de los 3 ejes 
  double arg =AccZ; // nos quedamos con el valor de Z*/
  double expO=2;    // elevamos al cuadrado para evitar valores negativos
  
  float valorA = pow(arg,expO)/100000000;    //dividimos el resultado para tener valores más bajos y controlables
  Serial.println(valorA);   // imprimimos valor por serial
  return valorA;
}

void anillo(int tiempo, int R, int G, int B) {
  unsigned long t = millis();
  float medida_sensor = 0;
  for(int i = 0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(R,G,B));
    pixels.show();
  }
  do {
    medida_sensor = sensorAc();
  } while((millis()-t)/1000 < tiempo && medida_sensor < 20);
  if (medida_sensor < 20) { 
    for(int i = 0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0,255,0));
      pixels.show();
    }
    delay(400);
  } else { 
    for(int i = 0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255,0,0));
      pixels.show();
    }
    delay(400);
  }
  for(int i = 0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    pixels.show();
  }
}

/*
void anillo(){
  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(255,0,0)); // Rojo inicial.
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(100); // Delay for a period of time (in milliseconds).
  }
  tiempo1=millis();   //Asignacion de valor de tiempo a variable
  tiempo2=millis();   //Asignacion de valor de tiempo a variable   
  do {
    // pixels.Co lor takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(numero, pixels.Color(0,0,255)); // LEds en azul.
    pixels.show(); // This sends the updated pixel color to the hardware.

    if (deltaTiempo < tiempoLed) {  // Bucle de comparción de tiempos para encender los leds del anillo
      tiempo2=millis();
      deltaTiempo = tiempo2 - tiempo1;
      float medida_sensor = sensorAc(); //invoca a la función de lectura del sensor
      if (medida_sensor>20) { // si la medida del sensor es menor que 100
        Serial.print("Conseguido en (s):....   ");
        Serial.println(numero*tiempoLed/1000);
        numero = 500; //asignamos un numero muy alto para que cumple la condicion de fin de ciclo por exito
        delay(5000);
      }
    } else {// si se ha cumplido el tiempom de un led, actualizamos contadores de tiempo e incrementamos la variable numero
      tiempo1=millis();
      tiempo2=millis();
      deltaTiempo = tiempo2 - tiempo1;
      numero++;      
    }
  } while(numero < NUMPIXELS); //cierre del bucle Do...While
  
  if (numero = NUMPIXELS) { // se se agota el tiempo
    numero = 0;
    Serial.println("Tiempo agotado");
    apaga(); // invocamos funcion que apaga los leds
    inicio=3;
  }
  if (numero = 500) { //Tras ciclo conseguido reseteamos la variable numero y apagamos los leds
    numero = 0;
    Serial.println("nuevo ciclo");
    apaga();
   inicio=2;
  }
}
//for(int i=0;i<NUMPIXELS;i++)// apagado de LEDs
  {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(100); // Delay for a period of time (in milliseconds).
  }*/


void apaga()    //funcion para el apagado de los leds
{
  
  for(int i=0;i<NUMPIXELS;i++)
  {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(100); // Delay for a period of time (in milliseconds).
  }
}

void loop() {
 if(recibir.tiempo != 0) {
    anillo(recibir.tiempo, recibir.R, recibir.G, recibir.B);
    recibir.tiempo = 0;
 }
}
  
