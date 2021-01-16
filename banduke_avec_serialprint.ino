
/*##################################################################################*/
/*#######                 Librairies et variables pour le GPS                #######*/
/*##################################################################################*/
//Import des librairies
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

//Pins de communication rx/tx
int rxPin = 16;
int txPin = 17;

//Variables pour stocker les écarts entre les positions
double deniv_pos = 0.0, deniv_neg = 0.0, distance_tot = 0.0, distance_m = 0.0, deniv = 0.0;

//Valeurs arbitraires, pour ne pas avoir 0,0 au milieu d'un océan.
double prev_pos_lat = 45.8992655922, prev_pos_lng = 6.1289521798, prev_pos_alt = 453.0;

//Variables pour stocker les données GPS, utilisées avec un mutex
int gps_satellites = 0;
double gps_lat = prev_pos_lat, gps_lng = prev_pos_lng, gps_alt = prev_pos_alt, gps_speed = 0.0;

//Mutex pour synchroniser l'utilisation des données GPS
SemaphoreHandle_t xSemaphore_GPS_Data = NULL;

//Seuil d'admission de positions identiques, distance entre les positions en mètres
const float SEUIL_DIST = 2.2;

//Variables pour stocker l'heure et la date en format HH-MM-SS et DD/MM/YY
char gps_time_str[9];
char gps_date_str[9];

//Instanciation
TinyGPSPlus gps;
SoftwareSerial ss(rxPin, txPin);
/*##################################################################################*/





/*##################################################################################*/
/*#######                Librairies et variables pour l'écran                #######*/
/*##################################################################################*/
//Import des librairies
#include <Wire.h>               
#include "SSD1306Wire.h"

//Import des fichiers utiles
#include "fonts.h"

//Instanciation
SSD1306Wire display(0x3c, SDA, SCL);   
/*##################################################################################*/





/*##################################################################################*/
/*#######    Librairies et variables pour l'accéléromètre et le gyroscope    #######*/
/*##################################################################################*/
//Import des librairies
#include <Arduino.h>
#include <TinyMPU6050.h>
//#include <Wire.h> //Déjà requis par l'écran

//Mutex pour synchroniser l'utilisation des valeurs d'accélération et d'angle
SemaphoreHandle_t xSemaphore_AccelGyro_Data = NULL;

//Variables pour stocker les valeurs d'angle et d'accélération. Axes Z et Y inutiles normalement.
double accel_X = 0.0, accel_Y = 0.0, accel_Z = 0.0, angle_X = 0.0, angle_Y = 0.0, angle_Z = 0.0;

//Nombre de mesures à moyenner pour les valeurs
int AVERAGING_VALUES = 25;
double average_angle_X = 0.0, average_angle_Y = 0.0, average_angle_Z = 0.0, average_accel_X = 0.0, average_accel_Y = 0.0, average_accel_Z = 0.0;

//Instanciation
MPU6050 mpu (Wire);
/*##################################################################################*/





/*##################################################################################*/
/*#######  Librairies et variables pour la gestion de la date et de l'heure  #######*/
/*##################################################################################*/
//Import des librairies
#define TIMELIB_ENABLE_MILLIS
#include <TimeLib.h>
int date_Years;
byte date_Months, date_Days, time_Hours, time_Minutes, time_Seconds;

//Variables pour stocker l'heure et la date en format HH-MM-SS et DD/MM/YY
char time_str[9];
char date_str[9];
/*##################################################################################*/


  


/*##################################################################################*/
/*#######              Librairies et variables pour la carte SD              #######*/
/*##################################################################################*/
//Librairie pour utiliser la carte SD
#include "FS.h"
#include "SD.h"
#include "SPI.h"


//Noms des dossiers et fichiers utilisés dans la carte SD
const char * projectRootDir = "/BanDuke"; //Dossier racine du projet, contenant les dossiers journaliers, eux-mêmes contenant les fichiers sessions
char dayDir_FullPath[18]; //Dossier journalier, contenant les fichiers sessions, 1 caractère de plus que le chemin complet pour que la chaîne soie NULL-terminated
char sessionFile_FullPath[31]; //1 caractère de plus que le chemin complet (3 noms de fichiers 8*3 = 24, 2 séparateur 24+2=26, . et extension 26+4 = 30) pour que la chaîne soie NULL-terminated
      
//Pins changeables par l'utilisateur
int CSpin = 4;
/*##################################################################################*/





/*##################################################################################*/
/*#######             Librairies et variables pour le WebServer              #######*/
/*##################################################################################*/
//Import des librairies
#include <WiFi.h>
#include <WiFiMulti.h> 
#include <ESP32WebServer.h> 
#include <ESPmDNS.h>
/*Librairies déjà incluses pour la carte SD
#include "FS.h"
#include "SD.h" 
#include "SPI.h"
*/

//Import des fichiers utiles
#include "Network.h"
#include "Sys_Variables.h"
#include "CSS.h"  

//Variable définissant le fichier portant les caractéristiques du réseau WiFi auquel se connecter
const char * pathToWifiDetails = "/BanDuke/Wifi/network.txt";

//Instanciation
WiFiMulti wifiMulti;
ESP32WebServer server(80);
/*##################################################################################*/


  


/*##################################################################################*/
/*#######                  Librairies et variables globales                  #######*/
/*##################################################################################*/
//Librairie pour manipuler les nombres
#include <math.h>

//Pins utilisés par les boutons
int btnChoicePin = 14; //D2
int btnLaunchPin = 15; //D4

//Booléens pour les boutons
bool choiceSession = false;
bool launchProgram = false;

//Semaphore pour la communication I2C qui est non thread-safe
SemaphoreHandle_t xSemaphore_I2C_Communication = NULL;
/*##################################################################################*/











