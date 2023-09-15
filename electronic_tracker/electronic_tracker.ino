// Déclaration des bibliothèques utilisées
#include "gps.h"
#include <SoftwareSerial.h>
SoftwareSerial gprsSerial(2,3);
#include <String.h>


#define LED_GREEN 9
#define LED_RED 10
#define LED_BLUE 11



// Définition des types
typedef enum
{
    PAS_DE_REPONSE = -1,
    ETEINT = 0,
    RECHERCHE = 1,
    TROUVE_2D = 2,
    TROUVE_3D = 3
} etat_gps_t;

typedef struct
{
  bool positif;
  uint8_t degree;
  uint8_t minute;
  float seconde;
} sexagesimale_t;

// Prototypes de fonction
void initialiser_Sim808(void);
void demarrer_gps(void);
void arreter_gps(void);
bool lire_informations_gps(sexagesimale_t*, sexagesimale_t*,sexagesimale_t*);
sexagesimale_t gps_brute_vers_sexagesimal(char*);
void donner_coordonees_en_texte(char*, sexagesimale_t, sexagesimale_t);
/*void donner_lien_google_map(char*, sexagesimale_t, sexagesimale_t);
void donner_lien_waze(char*, sexagesimale_t, sexagesimale_t);*/
float sexagesimale_vers_degres_decimaux(sexagesimale_t);
void flottant_vers_texte(char*, float);

// Déclarations globales
GPSGSM  *gestionnaire_gps;




// Fonction de démarrage, s'exécute une seule fois:
void setup()
{
  // Ouverture du port USB pour l'envoi des traces au PC
  Serial.begin(9600);

  
  connectToNetwork();
      delay(2000);

  // Positionnement sortie du port de la diode interne
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // La diode éteinte signale que le GPS n'est pas disponible

  // Initialisation de la communication avec la carte SIM808
  initialiser_sim808();  
  // Initialisation du modem GPS

  delay(400);
  demarrer_gps();



}




// Fonction principale du programme, s'exécute en boucle:
void loop()
{
  sexagesimale_t sexa_latitude, sexa_longitude, sexa_vitesse;
  char tampon_message[90];

  Serial.print('\n'); // Saut d'une ligne pour une meilleur visibilité
  if(lire_informations_gps(&sexa_latitude, &sexa_longitude, &sexa_vitesse))
  {
    // Calcul des coordonnées géographiques:
   // donner_coordonees_en_texte(tampon_message, sexa_latitude, sexa_longitude, sexa_vitesse);
    Serial.print(F("\nCoordonnées géographiques: "));    
    Serial.println(tampon_message);
    sendData(sexa_latitude, sexa_longitude,sexa_vitesse);
  /*  // Calcul du lien Google Map:
    donner_lien_google_map(tampon_message, sexa_latitude, sexa_longitude);
    Serial.print(F("Lien google Map: "));    
    Serial.println(tampon_message);

    // Calcul du lien Waze:
    donner_lien_waze(tampon_message, sexa_latitude, sexa_longitude);
    Serial.print(F("Lien Waze: "));    
    Serial.println(tampon_message);*/

    // Pause de  10 secondes
    delay(1000);
  }
  else
  {
    Serial.println(F("Erreur de lecture GPS"));  
  }
}



void upRed(){
  digitalWrite(LED_BLUE,0);
  digitalWrite(LED_GREEN,0);
  digitalWrite(LED_RED,1);
  
  }

void upBlue(){
  digitalWrite(LED_RED,0);
  digitalWrite(LED_GREEN,0);
  digitalWrite(LED_BLUE,1);
  
  }
void upGreen(){
digitalWrite(LED_RED,0);
digitalWrite(LED_BLUE,0);
digitalWrite(LED_GREEN,1);
}

//blink red
void bRed(){
  int i=0;
  for(i=0;i<3;i++){
      digitalWrite(LED_BLUE,0);
      digitalWrite(LED_GREEN,0);
      digitalWrite(LED_RED,1);
      delay(100);
      digitalWrite(LED_RED,0);
  }  
  }

//blink red
void bBlue(){
  int i=0;
  for(i=0;i<3;i++){
      digitalWrite(LED_RED,0);
      digitalWrite(LED_GREEN,0);
      digitalWrite(LED_BLUE,1);
      delay(100);
      digitalWrite(LED_BLUE,0);
  }  
  }
