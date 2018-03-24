#include <ArduinoJson.h>//JSON Communication made easy
#include <BoodskapCommunicator.h>//Platform Interaction
#include <ESP8266HTTPClient.h>//website communication(GET request method)

#define CONFIG_SIZE 512
#define REPORT_INTERVAL 15000
#define MESSAGE_ID 1000 //Message defined in the platform
//WiFi Credentials
# define ssid "123456789" 
# define password "*************"
String temp;
String payload="";

//Platform Credentials
#define DEF_DOMAIN_KEY "hgjfghfgvbvc" //your DOMAIN Key
#define DEF_API_KEY "jhjhghghgfgftg" //Your API Key
#define DEF_DEVICE_MODEL "BSKP-SMAG" //Your device model(BoodSkap-SmartAgriculture)
#define DEF_FIRMWARE_VER "N/A" //Your firmware version

//Declaring URLS for fetching climatic data
# define a "http://api.thingspeak.com/apps/thinghttp/send_request?api_key=TC1ZSTJ3EU9CCDSE"
#define b "http://api.thingspeak.com/apps/thinghttp/send_request?api_key=FBU4FGKMDKI9WLOO"
float t=0,h=0;
//Mode of Data Transfer(Protocol Specification)
BoodskapTransceiver Boodskap(UDP); //MQTT, UDP, HTTP
uint32_t lastReport = 0;

void launch(); //Defined later
void httpcall(String x){
   HTTPClient http;
   
   http.begin(x);

   int httpCode = http.GET();
   if(httpCode<0)Serial.println("Error Connecting to URL");
   
   payload = http.getString();
   Serial.print("Recieved this string-");
   Serial.println(payload);
   
   http.end();
  }
//Setup begins here
void setup() {
  Serial.begin(115200);
  //Initialising Pins for irrigation valves
  pinMode(D5,OUTPUT);
  pinMode(D6,OUTPUT);
  pinMode(D7,OUTPUT);
  
  StaticJsonBuffer<CONFIG_SIZE> buffer;
  JsonObject &config = buffer.createObject();

  config["ssid"] = ssid;
  config["psk"] = password;
  config["domain_key"] = DEF_DOMAIN_KEY;
  config["api_key"] = DEF_API_KEY;
  config["dev_model"] = DEF_DEVICE_MODEL;
  config["fw_ver"] = DEF_FIRMWARE_VER;
  config["dev_id"] = String("ESP8266-") + String(ESP.getChipId()); //Your unique device ID

  config["api_path"] = "https://api.boodskap.io"; //HTTP API Base Path Endpoint
  config["api_fp"] = "B9:01:85:CE:E3:48:5F:5E:E1:19:74:CC:47:A1:4A:63:26:B4:CB:32"; //In case of HTTPS enter your server fingerprint (https://www.grc.com/fingerprints.htm)
  config["udp_host"] = "udp.boodskap.io"; //UDP Server IP
  config["udp_port"] = 5555; //UDP Server Port
  config["mqtt_host"] = "mqtt.boodskap.io"; //MQTT Server IP
  config["mqtt_port"] = 1883; //MQTT Server Port
  config["heartbeat"] = 45; //seconds

  Boodskap.setup(config);
  temp=b;

}

void loop() {
  if(temp==a){
    temp=b;  
  }
  else{
    temp=a;
    }
  Boodskap.loop();
  if ((millis() - lastReport) >= REPORT_INTERVAL) {
    
    launch();
    lastReport = millis();
  }


}

void launch(){

 //http request
  httpcall(temp);
 

  if(temp==b)h=payload.toFloat();
 if(temp==a){t=payload.toFloat();t=(t-32)*5/9;}
  //Sending Data from NODEMCU to BoodSkap Platform
  StaticJsonBuffer<128> buffer;
  JsonObject &data = buffer.createObject();
  data["h"] =h;//present humidity
  data["t"] = t;//present temperature
  
  Boodskap.sendMessage(MESSAGE_ID, data);

  //setting up the conditions to turn on the irrigation valves

  if(t>=32.00 || h<0.4){
    digitalWrite(D5,HIGH);
    digitalWrite(D6,HIGH);
    digitalWrite(D7,HIGH);
    delay(1800000);
    digitalWrite(D5,LOW);
    digitalWrite(D6,LOW);
    digitalWrite(D7,LOW);
    }

    if((t>=27 && t<32) || h>0.50){
    digitalWrite(D5,HIGH);
    digitalWrite(D6,HIGH);
    //digitalWrite(D7,HIGH);
    delay(1800000);
    digitalWrite(D5,LOW);
    digitalWrite(D6,LOW);
    //digitalWrite(D7,LOW); 
      }

   if(t<27 || h>0.55){
    digitalWrite(D5,HIGH);
    delay(1000000);
    digitalWrite(D5,LOW);
    }
   
  }