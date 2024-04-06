//#define CONFIG_ARDUINO_LOOP_STACK_SIZE 32*1024; //32KB
#include <SD.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <M5Cardputer.h>
#include "M5GFX.h"
#include <ArduinoJson.h>
#include <string.h>
//#include <AudioFileSourceBuffer.h>
// #include <AudioFileSourceHTTPStream.h>
// #include <AudioGeneratorMP3.h>
//#include <AudioOutputI2S.h>
#include <stdlib.h>
#include <stdint.h>

const char* ssid = "WIFI_NAME"; //NOME DA REDE WIFI
const char* password = "PASSWORD"; //SENHA DA REDE WIFI

M5Canvas canvas(&M5Cardputer.Display);
String data = "> ";

void setup() {
  Serial.begin(115200);
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(0.5);
  M5Cardputer.Display.drawRect(0, 0, M5Cardputer.Display.width(),
                                M5Cardputer.Display.height() - 28, GREEN);
  M5Cardputer.Display.setTextFont(&fonts::FreeSerifBoldItalic18pt7b);

  M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 4,
                                M5Cardputer.Display.width(), 4, GREEN);

  canvas.setTextFont(&fonts::FreeSerifBoldItalic18pt7b);
  canvas.setTextSize(0.5);
  canvas.createSprite(M5Cardputer.Display.width() - 8,
                      M5Cardputer.Display.height() - 36);
  canvas.setTextScroll(true);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  canvas.println("Connecting");
  canvas.pushSprite(4, 4);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    canvas.print(".");
  }
  Serial.println("");
  canvas.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  canvas.println("Connected to WiFi network with IP Address: ");

  canvas.pushSprite(4, 4);
  
  Serial.println(WiFi.localIP());
  canvas.println(WiFi.localIP());
  canvas.pushSprite(4, 4);
  delay(3000);

   
  canvas.println("Type a question and hit ENTER.");
  canvas.pushSprite(4, 4);
  M5Cardputer.Display.drawString(data, 4, M5Cardputer.Display.height() - 24);
}

void loop() {
  M5Cardputer.update();

  if (M5Cardputer.Keyboard.isChange()) {
    if (M5Cardputer.Keyboard.isPressed()) {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

      for (auto i : status.word) {
          data += i;
      }

      if (status.del) {
          data.remove(data.length() - 1);
      }

      if (status.enter) {
          data.remove(0, 2);
          canvas.println(data);
          canvas.pushSprite(4, 4);
          String data2 = data;
          data = "> ";
          chamaAPIs(data2);
      }

      M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 28,
                                    M5Cardputer.Display.width(), 25,
                                    BLACK);

      M5Cardputer.Display.drawString(data, 4,
                                      M5Cardputer.Display.height() - 24);
    }
  }
}

void chamaAPIs(String text){
  //Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {

    String postParameterString = requisitarCohereAPI(text);
    int postSize = postParameterString.length();
    if(postSize > 0){

      imprimirResposta(postParameterString);
      
    }else{
      canvas.println("An error occurred. Try again.");
      canvas.pushSprite(4, 4);
    }

  } else {
    Serial.println("WiFi Disconnected");
    M5Cardputer.Display.println("WiFi Disconnected");
  }
}

void imprimirResposta(String resposta){
  char buf[resposta.length()];
  resposta.toCharArray(buf, resposta.length());
  int cont = 0;
  int contSpace = 0;
  for (int i=0; i<resposta.length(); i++) {
    cont++;
    if(buf[i] == '\n'){
      cont = 0;
    }
    if(buf[i] == ' '){
      if(contSpace < 5){
        delay(500);
      }else{
        if(contSpace == 8){

          contSpace = 0;
        }
      }    
      contSpace++;
    }
    int j=0;
    if(buf[i-1] == ' ' && buf[i] != ' '){
      j=0;
      while(buf[i+j] != ' ' && buf[i+j] != '\n'){        
        j++;
      }
    }
    if((cont + j) > 31){
      canvas.print('\n');
      cont = 0;
    }
    canvas.print(buf[i]);
    canvas.pushSprite(4, 4);
    canvas.pushSprite(4, 4);    
  }
  canvas.print("\n");
  canvas.pushSprite(4, 4);
  canvas.pushSprite(4, 4);
}


String urlencode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }

  int size = encodedString.length();
  char old_str[size];
  char new_str[size];
  encodedString.toCharArray(old_str, size);
  int i, j;

  for (i = 0, j = 0; i < size; i++, j++) {
    if (old_str[i] == '+') {
      new_str[j] = '%';
      new_str[j + 1] = '2';
      new_str[j + 2] = '0';
      j += 2;
    } else {
      new_str[j] = old_str[i];
    }
  }
  new_str[j] = '\0';

  return String(new_str);
}

String requisitarCohereAPI(String texto) {
  HTTPClient http;
  const char* serverName = "https://api.cohere.ai/v1/generate";
  http.begin(serverName);
  http.setTimeout(30000);
  char jsonOutput[500];

  // If you need an HTTP request with a content type: application/json, use the following:
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer YOUR_COHERE_API_KEY_HERE"); //get your token at Cohere website

  JsonDocument doc;
  JsonDocument doc2;

  String respostaText;

  char buf[texto.length()+1];
  texto.toCharArray(buf, texto.length()+1);
  Serial.println(buf);

  JsonObject object = doc.to<JsonObject>();
  object["model"] = "command";
  object["prompt"] = buf;
  object["max_tokens"] = 464;
  object["temperature"] = 0.9;
  object["k"] = 0;
  //object["stop_sequences"] = NULL; //array []
  object["return_likelihoods"] = "NONE";

  serializeJson(doc, jsonOutput);
  delay(2500);
  int httpResponseCode = -999;

  canvas.println("Thinking...");
  canvas.pushSprite(4, 4);
  canvas.pushSprite(4, 4);

  httpResponseCode = http.POST(String(jsonOutput));

  if(httpResponseCode == 200){
    String retorno;
    retorno = http.getString();
    //delay(1000);
    DeserializationError err = deserializeJson(doc2, retorno);

    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.f_str());
    } else {
      Serial.println("Sem erro JSON");
    }

    const char* text = doc2["generations"][0]["text"];
    //const String text = retorno;

    Serial.println(text);

    // liberar recursos
    http.end();

    return String(text);
  }else{
    http.end();
    return String("");
  }
  
}