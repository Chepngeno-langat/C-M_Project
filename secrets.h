#include <pgmspace.h>
 
#define SECRET
 
const char WIFI_SSID[] = "#####";               
const char WIFI_PASSWORD[] = "#######";      
 
#define THINGNAME "demothing"
 
int8_t TIME_ZONE = +3; //NYC(USA): -5 UTC
 
const char AWS_IOT_ENDPOINT[] = "#########################";
 
 
static const char AWS_IOT_ROOT_CA_1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
######################################
-----END CERTIFICATE-----
)EOF";
 
 
// Copy contents from XXXXXXXX-certificate.pem.crt here ▼
static const char AWS_IOT_CERTIFICATE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
###############################
-----END CERTIFICATE-----

 
)KEY";
 
 
// Copy contents from  XXXXXXXX-private.pem.key here ▼
static const char AWS_IOT_PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
###################################
-----END RSA PRIVATE KEY-----
 
)KEY";