/*##################################################################################*/
/*#######                     Fonctions pour la carte SD                     #######*/
/*##################################################################################*/
/*Checks if the root directory of the project exixts, if not, creates it. 
  Returns TRUE if everything went smoothly, FALSE otherwise*/
bool checkOrCreateRootDir(fs::FS &fs, const char * rootPath) {
  Serial.printf("Checking or creating project root directory '%s'\n", rootPath);
  File root = fs.open(rootPath);
  
  if(!root){
    Serial.printf("Failed to open directory '%s'\n", rootPath);
    Serial.printf("Creating Dir '%s'\n", rootPath);
    if(fs.mkdir(rootPath)){
      Serial.println("Dir created");
      return true;
    } 
    else {
      Serial.println("mkdir failed");
      return false;
    }
  }
  
  else {
    if(!root.isDirectory()){
      Serial.println("Not a directory");
      Serial.printf("Deleting file '%s'\n", rootPath);
      if(fs.remove(rootPath)){
        Serial.println("File deleted");
        
        Serial.printf("Creating Dir '%s'\n", rootPath);
        if(fs.mkdir(rootPath)){
          Serial.println("Dir created");
          return true;
        } 
        else {
          Serial.println("mkdir failed");
          return false;
        }
      } 
      else {
        Serial.println("Delete failed");
        return false;
      }
    }
    else {
      Serial.printf("Directory '%s' already exists\n", rootPath);
      return true;
    }
  }
  /*END OF CHECK OR CREATE ROOT DIRECTORY TO STORE SESSIONS DATA*/   
}



/*Checks if the daily directory for session files exixts, if not, creates it. 
  Returns TRUE if everything went smoothly, FALSE otherwise*/
bool checkOrCreateDayDir(fs::FS &fs, const char * dayDir_FullPath) {
  File dayDir = fs.open(dayDir_FullPath);
  if(!dayDir){
    Serial.printf("Failed to open directory '%s'\n", dayDir_FullPath);
    Serial.printf("Creating Dir '%s'\n", dayDir_FullPath);
    if(fs.mkdir(dayDir_FullPath)){
      Serial.println("Dir created");
      return true;
    } 
    else {
      Serial.println("mkdir failed");
      return false;
    }
  }
  else {
    if(!dayDir.isDirectory()){
      Serial.println("Not a directory");
      Serial.printf("Deleting file '%s'\n", dayDir_FullPath);
      if(fs.remove(dayDir_FullPath)){
        Serial.println("File deleted");
        
        Serial.printf("Creating Dir '%s'\n", dayDir_FullPath);
        if(fs.mkdir(dayDir_FullPath)){
          Serial.println("Dir created");
          return true;
        } 
        else {
          Serial.println("mkdir failed");
          return false;
        }
      } 
      else {
        Serial.println("Delete failed");
        return false;
      }
    }
    else {
      Serial.printf("Directory '%s' already exists\n", dayDir_FullPath);
      return true;
    }
  }
  
  /*END OF CHECK OR CREATE ROOT DIRECTORY TO STORE SESSIONS DATA*/   
}



/*Creates or overwrite the txt file for the current session. 
  Returns TRUE if everything went smoothly, FALSE otherwise*/
bool createOrOverWriteSessionFile(fs::FS &fs, const char * sessionFile_FullPath, char * startTimeMessage) {
  Serial.printf("Writing file: %s\n", sessionFile_FullPath);

  File sessionFile = fs.open(sessionFile_FullPath, FILE_WRITE);
  if(!sessionFile){
    Serial.printf("Failed to open file '%s' for writing\n", sessionFile_FullPath);
    sessionFile.close();
    return false;
  }
  if(sessionFile.print(startTimeMessage)){
    Serial.printf("File written with start message '%s'\n", startTimeMessage);
    sessionFile.close();
    return true;
  } 
  else {
    Serial.printf("Write failed for start message '%s'\n", startTimeMessage);
    sessionFile.close();
    return false;
  }
  /*END OF CREATE OR OVERWRITE SESSION FILE*/
}



/*Appends latest data to the txt file of the current session. 
  Returns TRUE if everything went smoothly, FALSE otherwise*/
bool appendToSessionFile(fs::FS &fs, const char * sessionFile_FullPath, char * messageToAppend) {
  Serial.printf("Appending to file: %s\n", sessionFile_FullPath);

  File sessionFile = fs.open(sessionFile_FullPath, FILE_APPEND);
  if(!sessionFile){
    Serial.printf("Failed to open file '%s' for appending\n", sessionFile_FullPath);
    sessionFile.close();
    return false;
  }
  if(sessionFile.print(messageToAppend)){
    Serial.printf("Message '%s' appended\n", messageToAppend);
    sessionFile.close();
    return true;
  } 
  else {
    Serial.printf("Append of '%s' failed\n", messageToAppend);
    sessionFile.close();
    return false;
  }
  /*END OF APPEND TO SESSION FILE*/
}
/*##################################################################################*/










/*##################################################################################*/
/*#######                     Fonctions pour le WebServer                    #######*/
/*##################################################################################*/
void HomePage(){
  SendHTML_Header();
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop(); // Stop is needed because no content length was sent
}

void File_Download(){ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  Serial.println(server.args());
  Serial.println(server.arg(0));
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("download")) SD_file_download(server.arg(0));
  }
  else SelectInput("Enter filename to download","download","download");
}

void SD_file_download(String filename){
  if (SD_present) { 
    File download = SD.open("/"+filename);
    if (download) {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } else ReportFileNotPresent("download"); 
  } else ReportSDNotPresent();
}

