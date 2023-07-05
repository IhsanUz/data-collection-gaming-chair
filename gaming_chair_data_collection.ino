#include <DHT.h>
#include <SoftwareSerial.h>//SoftwareSerial kütüphanimizi ekliyoruz.
#include <NewPing.h>
#define DHTPIN 2                                            //DHT11 sensör kütüphanemizi ekliyoruz.
#define DHTTYPE DHT11 //DHT 11
//HC-SR04 with Trig to Arduino Pin 9, Echo to Arduino pin 8
#define TRIGGER_PIN 9
#define ECHO_PIN 8
//Maximum distance we want to ping for (in centimeters).
#define MAX_DISTANCE 400  
// NewPing setup of pins and maximum distance.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
float mesafe;
float first_distance;

String wifi_name = "********";                 //Ağımızın adını buraya yazıyoruz.   
String wifi_password = "******";           //Ağımızın şifresini buraya yazıyoruz.

int rxPin = 10;                                               //ESP8266 RX pini
int txPin = 11;                                               //ESP8266 TX pini

String ip = "184.106.153.149";                                //Thingspeak ip adresi

int counter = 0;
bool flag =true;

DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial esp(rxPin, txPin);                             //Seri haberleşme pin ayarlarını yapıyoruz.

void setup() {  
  dht.begin();
  Serial.begin(9600);  //Seri port ile haberleşmemizi başlatıyoruz.
  Serial.println("Started");
  esp.begin(115200);                                          //ESP8266 ile seri haberleşmeyi başlatıyoruz.
  esp.println("AT");                                          //AT komutu ile modül kontrolünü yapıyoruz.
  Serial.println("AT Sent");
  while(!esp.find("OK")){                                     //Modül hazır olana kadar bekliyoruz.
    esp.println("AT");
    Serial.println("ESP8266 Can't found.");
  }
  Serial.println("OK Command ");
  esp.println("AT+CWMODE=1");                                 //ESP8266 modülünü client olarak ayarlıyoruz.
  while(!esp.find("OK")){                                     //Ayar yapılana kadar bekliyoruz.
    esp.println("AT+CWMODE=1");
    Serial.println("Adjustment is being made.....");
  }
  Serial.println("The client is set");
  Serial.println("Connecting to Wifi...");
  esp.println("AT+CWJAP=\""+wifi_name+"\",\""+wifi_password+"\"");    //Ağımıza bağlanıyoruz.
  while(!esp.find("OK"));                                     //Ağa bağlanana kadar bekliyoruz.
  Serial.println("Connection Successful.");
  delay(1000);
}
void loop() {
  esp.println("AT+CIPSTART=\"TCP\",\""+ip+"\",80");           //Thingspeak'e bağlanıyoruz.
  if(esp.find("Error")){                                      //Bağlantı hatası kontrolü yapıyoruz.
    Serial.println("AT+CIPSTART Error");
  }
  String datacollect;
  if(counter == 0 ){
    datacollect = send_firstData();
   }
 else{
  float sicaklik = dht.readTemperature();
  float nem = dht.readHumidity();
   if (isnan(nem) || isnan(sicaklik)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
     }
  mesafe = sonar.ping_cm();
  String veri = "GET https://api.thingspeak.com/update?api_key=8URIGBTRS35E6IGB";   //Thingspeak komutu. Key kısmına kendi api keyimizi yazıyoruz.                                   //Göndereceğimiz sıcaklık değişkeni
  veri += "&field1=";
  veri += String(sicaklik);
  veri += "&field2=";
  veri += String(nem);                                        //Göndereceğimiz nem değişkeni
  veri += "&field3=";
  veri += String(mesafe);   
  veri += "\r\n\r\n"; 
  datacollect = veri;
  
 }
  
  esp.print("AT+CIPSEND=");                                   //ESP'ye göndereceğimiz veri uzunluğunu veriyoruz.
  esp.println(datacollect.length()+2);
  delay(2000);
  if(esp.find(">")){                                          //ESP8266 hazır olduğunda içindeki komutlar çalışıyor.
    Serial.println("Data Sending...");
    esp.print(datacollect);                                          //Veriyi gönderiyoruz.
    Serial.println(datacollect);
    Serial.println("Data Sent.");
    delay(1000);
  }
  Serial.println("Connection Closed.");
  esp.println("AT+CIPCLOSE");                                //Bağlantıyı kapatıyoruz
  delay(15000);                                               //Yeni veri gönderimi için 1 dakika bekliyoruz.
}

 String send_firstData(){
  float firsttemp = dht.readTemperature();
  float firsthum = dht.readHumidity();
   if (isnan(firsthum) || isnan(firsttemp)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
     }
  first_distance = sonar.ping_cm();
  String veri = "GET https://api.thingspeak.com/update?api_key=90P6VID5C2RAVPMA";   //Thingspeak komutu. Key kısmına kendi api keyimizi yazıyoruz.                                   
  //Göndereceğimiz sıcaklık değişkeni
  veri += "&field1=";
  veri += String(firsttemp);
  veri += "&field2=";
  veri += String(firsthum);                                        //Göndereceğimiz nem değişkeni
  veri += "&field3=";
  veri += String(first_distance);   
  veri += "\r\n\r\n";
  counter = 1;
  Serial.println("First Data Sending...");
  return veri;
 }
