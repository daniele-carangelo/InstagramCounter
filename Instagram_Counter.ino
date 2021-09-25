#include <EEPROM.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Arduino_JSON.h>



#define NUM_MAX 5

#define DIN_PIN 15  // D8
#define CS_PIN  13  // D7
#define CLK_PIN 12  // D6
#include "max7219_hr.h"
#include "fonts.h"

WiFiClientSecure client; //For ESP8266 boards
HTTPClient http;
String host = "https://counts.live/api/instagram-follower-count/";
const int port = 443;
char username_insta[30];
JSONVar myObject;
char eeprom_user[30];
int last_counter;
int number;

void setup() {
  Serial.begin(115200);
  Serial.println("Start...");

  //MAX7219
  initMAX7219();
  sendCmdAll(CMD_SHUTDOWN,1);
  sendCmdAll(CMD_INTENSITY,0);
  
  WiFiManager wifiManager; //object WM
  username_insta[0] = 0;
  //wifiManager.resetSettings();
  //Custom parameter for Intagram ID
  WiFiManagerParameter custom_instagram_id("username","Nome Utente Instagram",username_insta,40);
  wifiManager.addParameter(&custom_instagram_id);

  
  
  //WifiManager
  bool res;
  //create AP and test 
  res = wifiManager.autoConnect("Instagram_Counter", "12345678");
  printStringWithShift("Connecting ",16);
  //EEPROM Load
  EEPROM.begin(512);
  EEPROM.get(0,eeprom_user);
  Serial.println("Settings loaded");
  
  if(!res){
  Serial.println("Connected Failed");
  }
  else{
  Serial.println("Connected to WI-FI");
  //Serial.print("local ip :");
  //Serial.println(WiFi.localIP());
   strcpy(username_insta,custom_instagram_id.getValue());
   if(strlen(username_insta) == 0){
      Serial.println("load to eeprom");
      Serial.println(eeprom_user);      
   }
   else{
      Serial.println("put in eeprom");
      Serial.println(username_insta);
/*
      for (int i = 0; i < usr_dest.length(); ++i) {
           Serial.println(usr_dest[i]);
           EEPROM.write(i, usr_dest[i]);
          }

        EEPROM.write(usr_dest.length(), '\0');
        EEPROM.commit();
      
      */
      EEPROM.put(0,username_insta);
        if (EEPROM.commit()) {
            Serial.println("Settings saved");
            strcpy(eeprom_user,username_insta);
        } else {
            Serial.println("EEPROM error");
        }
      
   }
  }
  host += eeprom_user;
  host += "/live";
  Serial.println(host);
}
void loop() {
  

  /*
  HTTPClient http;
  http.begin("http://ergast.com/api/f1/2004/1/results.json");  // request destination
    int httpCode = http.GET();                                  //Send the request
    Serial.println(httpCode);
    if (httpCode > 0) { //Check the returning code
 
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);             //Print the response payload
 
    }
 
    http.end();   //Close connection
 */

  //concat Host
  
 
  client.setInsecure();
  client.connect(host, port);
  http.begin(client,host);
  String payload;
  if(http.GET() == HTTP_CODE_OK){
   payload =  http.getString();
  }
  else{
    Serial.println("FAILED");
  }

/*
  JsonObject& parsed= JSONBuffer.parseObject(payload);
  if (!parsed.success()) {
      Serial.println("Parsing failed");
      delay(5000);
      return;
}
  String test = parsed["data"]["followers"];
  Serial.println(test);
 */

 /*
  StaticJsonDocument<256> doc;

  DeserializationError error = deserializeJson(doc, payload);
  
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  
  bool success = doc["success"]; // true
  const char* service = doc["service"]; // "instagram-follower-count"
  long long t = doc["t"]; // 1631523967711
  
  JsonObject data = doc["data"];
  const char* data_lv_identifier = data["lv_identifier"]; // "cristiano"
  long data_followers = data["followers"]; // 342577671
  int data_following = data["following"]; // 480
  int data_posts = data["posts"]; // 3141
  */

  myObject = JSON.parse(payload);

  // JSON.typeof(jsonVar) can be used to get the type of the variable
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }
  if(myObject["success"]){
  Serial.print("Followers di: ");
  Serial.println(eeprom_user);
  number = myObject["data"]["followers"];
  Serial.println(number);
  }
  else{
    Serial.println("username errato");
  }
  char temp[12];
  itoa(number,temp,10);
  if(last_counter != number){
      clr();
      printStringWithShift(temp,16);
  }

  
  last_counter = number;
  delay(10000); //every 10 seconds

}

int showChar(char ch, const uint8_t *data)
{
  int len = pgm_read_byte(data);
  int i,w = pgm_read_byte(data + 1 + ch * len);
  for (i = 0; i < w; i++)
    scr[NUM_MAX*8 + i] = pgm_read_byte(data + 1 + ch * len + 1 + i);
  scr[NUM_MAX*8 + i] = 0;
  return w;
}

void printCharWithShift(unsigned char c, int shiftDelay) {
  
  if (c < ' ' || c > '~'+25) return;
  c -= 32;
  int w = showChar(c, font);
  for (int i=0; i<w+1; i++) {
    delay(shiftDelay);
    scrollLeft();
    refreshAll();
  }
}

void printStringWithShift(const char* s, int shiftDelay){
  while (*s) {
    printCharWithShift(*s, shiftDelay);
    s++;
  }
}
