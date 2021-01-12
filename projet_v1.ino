
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
double deniv_pos = 0.0, deniv_neg = 0.0, distance_tot = 0.0, deniv = 0.0;

//Valeurs arbitraires, pour ne pas avoir 0,0 au milieu d'un océan.
double prev_pos_lat = 45.8992655922, prev_pos_lng = 6.1289521798, prev_pos_alt = 453.0;

//Seuil d'admission de positions identiques, distance entre les positions en mètres
const float SEUIL_DIST = 1.05;

//Variables pour stocker l'heure et la date en format DD-MM-SS et DD/MM/YY
char gps_time_str[16];
char gps_date_str[16];

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
int AVERAGING_VALUES = 5;
double average_angle_X = 0.0, average_angle_Y = 0.0, average_angle_Z = 0.0, average_accel_X = 0.0, average_accel_Y = 0.0, average_accel_Z = 0.0;

//Instanciation
MPU6050 mpu (Wire);
/*##################################################################################*/





/*##################################################################################*/
/*#######  Librairies et variables pour la gestion de la date et de l'heure  #######*/
/*##################################################################################*/
//Import des librairies

//#include <TimeLib.h>
//int date_Years;
//byte date_Months, date_Days, time_Hours, time_Minutes, time_Seconds;

//Variables pour stocker l'heure et la date en format DD-MM-SS et DD/MM/YY
char time_str[16];
char date_str[16];
/*##################################################################################*/


  


/*##################################################################################*/
/*#######                  Librairies et variables globales                  #######*/
/*##################################################################################*/
//Librairie pour manipuler les nombres
#include <math.h>


//Semaphore pour la communication I2C qui est non thread-safe
SemaphoreHandle_t xSemaphore_I2C_Communication = NULL;
/*##################################################################################*/





void setup()
{
/*
  //Initialisation de l'écran
  display.init();
  display.flipScreenVertically();
  display.setFont(Roboto_20);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Initializing, please wait...");
  display.display();
*/
  //Création du Mutex pour les variables d'accélération et d'angle
  xSemaphore_AccelGyro_Data = xSemaphoreCreateMutex();

  //Création du Mutex pour la communication I2C thread-safe
  xSemaphore_I2C_Communication = xSemaphoreCreateMutex();

  //Ouverture des liaisons séries
  Serial.begin(115200);
  while (!Serial)
    vTaskDelay(10); 

  ss.begin(9600);

  vTaskDelay(1000);

  Serial.println("############################################################################################################\n");
  Serial.println("Starting app, please wait...\n");

  Serial.println("*************************************");
  Serial.println("Launching GPS...");

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


  //while( (abs(prev_pos_lat) <= 0.001) && (abs(prev_pos_lng) <= 0.001) && (abs(prev_pos_alt) <= 0.1) ) {
  while(!gps.location.isValid() || !gps.date.isValid() || !gps.time.isValid()) {
    Serial.println("   GPS still inaccurate : ");
    Serial.print("   Location is valid : ");
    Serial.print(gps.location.isValid());
    Serial.print("   Date is valid : ");
    Serial.print(gps.date.isValid());
    Serial.print("   Time is valid : ");
    Serial.print(gps.time.isValid());
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
    delay(1000);
    while (ss.available())
      {
        Serial.println("SS available");
        char c = ss.read();
        gps.encode(c);
      }
    prev_pos_lat = gps.location.lat();
    prev_pos_lng = gps.location.lng();
    prev_pos_alt = gps.altitude.meters();
    sprintf(gps_time_str, "%02u:%02u:%02u", gps.time.hour(), gps.time.minute(), gps.time.second());
    sprintf(gps_date_str, "%02u/%02u/%02u", gps.date.day(), gps.date.month(), gps.date.year());
  }


  sprintf(gps_time_str, "%02u:%02u:%02u", gps.time.hour(), gps.time.minute(), gps.time.second());
  sprintf(gps_date_str, "%02u/%02u/%02u", gps.date.day(), gps.date.month(), gps.date.year());
  
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
  Serial.println("*************************************\n");


/*
  Serial.println("+++++++++++++++++++++++++++++++++++++");
  Serial.println("Setting board time from GPS data");
  date_Years = gps.date.year();
  date_Months = gps.date.month();
  date_Days = gps.date.day();
  time_Hours = gps.time.hour();
  time_Minutes = gps.time.minute();
  time_Seconds = gps.time.second();
  setTime(time_Hours, time_Minutes, time_Seconds, date_Days, date_Months, date_Years);
  sprintf(time_str, "%02u:%02u:%02u", hour(), minute(), second());
  sprintf(date_str, "%02u/%02u/%02u", day(), month(), year());
  Serial.print("Board time : ");
  Serial.println(time_str);
  Serial.print("Board date : ");
  Serial.println(date_str);
  Serial.println("+++++++++++++++++++++++++++++++++++++\n");
*/


  //Initialisation et calibration de l'accéléromètre/gyroscope
  mpu.Initialize();
  
  Serial.println("=====================================");
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
  Serial.println("=====================================\n");

/*
  display.clear();
  display.drawStringMaxWidth(0, 0, 128, "Application launched !");
  display.display();

  delay(2000);
  display.clear();
  display.display();
*/


  //Création de la routine de lecture GPS et affichage des données
  xTaskCreatePinnedToCore(
                    readGPSAndDisplayData,   // Task function. 
                    "readGPSAndDisplayData",     // name of task. 
                    65536,       // Stack size of task 
                    NULL,        // parameter of the task 
                    5,           // priority of the task 
                    NULL,      // Task handle to keep track of created task 
                    0);          // pin task to core 0

    
  
  Serial.println("Application launched ! \n");
  Serial.println("############################################################################################################\n");

}








