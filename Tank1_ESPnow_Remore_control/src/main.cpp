// TND - justy copied hre. "controller" sends the data, so it will be used for the remote control.
// ESP32 because we need 2 analog inputs for the joystick
#include "WiFi.h"
#include "esp_now.h"


#define MY_NAME         "TANK_REMOTE_CONTROL"
#define MY_ROLE         ESP_NOW_ROLE_CONTROLLER         // set the role of this device: CONTROLLER, SLAVE, COMBO
#define RECEIVER_ROLE   ESP_NOW_ROLE_SLAVE              // set the role of the receiver
#define WIFI_CHANNEL    1



//const int SW_pin = 2; // digital pin connected to switch output
const int X_pin = 34; // analog pin connected to X output
const int Y_pin = 35; // analog pin connected to Y output

uint8_t tank_rcv_Addr[] = {0x50, 0x02, 0x91, 0x69, 0x6F, 0x22};   // please update this with the MAC address of the receiver

struct __attribute__((packed)) dataPacket {
  int tank_command; // commands: 0-stop,1-move
  int X_value;
  int Y_value;
};

void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  if(transmissionStatus == 0) {
    Serial.println(" ... Data sent successfully");
  } else {
    Serial.print(" .. Error code: ");
    Serial.println(transmissionStatus);
  }
}


// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void setup() {
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(X_pin, INPUT);
  pinMode(Y_pin, INPUT);



  Serial.begin(9600);     // initialize serial por
  Serial.println();
  Serial.print("Initializing...");
  Serial.print(MY_NAME);
  Serial.print("    My MAC address is: ");
  Serial.print(WiFi.macAddress());

  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("    Error initializing ESP-NOW");
    return;
  }

  Serial.println("  .... init OK ....");
// Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);


  
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, tank_rcv_Addr, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
         
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
Serial.println("Initialized.");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  int rd_X = analogRead(X_pin);
  int rd_Y = analogRead(Y_pin);
  //int stam; // no need
  
  int map_X = map(rd_X, 0, 4096, -512, 512);
  int map_Y = map(rd_Y, 0, 4096, -512, 512);


  dataPacket packet;
  packet.X_value = map_X;
  packet.Y_value = map_Y;

  Serial.print("Sending:  ");
  Serial.print(rd_X);
  Serial.print(" /  ");
  Serial.print(rd_Y);
  Serial.print(" ...... ");
  Serial.print(map_X);
  Serial.print(" /  ");
  Serial.print(map_Y);
  Serial.print("  ");

  esp_now_send(tank_rcv_Addr, (uint8_t *) &packet, sizeof(packet));
  digitalWrite(LED_BUILTIN, LOW);
  delay(1400);
}