void SD_dir(){
  if (SD_present) { 
    File root = SD.open("/");
    if (root) {
      root.rewindDirectory();
      SendHTML_Header();
      webpage += F("<a href='/'><button>Home</button></a>");
      webpage += F("<a href='/download'><button>Download</button></a>");
      webpage += F("<a href='/delete'><button>Delete</button></a>");
      webpage += F("<a href='/dir'><button>Directory</button></a>");
      webpage += F("<br><br><table align='center'>");
      webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th></tr>");
      printDirectory("/");
      webpage += F("</table>");
      SendHTML_Content();
      root.close();
    }
    else 
    {
      SendHTML_Header();
      webpage += F("<h3>No Files Found</h3>");
    }
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();   // Stop is needed because no content length was sent
  } else ReportSDNotPresent();
}

void printDirectory(const char * dirname){
  File root = SD.open(dirname);
  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }
  File file = root.openNextFile();
  while(file){
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    String file_name = String(file.name());
    bool toSkip = file_name.startsWith("/System Volume Information");
    if(file.isDirectory()){
      if(!toSkip) {
        Serial.println(String(file.isDirectory()?"Dir ":"File ")+file_name);
        webpage += "<tr><td>"+file_name+"</td><td>Dir</td></tr>";
      }
      printDirectory(file.name());
    }
    else
    {
      if(!toSkip) {
        //Serial.print(String(file.name())+"\t");
        webpage += "<tr><td>"+file_name+"</td>";
        Serial.print(String(file.isDirectory()?"Dir ":"File ")+file_name+"\t");
        webpage += "<td>File</td>";
        int bytes = file.size();
        String fsize = "";
        if (bytes < 1024)                     fsize = String(bytes)+" B";
        else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
        else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
        else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
        webpage += "<td>"+fsize+"</td><td class='action_td'><a href='/download?download="+file_name+"'><i class='gg-software-download' style='margin-left:7px'></i></a></td><td class='action_td'><a href='/delete?delete="+file_name+"'><i class='gg-trash'></i></a></td></tr>";
        Serial.println(String(fsize));
      }
    }
    file = root.openNextFile();
  }
  file.close();
} 

void File_Delete(){
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("delete")) SD_file_delete(server.arg(0));
  }
  else SelectInput("Select a File to Delete","delete","delete");
}

