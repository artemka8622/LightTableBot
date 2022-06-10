#define USE_CLIENTSSL false  
#include <AsyncTelegram2.h>

// Timezone definition
#include <time.h>
#define MYTZ "CET-1CEST,M3.5.0,M10.5.0/3"

#include <ESP8266WiFi.h>
BearSSL::WiFiClientSecure client;
BearSSL::Session   session;
BearSSL::X509List  certificate(telegram_cert);
AsyncTelegram2 myBot(client);

const char* ssid  =  "mynet3";     // SSID WiFi network
const char* pass  =  "utsenuta";     // Password  WiFi network
const char* token =  "887034298:AAH3DN8UHe99zCZk6bRtiGTYWzJtpFH3f6E";  // Telegram token
InlineKeyboard myInlineKbd1, myInlineKbd2; // inline keyboards object helper

#define LIGHT_ON_CALLBACK  "lightON"   // callback data sent when "LIGHT ON" button is pressed
#define LIGHT_OFF_CALLBACK "lightOFF"  // callback data sent when "LIGHT OFF" button is pressed
#define BUTTON1_CALLBACK   "Button1"   // callback data sent when "Button1" button is pressed
#define BUTTON2_CALLBACK   "Button2"   // callback data sent when "Button1" button is pressed
const uint8_t LED = 4;

// Callback functions definition for inline keyboard buttons
void onPressed(const TBMessage &queryMsg){
  digitalWrite(LED, HIGH);
  Serial.printf("\nON button pressed (callback);\nQueryId: %s\n\n", queryMsg.callbackQueryID);
  myBot.endQuery(queryMsg, "Light on", true);
}

void offPressed(const TBMessage &queryMsg){
  digitalWrite(LED, LOW);
  Serial.printf("\nOFF button pressed (callback); \nQueryId: %s\n\n", queryMsg.callbackQueryID);
  myBot.endQuery(queryMsg, "Light on", false);
}

void button1Pressed(const TBMessage &queryMsg){
  Serial.printf("\nButton 1 pressed (callback); \nQueryId: %s\n\n", queryMsg.callbackQueryID);
  myBot.endQuery(queryMsg, "You pressed Button 1", true);
}

void button2Pressed(const TBMessage &queryMsg){
  Serial.printf("\nButton 2 pressed (callback); \nQueryId: %s\n\n", queryMsg.callbackQueryID);
  myBot.endQuery(queryMsg, "You pressed Button 2", false);
}

void setup() {
  // initialize the Serial
  Serial.begin(115200);

  // connects to access point
  WiFi.setAutoConnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  delay(500);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  
  configTime(MYTZ, "time.google.com", "time.windows.com", "pool.ntp.org");
  client.setSession(&session);
  client.setTrustAnchors(&certificate);
  client.setBufferSizes(1024, 1024);
  myBot.setUpdateTime(1000);
  myBot.setTelegramToken(token);

  Serial.print("\nTest Telegram connection... ");
  myBot.begin() ? Serial.println("OK") : Serial.println("NOK");
  Serial.print("Bot name: @");
  Serial.println(myBot.getBotName());

  // Add sample inline keyboard
  // add a button that will turn on LED on pin assigned
  myInlineKbd1.addButton("ON",  LIGHT_ON_CALLBACK, KeyboardButtonQuery, onPressed);
  // add a button that will turn off LED on pin assigned
  myInlineKbd1.addButton("OFF", LIGHT_OFF_CALLBACK, KeyboardButtonQuery, offPressed);
  // add a new empty button row
  myInlineKbd1.addRow();
  // add a button that will open browser pointing to this GitHub repository
  myInlineKbd1.addButton("GitHub", "https://github.com/cotestatnt/AsyncTelegram2/", KeyboardButtonURL);
  Serial.printf("Added %d buttons to keyboard\n", myInlineKbd1.getButtonsNumber());
  
  // Add another inline keyboard
  myInlineKbd2.addButton("Button 1", BUTTON1_CALLBACK, KeyboardButtonQuery, button1Pressed);
  myInlineKbd2.addButton("Button 2", BUTTON2_CALLBACK, KeyboardButtonQuery, button2Pressed);
  Serial.printf("Added %d buttons to keyboard\n", myInlineKbd2.getButtonsNumber());
  
  // Add pointer to this keyboard to bot (in order to run callback function)
  myBot.addInlineKeyboard(&myInlineKbd1);
  myBot.addInlineKeyboard(&myInlineKbd2);
}



void loop() {

  // In the meantime LED_BUILTIN will blink with a fixed frequency
  // to evaluate async and non-blocking working of library
  static uint32_t ledTime = millis();
  if (millis() - ledTime > 200) {
    ledTime = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  printHeapStats();  
  // a variable to store telegram message data
  TBMessage msg;
  // if there is an incoming message...
  if (myBot.getNewMessage(msg)) {
    // check what kind of message I received
    String tgReply;
    MessageType msgType = msg.messageType;
    
    switch (msgType) {
      case MessageText :
        // received a text message
        tgReply = msg.text;
        Serial.print("\nText message received: ");
        Serial.println(tgReply);

        if (tgReply.equalsIgnoreCase("/inline_keyboard1")) {          
          myBot.sendMessage(msg, "This is inline keyboard 1:", myInlineKbd1);          
        }        
        else if (tgReply.equalsIgnoreCase("/inline_keyboard2")) {          
          myBot.sendMessage(msg, "This is inline keyboard 2:", myInlineKbd2);          
        } 
        else {
          // write back feedback message and show a hint
          String text = "You write: \"";
          text += msg.text;
          text += "\"\nTry /inline_keyboard1 or /inline_keyboard2";
          myBot.sendMessage(msg, text);
        }
        break;
        case MessageQuery:
          break;        
        default:
          break;
    }
  }
}


void printHeapStats() {
  time_t now = time(nullptr);
  struct tm tInfo = *localtime(&now);
  static uint32_t infoTime;
  if (millis() - infoTime > 10000) {
    infoTime = millis();
    uint32_t free;
    uint16_t max;
    ESP.getHeapStats(&free, &max, nullptr);
    Serial.printf("\nTotal free: %5d - Max block: %5d", free, max);
  }
}
