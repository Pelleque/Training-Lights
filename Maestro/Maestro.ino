#include <SPI.h>
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include <ESP32Encoder.h>
ESP32Encoder encoder;
int c = 0;
int c_old = 1;
#define boton 12
int i;
int t_op;
String menu[] = {"CERO", "UNO", "DOS", "TRES", "CUATRO"};
#define t_menu 4 //tamaño del array menu
String op0[] = {"OP0", "OP1", "OP2", "OP3"};
#define t_op0 3


// Comunicación
uint8_t broadcastAddress[] = {0x7C, 0x9E, 0xBD, 0xF7, 0x0E, 0xD0};
typedef struct test_struct {
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
  //Serial.println(recibir.texto);
  digitalWrite(LED_BUILTIN, HIGH);
}


void setup(){
  pinMode(boton, INPUT_PULLUP);
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(1000);
  
  ESP32Encoder::useInternalWeakPullResistors=UP;
  encoder.attachHalfQuad(14, 13);
  encoder.setCount(0);
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("MENU");
  display.display();


  // Comunicación
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
  //recibir.texto = "";
  esp_now_register_recv_cb(OnDataRecv);
}

void loop(){
  delay(100);
  c = (int32_t)encoder.getCount()/2;
  if(c<0) {
    c = 0;
    encoder.setCount(0);
  } else if(c > t_menu) {
    c = t_menu;
    encoder.setCount(t_menu*2);
  }
  if(c != c_old) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("MENU");
    display.setTextSize(3);
    display.setCursor(0, 8);
    display.println(menu[c]);
    display.display();
    c_old = c;
  }
  if(digitalRead(boton)==false) {
    delay(100);
    switch(c) {
      case 0: // ############### OPERACION 0 ################
        i = 0; // num case
        t_op = t_op0; //tam op
        c = 0;
        c_old = 1;
        encoder.setCount(0);
        while(digitalRead(boton)==true) {
          c = (int32_t)encoder.getCount()/2;
          if(c<0) {
            c = 0;
            encoder.setCount(0);
          } else if(c > t_op) {
            c = t_op;
            encoder.setCount(t_op*2);
          }
          if(c != c_old) {
            display.clearDisplay();
            display.setTextColor(SSD1306_WHITE);
            display.setTextSize(1);
            display.setCursor(0, 0);
            display.println(menu[i]);
            display.setTextSize(3);
            display.setCursor(0, 8);
            display.println(op0[c]);
            display.display();
            c_old = c;
          }
        }
        break;
      case 1:
        enviar.tiempo = 5;
        enviar.R = 0;
        enviar.G = 0;
        enviar.B = 255;
        esp_err_t result;
        result = esp_now_send(broadcastAddress, (uint8_t *) &enviar, sizeof(test_struct));
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println(menu[i]);
        display.setTextSize(2);
        display.setCursor(0, 8);
        // envia
        break;
      case 2:
        i = 2; // num case
        c = 0;
        encoder.setCount(0);
        // envia
        break;
      case 3:
        i = 3; // num case
        c = 0;
        encoder.setCount(0);
        // envia
        break;
      
    }
  }
}