void SD_file_delete(String filename) { // Delete the file 
  if (SD_present) { 
    SendHTML_Header();
    File dataFile = SD.open("/"+filename, FILE_READ); // Now read data from SD Card 
    Serial.print("Deleting file: "); Serial.println(filename);
    if (dataFile)
    {
      if (SD.remove("/"+filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '"+filename+"' has been erased</h3>"; 
        webpage += F("<a href='/'>[Home]</a><br><br>");
      }
      else
      { 
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='/'>[Home]</a><br><br>");
      }
    } else ReportFileNotPresent("delete");
    append_page_footer(); 
    SendHTML_Content();
    SendHTML_Stop();
  } else ReportSDNotPresent();
} 

void SendHTML_Header(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves. 
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}

void SendHTML_Content(){
  server.sendContent(webpage);
  webpage = "";
}

void SendHTML_Stop(){
  server.sendContent("");
  server.client().stop(); // Stop is needed because no content length was sent
}

void SelectInput(String heading1, String command, String arg_calling_name){
  SendHTML_Header();
  webpage += F("<h3>"); webpage += heading1 + "</h3>"; 
  webpage += F("<form action='/"); webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><a href="); webpage += command; webpage += F("><button>Valider</button></a><br><br><br></type='submit'></form>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

void ReportSDNotPresent(){
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>"); 
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

void ReportFileNotPresent(String target){
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

String file_size(int bytes){
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}
/*##################################################################################*/




















void setupSessionRecording() {
  
  ss.begin(9600);
  Serial.println();

  //Création du Mutex pour les variables d'accélération et d'angle
  xSemaphore_AccelGyro_Data = xSemaphoreCreateMutex();

  //Création du Mutex pour la communication I2C thread-safe
  xSemaphore_I2C_Communication = xSemaphoreCreateMutex();

  //Création du Mutex pour les données GPS
  xSemaphore_GPS_Data = xSemaphoreCreateMutex();

  vTaskDelay(1000);

  Serial.println("############################################################################################################\n");
  Serial.println("Starting app, please wait...\n");
  
  Serial.println("****************************************");
  Serial.println("Launching GPS...");

  
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Starting GPS, please wait...");
  display.display();
  delay(100);

  //Lancement du GPS, 10 secondes allouées
  for (unsigned long start = millis(); millis() - start < 10000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      //Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      gps.encode(c);
    }
  }
  //Enregistrement de la position
  prev_pos_lat = gps.location.lat();
  prev_pos_lng = gps.location.lng();
  prev_pos_alt = gps.altitude.meters();
  
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Getting accurate pos");
  display.display();
  delay(100);

  //while( (abs(prev_pos_lat) <= 0.001) && (abs(prev_pos_lng) <= 0.001) && (abs(prev_pos_alt) <= 0.1) ) {
  while(!gps.location.isValid() || !gps.date.isValid() || !gps.time.isValid()) {
    Serial.println("   GPS still inaccurate : ");
    Serial.print("   Location is valid : ");
    Serial.print(gps.location.isValid());
    Serial.print("   Date is valid : ");
    Serial.print(gps.date.isValid());
    Serial.print("   Time is valid : ");
    Serial.print(gps.time.isValid());
    Serial.print("   Sentences with fix : ");
    Serial.print(gps.sentencesWithFix());
    Serial.print("   Satellites : ");
    Serial.print(gps.satellites.value());
    Serial.print("   Latitude : ");
    Serial.print(gps.location.lat(), 10);
    Serial.print("   Longitude : ");
    Serial.print(gps.location.lng(), 10);
    Serial.print("   Altitude : ");
    Serial.print(gps.altitude.meters(), 2);
    Serial.print("   Heure : ");
    Serial.print(gps_time_str);
    Serial.print("   Date : ");
    Serial.println(gps_date_str);
    prev_pos_lat = gps.location.lat();
    prev_pos_lng = gps.location.lng();
    prev_pos_alt = gps.altitude.meters();
    snprintf(gps_time_str, 9, "%02u:%02u:%02u", gps.time.hour(), gps.time.minute(), gps.time.second());
    snprintf(gps_date_str, 9, "%02u/%02u/%04u", gps.date.day(), gps.date.month(), gps.date.year());
    while (ss.available())
    {
      if(gps.encode(ss.read())) {
        Serial.println("GPS data updated !");
      }
    }
    delay(1000);
  }
  

  snprintf(gps_time_str, 9, "%02u:%02u:%02u", gps.time.hour(), gps.time.minute(), gps.time.second());
  snprintf(gps_date_str, 9, "%02u/%02u/%02u", gps.date.day(), gps.date.month(), gps.date.year());
  
  Serial.println("GPS tracking current position");
  Serial.print("Satellites : ");
  Serial.print(gps.satellites.value());
  Serial.print("   Latitude : ");
  Serial.print(gps.location.lat(), 10);
  Serial.print("   Longitude : ");
  Serial.print(gps.location.lng(), 10);
  Serial.print("   Altitude : ");
  Serial.print(gps.altitude.meters(), 2);
  Serial.print("   Vitesse : ");
  Serial.print(gps.speed.kmph(), 2);
  Serial.print("   Heure : ");
  Serial.print(gps_time_str);
  Serial.print("   Date : ");
  Serial.println(gps_date_str);

  prev_pos_lat = gps.location.lat();
  prev_pos_lng = gps.location.lng();
  prev_pos_alt = gps.altitude.meters();
  gps_satellites = gps.satellites.value();
  gps_lat = gps.location.lat(), 
  gps_lng = gps.location.lng(), 
  gps_alt = gps.altitude.meters(), 
  gps_speed = gps.speed.kmph();
  Serial.println("****************************************\n");



  Serial.println("++++++++++++++++++++++++++++++++++++++++");
  Serial.println("Setting board time from GPS data");
  
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Setting time");
  display.display();
  delay(100);
  
  date_Years = gps.date.year();
  date_Months = gps.date.month();
  date_Days = gps.date.day();
  time_Hours = gps.time.hour();
  time_Minutes = gps.time.minute();
  time_Seconds = gps.time.second();
  setTime(time_Hours, time_Minutes, time_Seconds, date_Days, date_Months, date_Years);
  snprintf(time_str, 9, "%02u:%02u:%02u", hour(), minute(), second());
  snprintf(date_str, 9, "%02u/%02u/%02u", day(), month(), year());
  Serial.print("Board time : ");
  Serial.println(time_str);
  Serial.print("Board date : ");
  Serial.println(date_str);
  Serial.println("++++++++++++++++++++++++++++++++++++++++\n");



  Serial.println("<><><><><><><><><><><><><><><><><><><><>");
  Serial.println("Initializing SD card and storage...");
  
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Checking SD card...");
  display.display();
  delay(100);

  if(!SD.begin(CSpin)){
    Serial.println("Card mount failed");

    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "Card mount failed");
    display.display();
    delay(5000);
    
    vTaskDelete(NULL);
    return;
  }

  if(SD.cardType() == CARD_NONE){
    Serial.println("No SD card attached");

    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "No SD card attached");
    display.display();
    delay(5000);
  
    return;
  }

  int totalMegaBytes = (SD.cardSize() / (1024 * 1024));
  int usedMegaBytes = (SD.usedBytes() / (1024 * 1024));
  int freeMegaBytes = (totalMegaBytes - usedMegaBytes);
  double freeSpace = ((100.0 * freeMegaBytes) / totalMegaBytes);
  
  if (freeSpace < 10.0) {
    Serial.printf("WARNING ! Only %dMB (%f%%) of free space remaining (%dMB used of a total of %dMB space)\n", freeMegaBytes, freeSpace, usedMegaBytes, totalMegaBytes);
  }
  else {
    Serial.printf("%dMB (%f%%) of free space remaining (%dMB used of a total of %dMB space)\n", freeMegaBytes, freeSpace, usedMegaBytes, totalMegaBytes);
  }

  if(!checkOrCreateRootDir(SD, projectRootDir)) {
    Serial.printf("\nProject root directory couldn't be created with path '%s'\n\n", projectRootDir);

    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "Root dir unavailable");
    display.display();
    delay(2000);

    vTaskDelete(NULL);
    return;
  }
  else {
    Serial.printf("\nProject root directory exists, its path is '%s'\n\n", projectRootDir);

    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "Root dir available");
    display.display();
    delay(2000);
  
    snprintf(dayDir_FullPath, 18, "%s/%02u~%02u~%02u", projectRootDir, day(), month(), year() % 100);
    
    Serial.printf("Checking or creating day directory '%s'\n", dayDir_FullPath);
        
    if(!checkOrCreateDayDir(SD, dayDir_FullPath)) {
      Serial.printf("\nDay directory couldn't be created with path '%s'\n\n", dayDir_FullPath);

      display.clear();
      display.drawStringMaxWidth(0, 0, 128, "Day dir unavailable");
      display.display();
      delay(2000);

      vTaskDelete(NULL);
      return;
    }
    else {
      Serial.printf("\nDay directory exists, its path is '%s'\n\n", dayDir_FullPath);

      display.clear();
      display.drawStringMaxWidth(0, 0, 128, "Day dir available");
      display.display();
      delay(2000);
  
      snprintf(sessionFile_FullPath, 31, "%s/%02u-%02u-%02u.txt", dayDir_FullPath, hour(), minute(), second());
          
      char startTimeMessage[40];
      
      snprintf(startTimeMessage, 40, "Starttime#%04u-%02u-%02uT%02u:%02u:%02u.%03u+0000", year(), month(), day(), hour(), minute(), second());
  
      if (!createOrOverWriteSessionFile(SD, sessionFile_FullPath, startTimeMessage)) {
        Serial.printf("\nSession file couldn't be created with path is '%s'\n\n", sessionFile_FullPath);

        display.clear();
        display.drawStringMaxWidth(0, 0, 128, "Session file unavailable");
        display.display();
        delay(2000);

        vTaskDelete(NULL);
        return;
      }
      else {
        Serial.printf("\nSession file exists, its path is '%s', and now has the start message '%s'\n\n", sessionFile_FullPath, startTimeMessage);

        display.clear();
        display.drawStringMaxWidth(0, 0, 128, "Session file available");
        display.display();
        delay(2000);
  
        if (!appendToSessionFile(SD, sessionFile_FullPath, "\r\n")) {
          Serial.printf("\nSession file couldn't be appended with CRLF\n\n");

          display.clear();
          display.drawStringMaxWidth(0, 0, 128, "Session file not appended");
          display.display();
          delay(2000);

          vTaskDelete(NULL);
          return;
        }
        else {
          Serial.printf("\nSession file succesfully appended with CRLF\n\n");

          display.clear();
          display.drawStringMaxWidth(0, 0, 128, "Session file appended");
          display.display();
          delay(2000);
        }
      }
    }
  }

  Serial.println("SD card and storage succesfully initialized !");
  Serial.println("<><><><><><><><><><><><><><><><><><><><>\n");

  
  
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Calibrating gyro/accel");
  display.display();
  delay(100);

  //Initialisation et calibration de l'accéléromètre/gyroscope
  mpu.Initialize();
  
  Serial.println("========================================");
  Serial.println("Starting gyroscope/accelerometer calibration...");
  mpu.Calibrate();
  Serial.println("Calibration complete!");
  Serial.println("Gyro Offsets:");
  Serial.print("  GyroX Offset = ");
  Serial.println(mpu.GetGyroXOffset());
  Serial.print("  GyroY Offset = ");
  Serial.println(mpu.GetGyroYOffset());
  Serial.print("  GyroZ Offset = ");
  Serial.println(mpu.GetGyroZOffset());
  Serial.println("Accel Offsets: None");
  Serial.println("========================================\n");


  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Application launched !");
  display.display();
  delay(2000);
  
  display.clear();
  display.display();



  //Création de la routine de récupération des données capteurs et persistance dans la carte SD
  xTaskCreatePinnedToCore(
                    grabDataAndSaveSession,   // Task function. 
                    "grabDataAndSaveSession",     // name of task. 
                    4000,       // Stack size of task 
                    NULL,        // parameter of the task 
                    5,           // priority of the task 
                    NULL,      // Task handle to keep track of created task 
                    0);          // pin task to core 0

  //Création de la routine de lecture GPS, accéléromètre et gyroscope, et affichage des données sur le LCD i2C
  xTaskCreatePinnedToCore(
                    readSensorsAndDisplayI2C,   // Task function. 
                    "readSensorsAndDisplayI2C",     // name of task. 
                    4000,       // Stack size of task 
                    NULL,        // parameter of the task 
                    5,           // priority of the task 
                    NULL,      // Task handle to keep track of created task 
                    1);          // pin task to core 1
    
  
  Serial.println("Application launched ! \n");
  Serial.println("############################################################################################################\n");

  /*END OF SETUP SESSION RECORDING LOOP*/
}