//blink Green
void bGreen(){
  int i=0;
  for(i=0;i<3;i++){
      digitalWrite(LED_RED,0);
      digitalWrite(LED_BLUE,0);
      digitalWrite(LED_GREEN,1);
      delay(100);
      digitalWrite(LED_GREEN,0);
  }  
  }








void initialiser_sim808(void)
{
  Serial.println(F("Connexion avec la carte SIM808."));  
  while(!gsm.begin(9600))
  {
    Serial.println(F("Echec de communication avec la carte SIM808. Nouvelle tentative..."));  
  }
  Serial.println(F("La communication avec la carte SIM808 est établie."));
}




void demarrer_gps(void)
{
  etat_gps_t etat_gps;
    
  if(gestionnaire_gps!=NULL)
  {
    free(gestionnaire_gps);
  }
  gestionnaire_gps = new GPSGSM();
  if (!gestionnaire_gps->attachGPS())
  {
    Serial.println(F("Impossible d'activer le GPS."));
  }
  else
  {
    Serial.println(F("Initialisation du GPS."));
    do
    {
      digitalWrite(LED_BUILTIN, HIGH);  // Clignotement de la diode pour signaler la recherche GPS
      etat_gps = (etat_gps_t)gestionnaire_gps->getStat();
      Serial.print(F("."));
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);  // Clignotement de la diode pour signaler la recherche GPS
    }
    while(etat_gps == RECHERCHE);
    if (etat_gps == TROUVE_2D)    
    {
      Serial.println(F("\nGPS initialisé en 2D."));
      digitalWrite(LED_BUILTIN, HIGH);  // La diode allumée signale que le GPS est prêt
    }
    else if(etat_gps == TROUVE_3D)
    {
      Serial.println(F("\nGPS initialisé en 3D."));
      digitalWrite(LED_BUILTIN, HIGH);  // La diode allumée signale que le GPS est prêt
    }
    else
    {
      Serial.println(F("\nImpossible d'initialiser le GPS."));     
    }
  }

}




void arreter_gps(void)
{   
  if(gestionnaire_gps==NULL)
  {
    Serial.println(F("Le GPS est déjà éteint."));
  }
  else
  {
    if (!gestionnaire_gps->deattachGPS())
    {
      Serial.println(F("Impossible d'éteindre le modem GPS."));
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);  // La diode éteinte signale que le GPS est arrêtée
      Serial.println(F("\nGPS éteint."));
    }
  }
}




bool lire_informations_gps(sexagesimale_t* latitude_p, sexagesimale_t* longitude_p,sexagesimale_t* vitesse_p)
{
  etat_gps_t etat_gps;

  bool informations_gps_lues = false;
  char longitude_brute[15];
  char latitude_brute[15];
  char altitude_brute[15];
  char heure_brute[20];
  char vitesse_brute[15];

  etat_gps = (etat_gps_t)gestionnaire_gps->getStat();
  if((etat_gps != TROUVE_2D)&&(etat_gps != TROUVE_3D))
  {
    demarrer_gps();  
  }
  
  Serial.println(F("Lecture des information gps"));
  informations_gps_lues = gestionnaire_gps->getPar(latitude_brute, longitude_brute, altitude_brute, heure_brute, vitesse_brute);

  if(informations_gps_lues)
  {
    Serial.println(F("Informations gps lues"));
    *latitude_p = gps_brute_vers_sexagesimal(latitude_brute);
    *longitude_p = gps_brute_vers_sexagesimal(longitude_brute);
    *vitesse_p = gps_brute_vers_sexagesimal(vitesse_brute);

  }
  
  return(informations_gps_lues);
}





sexagesimale_t gps_brute_vers_sexagesimal(char* coordonnee_brute)
{
  sexagesimale_t sexagecimale;
  double coordonnee_dble;
  
  coordonnee_dble = atof(coordonnee_brute);
  
  // Détermination du signe +/- de la coordonnée
  sexagecimale.positif = (coordonnee_dble>=0);
  coordonnee_dble = fabs(coordonnee_dble);
  
  // Calcul des degrés
  sexagecimale.degree = ((int)coordonnee_dble) / 100;
  coordonnee_dble = coordonnee_dble - sexagecimale.degree * 100;
  
  // Calcul des minutes
  sexagecimale.minute = (int)coordonnee_dble;
  coordonnee_dble = coordonnee_dble - sexagecimale.minute;
  
  // Calcul des secondes
  sexagecimale.seconde = 60 * coordonnee_dble;
  
  return(sexagecimale);
}



