#include <WiFiClientSecure.h>   
#ifndef DEBUG                   
#  define DEBUG true             
#  define DebugPrint(x);        if(DEBUG){Serial.print(x);}  
#  define DebugPrintln(x);      if(DEBUG){Serial.println(x);} 
#endif


const char* deepgramApiKey =    "6bdb9149a362f8ca5585abdc8f2ec9c3304e3ade";  // enter your deepgram API key within "" 

#define STT_LANGUAGE      "en-IN" 

#define TIMEOUT_DEEPGRAM   12     

#define STT_KEYWORDS            "&keywords=KALO&keywords=Sachin&keywords=Google"  


WiFiClientSecure client;       


String SpeechToText_Deepgram( String audio_filename )
{ 
  uint32_t t_start = millis(); 
  


  if ( !client.connected() )
  { DebugPrintln("> Initialize Deepgram Server connection ... ");
    client.setInsecure();
   
    if (!client.connect("api.deepgram.com", 443)) 
    { Serial.println("\nERROR - WifiClientSecure connection to Deepgram Server failed!");
      client.stop(); 
      return ("");   
    }
    DebugPrintln("Done. Connected to Deepgram Server.");
  }
  uint32_t t_connected = millis();  

 
  
  File audioFile = SD.open( audio_filename );    
  if (!audioFile) {
    Serial.println("ERROR - Failed to open file for reading");
    return ("");
  }
  size_t audio_size = audioFile.size();
  audioFile.close();
  
  


  String socketcontent = "";
  while (client.available()) 
  { char c = client.read(); socketcontent += String(c);
  } int RX_flush_len = socketcontent.length(); 
 



  String optional_param;                          
  optional_param =  "?model=nova-2-general";      
  optional_param += (STT_LANGUAGE != "") ? ("&language="+(String)STT_LANGUAGE) : ("&detect_language=true");  
  optional_param += "&smart_format=true";         
  optional_param += "&numerals=true";         
  optional_param += STT_KEYWORDS;                
  
  client.println("POST /v1/listen" + optional_param + " HTTP/1.1"); 
  client.println("Host: api.deepgram.com");
  client.println("Authorization: Token " + String(deepgramApiKey));
  client.println("Content-Type: audio/wav");
  client.println("Content-Length: " + String(audio_size));
  client.println();   
  uint32_t t_headersent = millis();     

  

  
  File file = SD.open( audio_filename, FILE_READ );
  const size_t bufferSize = 1024;      
  uint8_t buffer[bufferSize];
  size_t bytesRead;
  while (file.available()) 
  { bytesRead = file.read(buffer, sizeof(buffer));
    if (bytesRead > 0) {client.write(buffer, bytesRead);}        
  }
  file.close();
  uint32_t t_wavbodysent = millis();  


 
  String response = "";   
  while ( response == "" && millis() < (t_wavbodysent + TIMEOUT_DEEPGRAM*1000) )   
  { while (client.available())                         
    { char c = client.read();
      response += String(c);      
    }

    DebugPrint(".");  delay(100);           
  } 
  DebugPrintln();
  if (millis() >= (t_wavbodysent + TIMEOUT_DEEPGRAM*1000))
  { Serial.print("\n*** TIMEOUT ERROR - forced TIMEOUT after " + (String) TIMEOUT_DEEPGRAM + " seconds");
    Serial.println(" (is your Deepgram API Key valid ?) ***\n");    
  } 
  uint32_t t_response = millis();  



  client.stop();  
                     
    

  
  int    response_len  = response.length();
  String transcription = json_object( response, "\"transcript\":" );
  String language      = json_object( response, "\"detected_language\":" );
  String wavduration   = json_object( response, "\"duration\":" );
  return transcription;    
}





void Deepgram_KeepAlive()     
{
  uint32_t t_start = millis();  
  DebugPrint( "* Deepgram KeepAlive | " );

  
  if ( !client.connected() )
  { DebugPrint("NEW Reconnection ... ");
    client.setInsecure();
   
    if (!client.connect("api.deepgram.com", 443)) 
    { Serial.println("\n* PING Error - Server Connection failed.");
 
      return;  
    }  
    DebugPrint( "Done, connected.  -->  Connect Latency [sec]: ");  
    DebugPrintln( (String)((float)((millis()-t_start))/1000) );  
    return;  
  } 
   

  
  uint8_t empty_wav[] = {
  0x52,0x49,0x46,0x46, 0x40,0x00,0x00,0x00, 0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20, 
  0x10,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x80,0x3E,0x00,0x00,0x80,0x3E,0x00,0x00,
  0x01,0x00,0x08,0x00,0x64,0x61,0x74,0x61, 0x14,0x00,0x00,0x00, 0x80,0x80,0x80,0x80, 
  0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 }; 
  
  client.println("POST /v1/listen HTTP/1.1"); 
  client.println("Host: api.deepgram.com");
  client.println("Authorization: Token " + String(deepgramApiKey));
  client.println("Content-Type: audio/wav");
  client.println("Content-Length: " + String(sizeof(empty_wav)));
  client.println();
  client.write(empty_wav, sizeof(empty_wav)); 

  

  
  String response = "";
  while (client.available()) 
  { char c = client.read(); response += String(c);
  } int RX_len = response.length();    
}





String json_object( String input, String element )
{ String content = "";
  int pos_start = input.indexOf(element);      
  if (pos_start > 0)                                     
  {  pos_start += element.length();                            
     int pos_end = input.indexOf( ",\"", pos_start);      
     if (pos_end > pos_start)                            
     { content = input.substring(pos_start,pos_end);      
     } content.trim();                                   
     if (content.startsWith("\""))                           
     { content=content.substring(1,content.length()-1);   
     }     
  }  
  return (content);
}