void displayGPS(int gps_nb_sats, double gps_latitude, double gps_longitude, double gps_altitude, double gps_spd) {
  Serial.print("Satellites : ");
  Serial.print(gps_nb_sats);
  Serial.print("   Latitude : ");
  Serial.print(gps_latitude, 10);
  Serial.print("   Longitude : ");
  Serial.print(gps_longitude, 10);
  Serial.print("   Altitude : ");
  Serial.print(gps_altitude, 2);
  Serial.print("   Vitesse : ");
  Serial.print(gps_spd, 2);

  snprintf(time_str, 9, "%02u:%02u:%02u", hour(), minute(), second());
  snprintf(date_str, 9, "%02u/%02u/%02u", day(), month(), year());
  Serial.print("   Board time : ");
  Serial.print(time_str);
  Serial.print("   Board date : ");
  Serial.println(date_str);
  
  double distance_m =
    TinyGPSPlus::distanceBetween(
      gps_latitude,
      gps_longitude,
      prev_pos_lat,
      prev_pos_lng);
  deniv = prev_pos_alt - gps_altitude;
     
  Serial.print("            Prev latitude : ");
  Serial.print(prev_pos_lat, 10);
  Serial.print("   longitude : ");
  Serial.print(prev_pos_lng, 10);
  Serial.print("   altitude : ");
  Serial.print(prev_pos_alt, 2);
  Serial.print("   Distance (m) avec la dernière pos : ");
  Serial.print(distance_m, 2);
  Serial.print("   Dénivelé (m) avec la dernière pos : ");
  Serial.println(deniv, 2);   
    
  //Réduction du bruit sur la position GPS
  if(distance_m < SEUIL_DIST) {  
    Serial.println("  Positions identiques !"); 
  }
    
  Serial.print("            Distance totale (m) : ");
  Serial.print(distance_tot, 2);
  Serial.print("   dénivelé positif (m) : ");
  Serial.print(deniv_pos, 2);
  Serial.print("   dénivelé négatif (m) : ");
  Serial.println(deniv_neg, 2);
  Serial.println("");
    
  Serial.println("");

  /*END OF DISPLAYING GPS LOOP*/
}