void displayGPS() {
    sprintf(gps_time_str, "%02u:%02u:%02u", gps.time.hour(), gps.time.minute(), gps.time.second());
    sprintf(gps_date_str, "%02u/%02u/%02u", gps.date.day(), gps.date.month(), gps.date.year());
  
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
    Serial.print(gps_date_str);

/*
    sprintf(time_str, "%02u:%02u:%02u", hour(), minute(), second());
    sprintf(date_str, "%02u/%02u/%02u", day(), month(), year());
    Serial.print("   Board time : ");
    Serial.print(time_str);
    Serial.print("   Board date : ");
    Serial.println(date_str);
*/
  
    double distance_m =
      TinyGPSPlus::distanceBetween(
        gps.location.lat(),
        gps.location.lng(),
        prev_pos_lat,
        prev_pos_lng);
    deniv = prev_pos_alt - gps.altitude.meters();
     /*  
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
    */
    
    //Réduction du bruit sur la position GPS
    if(distance_m < SEUIL_DIST) {
      /*
      Serial.println("  Positions identiques !");
      */
    }
    else {
      if(deniv < 0.0) {
        deniv_neg += deniv;
      }
      else {
        deniv_pos += deniv;
      }
      distance_tot += distance_m;
      
      prev_pos_lat = gps.location.lat();
      prev_pos_lng = gps.location.lng();
      prev_pos_alt = gps.altitude.meters();
    }
    /*
    Serial.print("            Distance totale (m) : ");
    Serial.print(distance_tot, 2);
    Serial.print("   dénivelé positif (m) : ");
    Serial.print(deniv_pos, 2);
    Serial.print("   dénivelé négatif (m) : ");
    Serial.println(deniv_neg, 2);
    Serial.println("");
    */
    Serial.println("");
}







