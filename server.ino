extern "C" {
#include <AES.h>
#include <uECC.h>
};
#include <ESP8266WiFi.h>

AES aes ;

char ssid[] = "";               // SSID of your home WiFi
char pass[] = "";               // password of your home WiFi
WiFiServer server(80);                    

IPAddress ip(192, 168, 43, 80);            // IP address of the server
IPAddress gateway(192, 168, 43, 26);           // gateway of your network
IPAddress subnet(255, 255, 255, 0);          // subnet mask of your network

  
byte plain[] = "retro"; // plaintext to encrypt
 
unsigned long long int myIv = 36753562; // CBC initialization vector; real iv = iv x2 ex: 01234567 = 0123456701234567
 
void setup ()
{
  Serial.begin (115200) ;
   WiFi.config(ip, gateway, subnet);       // forces to use the fix IP
  WiFi.begin(ssid, pass);                 // connects to the WiFi router
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  server.begin();                         // starts the server
}
 
void loop () 
{
  int bits = 128;
  byte iv [N_BLOCK] ;
  int plainPaddedLength = sizeof(plain) + (N_BLOCK - ((sizeof(plain)-1) % 16)); // length of padded plaintext [B]
  byte encrypted [plainPaddedLength]; // ciphertext (encrypted plaintext)
  byte decrypted [100]; // decrypted plaintext
  byte private1[20], private2[20], public1[40], public2[40], secret1[20] ; // initialise the variables 

  // generate the ECC key
  
  uECC_make_key(public1, private1, curve);  
  uECC_make_key(public2, private2, curve);
  int r = uECC_shared_secret(public2, private1, secret1, curve);

  // Encrypt the message with the help of AES encryption
      
  const struct uECC_Curve_t * curve = uECC_secp160r1();
  aes.set_IV(myIv);
  aes.get_IV(iv);
  aes.do_aes_encrypt(plain, sizeof(plain), encrypted, secret1, bits, iv); 
 
  String answer = String((char*)encrypted); // convert the encrypted to string for sending it

  // establish the TCP connection and send the message
  
  WiFiClient client = server.available();  
  if (client) {
    if (client.connected()) {
      Serial.println(".");
      client.println(answer + '\r'); // sends the answer to the client
    }
    client.stop();                // terminates the connection with the client
  }
}