void grabDataAndSaveSession(void * pvParameters) {
  /*DELAY IN ORDER TO LET THE TIME TO READ FIRST VALUES AND GET MORE PRECISE*/
  vTaskDelay(3000);

  /*TEMPORARY VARS TO FETCH GLOBAL VALUES IN MUTEXES, FOR ACCEL & GYRO AND GPS*/
  double temp_accel_X = 0.0, temp_accel_Y = 0.0, temp_accel_Z = 0.0, temp_angle_X = 0.0, temp_angle_Y = 0.0, temp_angle_Z = 0.0;
  int temp_gps_satellites = 0;
  double temp_gps_lat = 0.0, temp_gps_lng = 0.0, temp_gps_alt = 0.0, temp_gps_speed = 0.0;
  bool gps_data_fetched = false;

  /*STORES THE MESSAGE TO APPEND TO THE SESSION FILE*/
  char messageToAppend[200];
  
  for(;;) {

    //TODO: écrire les variables dans le fichier session


    /*FETCHING GLOBAL GPS DATA*/
    if( xSemaphoreTake( xSemaphore_GPS_Data, ( TickType_t ) 100 ) == pdTRUE ) {
      //Mutex obtenu, affichage des données GPS
      temp_gps_satellites = gps_satellites;
      temp_gps_lat = gps_lat;
      temp_gps_lng = gps_lng;
      temp_gps_alt = gps_alt;
      temp_gps_speed = gps_speed;
  
      //Relâche le Mutex une fois l'affichage des valeurs effectué
      xSemaphoreGive( xSemaphore_GPS_Data );
      
      gps_data_fetched = true;
    }
    else{
      //Impossible d'obtenir le Mutex (logiquement ne devrait pas arriver)
      Serial.println("@@@@@@@@@@@@@@@@@@@\n  Mutex indisponible pour afficher les données GPS\n@@@@@@@@@@@@@@@@@@@");

      gps_data_fetched = false;
    }

    if(gps_data_fetched) {
      distance_m =
        TinyGPSPlus::distanceBetween(
          temp_gps_lat,
          temp_gps_lng,
          prev_pos_lat,
          prev_pos_lng);
      deniv = prev_pos_alt - temp_gps_alt;
      
      //Réduction du bruit sur la position GPS
      if(distance_m < SEUIL_DIST) {  
        Serial.println("  Positions identiques !"); 
      }
      else {
        if(deniv < 0.0) {
          deniv_neg += deniv;
        }
        else {
          deniv_pos += deniv;
        }
        distance_tot += distance_m;
          
        prev_pos_lat = temp_gps_lat;
        prev_pos_lng = temp_gps_lng;
        prev_pos_alt = temp_gps_alt;
      }
      /*DISPLAYING GPS DATA*/
      //TODO : remove
      displayGPS(temp_gps_satellites, temp_gps_lat, temp_gps_lng, temp_gps_alt, temp_gps_speed);
    }

    

    /*FETCHING GLOBAL ACCEL & GYRO DATA*/
    if( xSemaphoreTake( xSemaphore_AccelGyro_Data, ( TickType_t ) 100 ) == pdTRUE ) {
      //Mutex obtenu, récupération des valeurs d'angle et d'accélération
      temp_angle_X = angle_X;
      temp_angle_Y = angle_Y;
      temp_angle_Z = angle_Z;

      temp_accel_X = accel_X;
      temp_accel_Y = accel_Y;
      temp_accel_Z = accel_Z;
          
      //TODO: écrire les variables dans le fichier session
  
      //Relâche le Mutex une fois la récupération des valeurs effectuées
      xSemaphoreGive( xSemaphore_AccelGyro_Data );
    }
    else{
      //Impossible d'obtenir le Mutex (logiquement ne devrait pas arriver)
      Serial.println("°°°°°°°°°°°°°°°°°°°\n  Mutex indisponible pour récupérer les valeurs d'accélération et d'angle\n°°°°°°°°°°°°°°°°°°°");
    }

    //GPS(lat, lon, alt, speed) -> ACC(x,y,z) -> GYR(x,y,z) -> CALC (dist, deniv+, deniv-)

    /*APPENDING RECORDED DATA TO SESSION FILE*/
    /*                               ~~~~~~~~~~~~~~~~~TIME~~~~~~~~~~~~~~~~~ ---------GPS--------- _____ACCL_____ =====GYRO===== +++++CALC+++++*/
    snprintf(messageToAppend, 200, "%04u-%02u-%02uT%02u:%02u:%02u.%03u+0000,%.10f,%.10f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\r\n", 
    year(), month(), day(), 
    hour(), minute(), second(), millisecond(), 
    temp_gps_lat, temp_gps_lng, temp_gps_alt, temp_gps_speed, 
    temp_accel_X, temp_accel_Y, temp_accel_Z, 
    temp_angle_X, temp_angle_Y, temp_angle_Z, 
    distance_tot, deniv_pos, deniv_neg);

    //Serial.printf("\nData message : '%s'\n\n", messageToAppend);
    
    if(appendToSessionFile(SD, sessionFile_FullPath, messageToAppend)) {
      Serial.printf("Successfully appended to session file : '%s'\n", messageToAppend);
    }
    else {
      Serial.printf("Append to session file failed for msg : '%s'\n", messageToAppend);
    }
    
    
    /*DISPLAYING ACCEL & GYRO DATA*/
    String angle_str = "Angle   X:" + String(temp_angle_X,2) + " Y:" + String(temp_angle_Y,2) + " Z:" + String(temp_angle_Z,2);
    String accel_str = "Accel   X:" + String(temp_accel_X,2) + " Y:" + String(temp_accel_Y,2) + " Z:" + String(temp_accel_Z,2);
    Serial.println(angle_str);
    Serial.println(accel_str);

    delay(5);

    /*END OF CORE #0 INFINITE LOOP*/
  }
  /*END OF CORE #0 TASK*/
}






