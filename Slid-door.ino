#include <SPI.h>
#include <MFRC522.h>
//#include <AccelStepper.h>
#include <Stepper.h>
#include <map>

#define SS_PIN 15
#define RST_PIN 2
#define SS_PIN2 5
#define RST_PIN2 4


MFRC522 Enter_mfrc522(SS_PIN, RST_PIN);                 // Create MFRC522 instance.
MFRC522 Exit_mfrc522(SS_PIN2, RST_PIN2);                // Create MFRC522 instance.
const int stepsPerRevolution = 200;                     // จำนวนขั้นตอนต่อหนึ่งรอบ
Stepper myStepper(stepsPerRevolution, 27, 25, 26, 33);  // กำหนดขาที่ต่อกับ stepper motor  (stepsPerRevolution, IN1, IN3, IN2, IN1)

std::map<String, String> authorizedCards = {
  { "90 28 50 20", "Bunthita" },
  { "63 0A E2 1B", "Nutchanat" },
  { "F3 C7 BF 1D", "Ratthawat" },
  { "90 F3 46 20", "Anusorn" }
};



// Load Wi-Fi library
#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "CoolSpot";
const char* password = "CSIoT2023";

//Your Domain name with URL path or IP address with path
const char* enterService = "http://192.168.31.106:1880/enter";
const char* exitService = "http://192.168.31.106:1880/exit";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

#define ENTER 0
#define EXIT 1

int direction;


void setup() {
  Serial.begin(115200);      // Initiate a serial communication
  SPI.begin();               // Initiate  SPI bus
  Enter_mfrc522.PCD_Init();  // Initiate MFRC522
  Exit_mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  myStepper.setSpeed(110);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
void loop() {
  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {  // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");                                             // print a message out in the serial port
    String currentLine = "";                                                   // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        header += c;
        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on
            if (header.indexOf("GET /open") >= 0) {
              Serial.println("GPIO 2 on");
            }

            // Send JSON
            client.println("{\"status\":\"OK\"}");




            unsigned long startTime = millis();
            Serial.println("❎❎❎Opening❎❎❎");

            while (millis() - startTime < 10000) {  // ทำให้motorทำงานให้ครบตามเวลา
              myStepper.step(stepsPerRevolution);
            }

            delay(2000);
            startTime = millis();

            Serial.println("✅✅✅Closing✅✅✅");
            while (millis() - startTime < 10000) {
              myStepper.step(-stepsPerRevolution);
            }
            Serial.println("😎😎😎finished😎😎😎");

            // Break out of the while loop
            break;
          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }




  unsigned long startTime = millis();

  String uidContent = "";
  if (Enter_mfrc522.PICC_IsNewCardPresent() && Enter_mfrc522.PICC_ReadCardSerial()) {
    direction = ENTER;
    Serial.println("ENTER!!!");
    Serial.print("UID tag :");
    for (byte i = 0; i < Enter_mfrc522.uid.size; i++) {
      Serial.print(Enter_mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(Enter_mfrc522.uid.uidByte[i], HEX);
      uidContent.concat(String(Enter_mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      uidContent.concat(String(Enter_mfrc522.uid.uidByte[i], HEX));
    }
  } else if (Exit_mfrc522.PICC_IsNewCardPresent() && Exit_mfrc522.PICC_ReadCardSerial()) {
    direction = EXIT;
    Serial.println("EXIT!!!");
    Serial.print("UID tag :");
    for (byte i = 0; i < Exit_mfrc522.uid.size; i++) {
      Serial.print(Exit_mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(Exit_mfrc522.uid.uidByte[i], HEX);
      uidContent.concat(String(Exit_mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      uidContent.concat(String(Exit_mfrc522.uid.uidByte[i], HEX));
    }
  } else {
    return;
  }

  Serial.println();
  Serial.print("Message : ");
  uidContent.toUpperCase();

  auto it = authorizedCards.find(uidContent.substring(1));  //การค้นหาข้อมูลจากฟังก์ชัน map

  if (it != authorizedCards.end()) {  //ถ้าไม่เจอข้อมูลที่มีก็จะข้ามไปทำอีกฟังก์ชัน
    Serial.print("Card: ");
    Serial.println(it->second);  // แสดงค่าจากที่ตรงกับรหัสบัตร


    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient serviceNode;
      HTTPClient http;

      // Your Domain name with URL path or IP address with path
      http.begin(serviceNode, direction == ENTER ? enterService : exitService);

      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

      // Specify content-type header
      http.addHeader("Content-Type", "application/json");
      // Data to send with HTTP POST
      String httpRequestData = "{\"id\":\"" + uidContent.substring(1) + "\",\"name\":\"" + it->second + "\"}";
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

      // If you need an HTTP request with a content type: application/json, use the following:
      //http.addHeader("Content-Type", "application/json");
      //int httpResponseCode = http.POST("{\"api_key\":\"tPmAT5Ab3j7F9\",\"sensor\":\"BME280\",\"value1\":\"24.25\",\"value2\":\"49.54\",\"value3\":\"1005.14\"}");

      // If you need an HTTP request with a content type: text/plain
      //http.addHeader("Content-Type", "text/plain");
      //int httpResponseCode = http.POST("Hello, World!");

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      // Free resources
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    previousTime = millis();


    Serial.println("❎❎❎Opening❎❎❎");
    // Print the card information

    while (millis() - startTime < 10000) {  // ทำให้motorทำงานให้ครบตามเวลา
      myStepper.step(stepsPerRevolution);
    }

    delay(2000);
    startTime = millis();

    Serial.println("✅✅✅Closing✅✅✅");
    while (millis() - startTime < 10000) {
      myStepper.step(-stepsPerRevolution);
    }
    Serial.println("😎😎😎finished😎😎😎");
  }

  else {
    Serial.println("❗❗❗NEXT❗❗❗");
    delay(3000);
  }
}