void donner_coordonees_en_texte(char* position_texte, sexagesimale_t latitude, sexagesimale_t longitude)
{
  char signe_latitude;
  char signe_longitude;
  float partie_entiere;

  if(latitude.positif)
  {
    signe_latitude = 'N';
  }
  else
  {
    signe_latitude = 'S';
  }
  if(longitude.positif)
  {
    signe_longitude = 'E';
  }
  else
  {
    signe_longitude = 'O';
  }
  sprintf(position_texte, "%d°%d'%02d.%02d\"%c, %d°%d'%02d.%02d\"%c", latitude.degree, latitude.minute , int(latitude.seconde),  int(modff(latitude.seconde, &partie_entiere)*100), signe_latitude,
                                                                     longitude.degree, longitude.minute , int(longitude.seconde),  int(modff(longitude.seconde, &partie_entiere)*100), signe_longitude);    
}




float sexagesimale_vers_degres_decimaux(sexagesimale_t coordonnee)
{
  float valeur;
  
  valeur = float(coordonnee.degree) + float(coordonnee.minute) / 60.0 + float(coordonnee.seconde) / 3600.0 ;
  
  if(!coordonnee.positif)
  {
    valeur = valeur * -1;
  }
  return(valeur);
}




void flottant_vers_texte(char* texte, float valeur)
{
  float partie_entiere, partie_fractionnaire;

  partie_fractionnaire = modff(valeur, &partie_entiere);
  sprintf(texte,"%d.%ld", (int)partie_entiere, (long)abs(partie_fractionnaire*pow(10, 7)));
}


///** DEBUT FONCTION GSM DU MODULE**////

void connectToNetwork(){

  gprsSerial.begin(9600);               // the GPRS baud rate   


    
 
  gprsSerial.println("AT");
  delay(400);
 
  gprsSerial.println("AT+CPIN?");
  delay(400);
 
  gprsSerial.println("AT+CREG?");
  delay(400);
 
  gprsSerial.println("AT+CGATT?");
  delay(400);
 
  gprsSerial.println("AT+CIPSHUT");
  delay(400);
 
  gprsSerial.println("AT+CIPSTATUS");
  delay(2000);
 
  gprsSerial.println("AT+CIPMUX=0");
  delay(2000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CSTT=\"www.orange.com\"");//start task and setting the APN,
  delay(1000);
  
}





void sendData(sexagesimale_t la, sexagesimale_t lo, sexagesimale_t spd){
  
  Serial.println("TEST COMMUNICATION WITH THINGSPEAK");


  char t_lat[15], t_lon[15], t_spd[15];

  flottant_vers_texte(t_lat, sexagesimale_vers_degres_decimaux(la));
  flottant_vers_texte(t_lon, sexagesimale_vers_degres_decimaux(lo));
  flottant_vers_texte(t_spd, sexagesimale_vers_degres_decimaux(spd));  
      
  if (gprsSerial.available()){
    Serial.write(gprsSerial.read());
  }

 
  ShowSerialData();
 
  gprsSerial.println("AT+CIICR");//bring up wireless connection
  delay(1000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(2000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSPRT=0");
  delay(1000);
 
  ShowSerialData();
  
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(3000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  delay(2000);
  ShowSerialData();

  String t_lats=t_lat;
  String t_lons=t_lon;
  String t_spds=t_spd;
  
  String str="GET https://api.thingspeak.com/update?api_key=4A9C2GERM2G0AZJ0&field1="+t_lats+"+&field2="+t_lons+"&field3="+t_spds; /*   /?la="+t_lats+"&lo="+t_lons+"&spd="+t_spds+"";  */
  Serial.println(str);
  gprsSerial.println(str);//begin send data to remote server 
  
  delay(3000);
  ShowSerialData();
 
  gprsSerial.println((char)26);//sending
  //delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  gprsSerial.println();
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData();
  
  
  }


  

void ShowSerialData()
{
  while(gprsSerial.available()){
    Serial.write(gprsSerial.read());
  }

  delay(400); 
  
}








///** FIN FONCTION GSM DU MODULE**////