void readSensorsAndDisplayI2C(void * pvParameters){  
  vTaskDelay(100); /*Laisse le temps de supprimer la void loop*/
  for(;;){
    /*READING AND SAVING GPS DATA*/
    while (ss.available() > 0) {
      if(gps.encode(ss.read())) {
        if( xSemaphoreTake( xSemaphore_GPS_Data, ( TickType_t ) 100 ) == pdTRUE ) {
          //Mutex obtenu, mise à jour des données GPS
          
          gps_satellites = gps.satellites.value();
          gps_lat = gps.location.lat();
          gps_lng = gps.location.lng();
          gps_alt = gps.altitude.meters();
          gps_speed = gps.speed.kmph();
    
          //Relâche le Mutex une fois la récupération des valeurs effectuées
          xSemaphoreGive( xSemaphore_GPS_Data );
        }
        else{
          //Impossible d'obtenir le Mutex (logiquement ne devrait pas arriver)
          Serial.println("===================\n  Mutex indisponible pour mettre à jour les valeurs GPS\n===================");
        }
      }
    }
  
  
  
    /*READING AND SUMMING ACCEL & GYRO DATA*/
    for (int i = 0; i < AVERAGING_VALUES;) {
      if( xSemaphoreTake( xSemaphore_I2C_Communication, ( TickType_t ) 20 ) == pdTRUE ) {
        //Mutex I2C obtenu, récupération des valeurs d'angle et d'accélération
        
        mpu.Execute();
  
        average_angle_X += mpu.GetAngX();
        average_angle_Y += mpu.GetAngY();
        average_angle_Z += mpu.GetAngZ();
  
        average_accel_X += mpu.GetAccX();
        average_accel_Y += mpu.GetAccY();
        average_accel_Z += mpu.GetAccZ();
  
        i++;
                 
        //Relâche le Mutex une fois la récupération des valeurs effectuée
        xSemaphoreGive( xSemaphore_I2C_Communication );
      }
      else{
        //Impossible d'obtenir le Mutex de communication I2C
        Serial.println("¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤\n  Mutex I2C indisponible pour extraire les valeurs d'accélération et d'angle du capteur\n¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤¤");
      }
    }
  
  
    /*AVERAGING AND UPDATING GLOBAL ACCEL & GYRO DATA*/  
    if( xSemaphoreTake( xSemaphore_AccelGyro_Data, ( TickType_t ) 50 ) == pdTRUE ) {
      //Mutex obtenu, mise à jour des valeurs d'angle et d'accélération
      
      angle_X = (average_angle_X / AVERAGING_VALUES);
      angle_Y = (average_angle_Y / AVERAGING_VALUES);
      angle_Z = (average_angle_Z / AVERAGING_VALUES);
  
      accel_X = (average_accel_X / AVERAGING_VALUES);
      accel_Y = (average_accel_Y / AVERAGING_VALUES);
      accel_Z = (average_accel_Z / AVERAGING_VALUES);
  
      //Relâche le Mutex une fois la mise à jour des valeurs effectuées
      xSemaphoreGive( xSemaphore_AccelGyro_Data );
    }
    else{
      //Impossible d'obtenir le Mutex (logiquement ne devrait pas arriver)
      Serial.println("-------------------\n  Mutex non disponible pour mettre à jour les valeurs d'angle et d'accélération moyennées\n-------------------");
    }
  
    /*RESETTING TEMPORARY VALUES FOR ACCEL & GYRO DATA*/
    average_angle_X = 0.0;
    average_angle_Y = 0.0;
    average_angle_Z = 0.0;
    average_accel_X = 0.0;
    average_accel_Y = 0.0;
    average_accel_Z = 0.0;
  
  
  
    /*DISPLAYING GPS DATA (LAT, LNG, ALT, SATS) ON I2C DISPLAY*/
    if( xSemaphoreTake( xSemaphore_I2C_Communication, ( TickType_t ) 100 ) == pdTRUE ) {
      //Mutex I2C obtenu, affichage des valeurs d'angle et d'accélération
          
      display.clear();
      display.drawStringMaxWidth(0, 0, 128,
        String(prev_pos_lat, 3) + 
          ";" + String(prev_pos_lng, 3) + "\n" + 
          "alt:" + String(prev_pos_alt, 0) +
          ",sat:" + gps.satellites.value());
      display.display();
                  
      //Relâche le Mutex une fois l'affichage des valeurs effectué
      xSemaphoreGive( xSemaphore_I2C_Communication );
    }
    else{
      //Impossible d'obtenir le Mutex de communication I2C
      Serial.println("~~~~~~~~~~~~~~~~~~~\n  Mutex I2C indisponible pour afficher les valeurs d'accélération et d'angle\n~~~~~~~~~~~~~~~~~~~");
    }
  }
}