void readGPSAndDisplayData(void * pvParameters) {
  
  double temp_accel_X = 0.0, temp_accel_Y = 0.0, temp_accel_Z = 0.0, temp_angle_X = 0.0, temp_angle_Y = 0.0, temp_angle_Z = 0.0;

  for(;;) {

    
   /*
    while (ss.available() > 0) {
      //Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if(gps.encode(ss.read())) {
        displayGPS();
      }
    }
    */




    
    //TODO: écrire les variables dans le fichier session
    

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

      String angle_str = "Angle   X:" + String(temp_angle_X,2) + " Y:" + String(temp_angle_Y,2) + " Z:" + String(temp_angle_Z,2);
      String accel_str = "Accel   X:" + String(temp_accel_X,2) + " Y:" + String(temp_accel_Y,2) + " Z:" + String(temp_accel_Z,2);

      Serial.println(angle_str);
      Serial.println(accel_str);


/*
      if( xSemaphoreTake( xSemaphore_I2C_Communication, ( TickType_t ) 100 ) == pdTRUE ) {
        //Mutex I2C obtenu, affichage des valeurs d'angle et d'accélération
*/
/*
        display.clear();
        display.drawStringMaxWidth(0, 0, 128, accel_str);
        display.display();
*/

/*
        display.clear();
        display.drawStringMaxWidth(0, 0, 128,
           String(prev_pos_lat, 3) + 
           ";" + String(prev_pos_lng, 3) + "\n" + 
           "alt:" + String(prev_pos_alt, 0) +
           ",sat:" + gps.satellites.value());
        display.display();
*/
/*
        display.clear();
        //String disp_str = String(time_str) + "\n" + String(date_str);
        display.drawStringMaxWidth(0, 0, 128, "coucou");
        display.display();
            
        //Relâche le Mutex une fois l'affichage des valeurs effectué
        xSemaphoreGive( xSemaphore_I2C_Communication );
      }
      else{
        //Impossible d'obtenir le Mutex de communication I2C
        Serial.println("~~~~~~~~~~~~~~~~~~~\n  Mutex I2C indisponible pour afficher les valeurs d'accélération et d'angle\n~~~~~~~~~~~~~~~~~~~");
      }
*/
      delay(100);
  }
}



void loop(){  
  while (ss.available() > 0) {
    //Serial.write(c); // uncomment this line if you want to see the GPS data flowing
    if(gps.encode(ss.read())) {
      displayGPS();
    }
  }

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

     
  /*
  Serial.println("========================");
  Serial.print("            Angle X : ");
  Serial.print(mpu.GetAngX(), 2);
  Serial.print("   Angle Y : ");
  Serial.print(mpu.GetAngY(), 2);
  Serial.print("   Angle Z : ");
  Serial.println(mpu.GetAngZ(), 2);

  Serial.print("            Accel X : ");
  Serial.print(mpu.GetAccX(), 2);
  Serial.print("   Accel Y : ");
  Serial.print(mpu.GetAccY(), 2);
  Serial.print("   Accel Z : ");
  Serial.println(mpu.GetAccZ(), 2);
  Serial.println("========================");
  */
  
  if( xSemaphoreTake( xSemaphore_AccelGyro_Data, ( TickType_t ) 50 ) == pdTRUE ) {
    //Mutex obtenu, mise à jour des valeurs d'angle et d'accélération
    
    
    angle_X = (average_angle_X / AVERAGING_VALUES);
    angle_Y = (average_angle_Y / AVERAGING_VALUES);
    angle_Z = (average_angle_Z / AVERAGING_VALUES);

    accel_X = (average_accel_X / AVERAGING_VALUES);
    accel_Y = (average_accel_Y / AVERAGING_VALUES);
    accel_Z = (average_accel_Z / AVERAGING_VALUES);
    

    /*
    Serial.println("************************");
    Serial.print("            Angle X : ");
    Serial.print(angle_X, 2);
    Serial.print("   Angle Y : ");
    Serial.print(angle_Y, 2);
    Serial.print("   Angle Z : ");
    Serial.println(angle_Z, 2);

    Serial.print("            Accel X : ");
    Serial.print(accel_X, 2);
    Serial.print("   Accel Y : ");
    Serial.print(accel_Y, 2);
    Serial.print("   Accel Z : ");
    Serial.println(accel_Z, 2);
    Serial.println("************************");
    */

    //Relâche le Mutex une fois la mise à jour des valeurs effectuées
    xSemaphoreGive( xSemaphore_AccelGyro_Data );
  }
  else{
    //Impossible d'obtenir le Mutex (logiquement ne devrait pas arriver)
    Serial.println("-------------------\n  Mutex non disponible pour mettre à jour les valeurs d'angle et d'accélération moyennées\n-------------------");
  }
  
  average_angle_X = 0.0;
  average_angle_Y = 0.0;
  average_angle_Z = 0.0;
  average_accel_X = 0.0;
  average_accel_Y = 0.0;
  average_accel_Z = 0.0;
  
   
}