void setupWebServer() {
  Serial.print(F("Initializing SD card...\n")); 
  if (!SD.begin(SD_CS_pin)) { // see if the card is present and can be initialised. Wemos SD-Card CS uses D8 
    Serial.println(F("Card failed or not present, no SD Card data logging possible..."));
    SD_present = false; 
    
    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "SD mount failed");
    display.display();
    delay(5000);
  } 
  else
  {
    Serial.println(F("Card initialised... file access enabled..."));
    SD_present = true; 
    
    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "SD initialized");
    display.display();
    delay(5000);
  }
  
  if (!WiFi.config(local_IP, gateway, subnet, dns)) { //WiFi.config(ip, gateway, subnet, dns1, dns2);
    Serial.println("WiFi STATION Failed to configure Correctly"); 
    
    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "WiFi failed");
    display.display();
    delay(5000);
  } 
  
  if (SD_present) {
    char readFromFile[389], ssid[129], password[257];
    int readFromFile_Index = 0;
    bool parseDetails = true;

    char readByte;

    File wifiDetails = SD.open(pathToWifiDetails);
    if(!wifiDetails){
      Serial.printf("Failed to open file '%s' for reading, falling back to ssid and password from Network.h\n", pathToWifiDetails);
      wifiMulti.addAP(ssid_1, password_1);
      
      display.clear();
      display.drawStringMaxWidth(0, 0, 128, "WiFi details failed");
      display.display();
      delay(5000);
    }
    else {
      Serial.printf("Reading from file '%s'\n", pathToWifiDetails);
      while(wifiDetails.available()){
        readByte = wifiDetails.read();
        if(readByte != EOF && readByte != '\n' && readByte != '\r' && readByte != '\t') {
          readFromFile[readFromFile_Index] = readByte;
        }
        else {
          break;
        }
        readFromFile_Index++;
        if(readFromFile_Index == 388) {
          Serial.println("Max size reached for readFromFile, falling back to ssid and password from Network.h");
          wifiMulti.addAP(ssid_1, password_1);
          parseDetails = false;
          break;
        }
      }
      wifiDetails.close();
  
      Serial.printf("Raw value read :  '%s'\n", readFromFile);

      if (parseDetails) {
        char* identifier = strtok(readFromFile, " :: ");
        strncpy(ssid, identifier, 128);
        identifier = strtok(NULL, " :: ");
        strncpy(password, identifier, 256);
    
        Serial.printf("WiFi details fetched from SD :     SSID : '%s'     PASSWORD: '%s'\n", ssid, password);
        
        wifiMulti.addAP(ssid, password);
      
        display.clear();
        display.drawStringMaxWidth(0, 0, 128, "WiFi details success");
        display.display();
        delay(5000);
      } 
    }

  }
  else {
    vTaskDelete(NULL);
  } 
  
  Serial.println("Connecting ...");
  while (wifiMulti.run() != WL_CONNECTED) { 
    delay(100); Serial.print('.');
  }
  Serial.println("\nConnected to "+WiFi.SSID()+" Use Domain name : "+servername+".local or IP address: "+WiFi.localIP().toString()); 
    
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, String("WiFi: " + WiFi.SSID()));
  display.display();
  delay(5000);
    
  if (!MDNS.begin(servername)) {
    Serial.println(F("Error setting up MDNS responder!")); 
    ESP.restart(); 
  }
  
  server.on("/",         HomePage);
  server.on("/download", File_Download);
  server.on("/delete",   File_Delete);
  server.on("/dir",      SD_dir);
  
  server.begin();
  Serial.println("HTTP server started");

  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Server launched");
  display.display();
  delay(5000);

  display.clear();
  display.drawStringMaxWidth(0, 0, 128, String("IP: "+WiFi.localIP().toString()));
  display.display();

  //Création de la routine de lecture GPS, accéléromètre et gyroscope, et affichage des données sur le LCD i2C
  xTaskCreatePinnedToCore(
                    loopWebServer,   // Task function. 
                    "loopWebServer",     // name of task. 
                    4000,       // Stack size of task 
                    NULL,        // parameter of the task 
                    5,           // priority of the task 
                    NULL,      // Task handle to keep track of created task 
                    1);          // pin task to core 1
}






void loopWebServer(void * pvParameters){
  vTaskDelay(100); /*Laisse le temps de supprimer la void loop*/
  for(;;){
    server.handleClient();
  }
}














void IRAM_ATTR changeChoiceBtn() {
  choiceSession = !choiceSession;
}
void IRAM_ATTR launchProgramBtn() {
  launchProgram = !launchProgram;
}

void setup()
{
  //Ouverture des liaisons séries
  Serial.begin(115200);
  while (!Serial)
    vTaskDelay(10); 

  Serial.println();

  //Initialisation de l'écran
  display.init();
  display.flipScreenVertically();
  display.setFont(Roboto_20);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Initializing, please wait...");
  display.display();
  delay(100);
  
  char * menuDispSessionRecorder = ">SessionRec FileTransfer";
  char * menuDispFileTransfer = "SessionRec >FileTransfer";

  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "SessionRec FileTransfer");
  display.display();
  delay(100);

  attachInterrupt(btnChoicePin, changeChoiceBtn, RISING); //D2
  attachInterrupt(btnLaunchPin, launchProgramBtn, RISING); //D4

  while(!launchProgram) {
    if(choiceSession) {
      display.clear();
      display.drawStringMaxWidth(0, 0, 128, menuDispSessionRecorder);
      display.display();
    }
    else {
      display.clear();
      display.drawStringMaxWidth(0, 0, 128, menuDispFileTransfer);
      display.display();
    }
    delay(50);
  }

  if(choiceSession){
    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "Launching SessionRec");
    display.display();
    delay(2000);
    setupSessionRecording();
  }
  else {
    display.clear();
    display.drawStringMaxWidth(0, 0, 128, "Launching FileTransfer");
    display.display();
    delay(2000);
    setupWebServer();
  }
}



void loop(){
  Serial.println("Will kill loop task...");
  vTaskDelete(NULL);
  Serial.println("Loop task is more resilient than Chuck Norris");
}
