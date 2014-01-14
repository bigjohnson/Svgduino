/*			          *
 *    Arduino Webserver with      *
 *   Json and Svg image dislpay   *
 *	    10 Jan 2014           *
 *        by Alberto Panu         *
 *      alberto[at]panu.it        *

    copyright Alberto Panu 2014
    
    Ardupower http://www.panu.it/svgduino
    
    Your comments and suggestion are welcomed to alberto[at]panu.it
    
    Released under GPL licence V3 see http://www.gnu.org/licenses/gpl.html and file gpl.txt
*/

// Standart libaryes from arduino 1.0.1
#include <pins_arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// Need extra Flash library from http://arduiniana.org/libraries/flash/
#include <Flash.h>

// Need extra MyTinyWebServer library from http://www.panu.it/wjsonduino/MyTinyWebServer.zip
#include <MyTinyWebServer.h>
// Or if you wont you can use the original library from https://github.com/ovidiucp/MyTinyWebServer
//#include <TinyWebServer.h>

/****************VALUES YOU CHANGE*************/
// pin 4 is the SPI select pin for the SDcard
#define SD_CS 4

// pin 10 is the SPI select pin for the Ethernet
#define ETHER_CS 10

// Enable serial print debug message
//#define DEBUG 1

// The 4 pin is used by SD_CS and for future development is jumped
static byte ioports[] = { 2, 3, 5, 6, 7, 8, 9 };
/* The digital port input are translated
Port 2 is input 0
Port 3 is input 1
Port 5 is input 2
Port 6 is input 3
Port 7 is input 4
Port 8 is input 5
Port 9 is input 6
*/
#define ioportsnum 7
byte ioportsStatus[ioportsnum];
byte actualread = 0;
unsigned int analogValues[6];
byte actualanalog = 0;

// Don't forget to modify the IP to an available one on your home network
byte ip[] = { 192, 168, 0, 5 };
byte gateway[] = { 192, 168, 0, 1 };
byte subnet[] = { 255, 255, 255, 0 };


/*********************************************/
// Substitute with the eth shiel mac address
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

boolean index_handler(MyTinyWebServer& web_server);
boolean json_handler(MyTinyWebServer& web_server);
boolean svg_handler(MyTinyWebServer& web_server);

MyTinyWebServer::PathHandler handlers[] = {
  {"/", MyTinyWebServer::GET, &index_handler },
  {"/json.txt" "*", MyTinyWebServer::GET, &json_handler },
  {"/Image.svg" "*", MyTinyWebServer::GET, &svg_handler },
  {NULL},
};

boolean json_handler(MyTinyWebServer& web_server) {
  web_server.send_error_code(200);
  web_server.end_headers();
  
  web_server << F("processData( { \"digital\" : [ ");

    for (byte i=0; i < ioportsnum; i++) {
       if (ioportsStatus[i]) {
        web_server << F("\"H\"");
       } else {
        web_server << F("\"L\"");
       }
       if ( i < (ioportsnum - 1)) {
         web_server << F(", ");
       }
    }
    
    web_server << F(" ], \"digname\" : [ ");
    for (byte i=0; i < ioportsnum; i++) {
      web_server << F("\"");
      web_server << ioports[i];
      web_server << F("\"");
      if ( i < (ioportsnum - 1)) {
        web_server << F(", ");
      }
    }
    
    web_server << F(" ], \"analog\" : [ ");
    for (byte i=0; i < 6; i++){
      web_server << F("\"");
      web_server << analogValues[i];
      web_server << F("\"");
      if ( i < 5) {
        web_server << F(", ");
      }
    }
    web_server << F(" ] } );\n"); 
    #ifdef DEBUG
      Serial << F("Sent json\n");
    #endif
  return true;
}

boolean index_handler(MyTinyWebServer& web_server) {
  web_server.send_error_code(200);
  web_server.end_headers();
  web_server << F("<html>\n\
<head>\n\
<title>Svgduino</title>\n\
<SCRIPT language=\"JavaScript\">\n\
<!--hide from old browsers\n\
\n\
var ciclo=500;\n\
var tm=1500;\n\
var ma=511;\n\
var mu=2.048;\n\
var it;\n\
var lastexecutiontime;\n\
var ajaxisrunning;\n\
var ph=\"H\";\n\
var pl=\"L\";\n\
var f=\"fill\";\n\
var g=\"green\";\n\
var r=\"red\";\n\
var h=\"height\";\n\
var s=\"H\";\n\
var y=\"y\";\n\
var x=\"X\";\n\
\n\
function prepare(){\n\
 var SVGimage=document.getElementById(\"immagine\");\n\
 var SVGd=SVGimage.getSVGDocument();\n\
 an = new Array();\n\
 for (i=0; i<=5; i++) {\n\
	an[i]=SVGd.getElementById(\"a\"+i);\n\
 }\n\
 laa = new Array();\n\
 for (i=0; i<=5; i++) {\n\
	laa[i]=SVGd.getElementById(\"la\"+i);\n\
 }\n\
 di = new Array();\n\
   for (i=0; i<=6; i++) {\n\
	di[i]=SVGd.getElementById(\"d\"+i);\n\
 }\n\
 lad =  new Array();\n\
    for (i=0; i<=6; i++) {\n\
	lad[i]=SVGd.getElementById(\"ld\"+i);\n\
 }\n\
}\n\
function ajaxObject(url, callbackFunction) {\n\
 var that=this;\n\
 this.updating = false;\n\
 this.abort = function() {\n\
  if (that.updating) {\n\
   that.updating=false;\n\
   that.AJAX.abort();\n\
   that.AJAX=null;\n\
  }\n\
 }\n\
 this.update = function(passData,postMethod) {\n\
 if (that.updating) {return false;}\n\
  that.AJAX = null;\n\
  if (window.XMLHttpRequest) {\n\
   that.AJAX=new XMLHttpRequest();\n\
  } else {\n\
   that.AJAX=new ActiveXObject(\"Microsoft.XMLHTTP\");\n\
  }\n\
  if (that.AJAX==null) {\n\
   return false;\n\
  } else {\n\
   that.AJAX.onreadystatechange = function() {\n\
   if (that.AJAX.readyState==4) {\n\
    that.updating=false;\n\
    that.callback(that.AJAX.responseText,that.AJAX.status,that.AJAX.responseXML);\n\
    that.AJAX=null;\n\
   }\n\
  }\n\
  that.updating = new Date();\n\
  if (/post/i.test(postMethod)) {\n\
   var uri=urlCall+'?'+that.updating.getTime();\n\
   that.AJAX.open(\"POST\", uri, true);\n\
   that.AJAX.setRequestHeader(\"Content-type\", \"application/x-www-form-urlencoded\");\n\
   that.AJAX.setRequestHeader(\"Content-Length\", passData.length);\n\
   that.AJAX.send(passData);\n\
  } else {\n\
   var uri=urlCall+'?'+passData+'&timestamp='+(that.updating.getTime());\n\
   that.AJAX.open(\"GET\", uri, true);\n\
   that.AJAX.send(null);\n\
   }\n\
  return true;\n\
  }\n\
 }\n\
 var urlCall = url;\n\
 this.callback = callbackFunction || function () { };\n\
}\n\
\n\
function processData(JSONData) {\n\
 if (buttonstart.disabled) {\n\
  ajaxisrunning = true;\n\
  avviso.firstChild.data=\"Arduino connected\";\n\
  for (i=0; i<=6; i++) {\n\
    var pn=JSONData.digname[i]\n\
    if (JSONData.digital[i] == s) {\n\
     di[i].setAttribute(f, g);\n\
     lad[i].textContent=\"D\"+pn+\"=\"+ph;\n\
    } else {\n\
      di[i].setAttribute(f, r);\n\
      lad[i].textContent=\"D\"+pn+\"=\"+pl;\n\
    }\n\
  }\n\
  for (i=0; i<=5; i++) {\n\
    laa[i].textContent=\"A\"+i+\"=\"+JSONData.analog[i];\n\
    an[i].setAttribute(h, JSONData.analog[i] / mu);\n\
    an[i].setAttribute(y, 549 - ( JSONData.analog[i] / mu));\n\
    if (JSONData.analog[i] > ma) {\n\
      an[i].setAttribute(f, g);\n\
    } else {\n\
      an[i].setAttribute(f, r);\n\
    }\n\
  }\n\
  \n\
  var d1 = new Date();\n\
  lastexecutiontime=d1.getTime();\n\
  ajaxisrunning = false;\n\
  }\n\
}\n\
\n\
function cambiacolore(c) {\n\
for (i=0; i<=6; i++) {\n\
 di[i].setAttribute(f, c);\n\
 lad[i].textContent=\"   \"+x;\n\
 }\n\
for (var i=0; i<6; i++) {\n\
  laa[i].textContent=\"  A\"+i+\"=\"+x;\n\
  an[i].setAttribute(h, 500);\n\
  an[i].setAttribute(y, 50);\n\
  an[i].setAttribute(f, c);\n\
 }\n\
\n\
}\n\
\n\
function fai() {\n\
\n\
 var d1 = new Date();\n\
 var adesso=d1.getTime();\n\
 var miadifferenza=adesso-lastexecutiontime;\n\
 momento_differenza.firstChild.data=miadifferenza + \" ms from last refresh\";\n\
\n\
 if ( miadifferenza > tm ) {\n\
  cambiacolore(\"yellow\");\n\
  avviso.firstChild.data=\"Arduino connection error!\";\n\
 }\n\
\n\
 var ajaxRequest = new ajaxObject('json.txt');\n\
 ajaxRequest.callback = function (responseText) {\n\
  eval(responseText);\n\
 }\n\
 ajaxRequest.update();\n\
}\n\
\n\
function ferma() {\n\
 clearInterval(it);\n\
 buttonstart.disabled=false;\n\
 while (ajaxisrunning) {\n\
  // aspetta che la funzione di json lanciata da arduino termini e poi cancella il messaggio\n\
 }\n\
 cambiacolore(\"aquamarine\");\n\
 avviso.firstChild.data=\"Suspended app, to run click on Start\";\n\
 momento_differenza.firstChild.data=\"X\";\n\
 buttonferma.disabled=true;\n\
}\n\
\n\
function avvia() {\n\
 var d1 = new Date();\n\
 lastexecutiontime=d1.getTime();\n\
 it = setInterval('fai()',ciclo);\n\
 buttonferma.disabled=false;\n\
 buttonstart.disabled=true;\n\
 messaggio.firstChild.data=\"Refresh time \" + ciclo + \" ms, timeout \" + tm + \" ms\";\n\
}\n\
\n\
function inizia() {\n\
// message.firstChild.data=\"Refresh time \" + ciclo + \" Timeout time \" + tm;\n\
 prepare();\n\
 ferma();\n\
 avvia();\n\
}\n\
//-->\n\
  </SCRIPT>\n\
 </head>\n\
 <body onload=\"javascript:inizia();\">\n\
			<div align=\"center\">\n\
 <h1><a href=\"http://www.panu.it/svgduino\" target=\"_blank\">Arduino json ports status svg graph</a></h1>\n\
			<div id=\"momento_differenza\">milliseconds from last refresh</div>\n\
 <div id=\"avviso\">Suspended app, to run click on Start<div>\n\
		<FORM>\n\
			<INPUT type=\"button\" value=\"Start\" name=\"button1\" onClick=\"javascript:avvia();\" disabled=\"true\" id=\"buttonstart\">\n\
			<INPUT type=\"button\" value=\"Stop\" name=\"button2\" onClick=\"javascript:ferma();\" id=\"buttonferma\">\n\
		</FORM>\n\
                <embed id=\"immagine\" src=\"Image.svg\"></embed>\n\
                <div id=\"messaggio\"> </div>\n\
                Don't exaggerate with the number of client, Arduino has an 8 bit 16 MHz cpu! \n\
		</div>\n\
	</body>\n\
</html>");
  #ifdef DEBUG
    Serial << F("Sent home\n");
  #endif
  return true;
}

boolean svg_handler(MyTinyWebServer& web_server) {
 web_server << F( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<!-- Generator: Adobe Illustrator 15.0.2, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\n\
<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n\
<svg version=\"1.1\" id=\"Livello_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n\
	 width=\"625px\" height=\"700px\" viewBox=\"0 0 625 700\" enable-background=\"new 0 0 625 700\" xml:space=\"preserve\">\n\
<rect id=\"a0\" x=\"100\" y=\"50\" fill=\"#7FFFD4\" width=\"50\" height=\"500\"/>\n\
<rect id=\"a1\" x=\"175\" y=\"50\" fill=\"#7FFFD4\" width=\"50\" height=\"500\"/>\n\
<rect id=\"a2\" x=\"250\" y=\"50\" fill=\"#7FFFD4\" width=\"50\" height=\"500\"/>\n\
<rect id=\"a3\" x=\"325\" y=\"50\" fill=\"#7FFFD4\" width=\"50\" height=\"500\"/>\n\
<rect id=\"a4\" x=\"400\" y=\"50\" fill=\"#7FFFD4\" width=\"50\" height=\"500\"/>\n\
<rect id=\"a5\" x=\"475\" y=\"50\" fill=\"#7FFFD4\" width=\"50\" height=\"500\"/>\n\
<line fill=\"none\" stroke=\"#000000\" x1=\"550\" y1=\"550\" x2=\"75\" y2=\"550\"/>\n\
<line fill=\"none\" stroke=\"#000000\" x1=\"75\" y1=\"550\" x2=\"75\" y2=\"50\"/>\n\
<line fill=\"none\" stroke=\"#000000\" x1=\"550\" y1=\"550\" x2=\"550\" y2=\"50\"/>\n\
<line fill=\"none\" stroke=\"#000000\" x1=\"75\" y1=\"150\" x2=\"550\" y2=\"150\"/>\n\
<line fill=\"none\" stroke=\"#000000\" x1=\"75\" y1=\"50\" x2=\"550\" y2=\"50\"/>\n\
<line fill=\"none\" stroke=\"#000000\" x1=\"75\" y1=\"250\" x2=\"550\" y2=\"250\"/>\n\
<line fill=\"none\" stroke=\"#000000\" x1=\"75\" y1=\"350\" x2=\"550\" y2=\"350\"/>\n\
<line fill=\"none\" stroke=\"#000000\" x1=\"75\" y1=\"450\" x2=\"550\" y2=\"450\"/>\n\
<line fill=\"none\" stroke=\"#000000\" x1=\"75\" y1=\"200\" x2=\"80\" y2=\"200\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-dasharray=\"12,12\" x1=\"83\" y1=\"200\" x2=\"544\" y2=\"200\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-dasharray=\"12,12\" x1=\"83\" y1=\"100\" x2=\"544\" y2=\"100\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-dasharray=\"12,12\" x1=\"83\" y1=\"300\" x2=\"544\" y2=\"300\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-dasharray=\"12,12\" x1=\"83\" y1=\"400\" x2=\"544\" y2=\"400\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-dasharray=\"12,12\" x1=\"83\" y1=\"500\" x2=\"544\" y2=\"500\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-miterlimit=\"10\" stroke-dasharray=\"6,6\" x1=\"83\" y1=\"75\" x2=\"544\" y2=\"75\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-miterlimit=\"10\" stroke-dasharray=\"6,6\" x1=\"83\" y1=\"223\" x2=\"544\" y2=\"223\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-miterlimit=\"10\" stroke-dasharray=\"6,6\" x1=\"83\" y1=\"175\" x2=\"544\" y2=\"175\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-miterlimit=\"10\" stroke-dasharray=\"6,6\" x1=\"83\" y1=\"275\" x2=\"544\" y2=\"275\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-miterlimit=\"10\" stroke-dasharray=\"6,6\" x1=\"83\" y1=\"125\" x2=\"544\" y2=\"125\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-miterlimit=\"10\" stroke-dasharray=\"6,6\" x1=\"83\" y1=\"475\" x2=\"544\" y2=\"475\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-miterlimit=\"10\" stroke-dasharray=\"6,6\" x1=\"83\" y1=\"425\" x2=\"544\" y2=\"425\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-miterlimit=\"10\" stroke-dasharray=\"6,6\" x1=\"83\" y1=\"375\" x2=\"544\" y2=\"375\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-miterlimit=\"10\" stroke-dasharray=\"6,6\" x1=\"83\" y1=\"325\" x2=\"544\" y2=\"325\"/>\n\
<line fill=\"none\" stroke=\"#000000\" stroke-miterlimit=\"10\" stroke-dasharray=\"6,6\" x1=\"83\" y1=\"525\" x2=\"544\" y2=\"525\"/>\n\
<text transform=\"matrix(1 0 0 1 30 60)\" font-size=\"36\">5v</text>\n\
<text transform=\"matrix(1 0 0 1 30 160)\" font-size=\"36\">4v</text>\n\
<text transform=\"matrix(1 0 0 1 30 258)\" font-size=\"36\">3v</text>\n\
<text transform=\"matrix(1 0 0 1 30 360)\" font-size=\"36\">2v</text>\n\
<text transform=\"matrix(1 0 0 1 30 460)\" font-size=\"36\">1v</text>\n\
<text transform=\"matrix(1 0 0 1 30 105)\" font-size=\"18\">4,5v</text>\n\
<text transform=\"matrix(1 0 0 1 30 205)\" font-size=\"18\">3,5v</text>\n\
<text transform=\"matrix(1 0 0 1 30 305)\" font-size=\"18\">2,5v</text>\n\
<text transform=\"matrix(1 0 0 1 30 405)\" font-size=\"18\">1,5v</text>\n\
<text transform=\"matrix(1 0 0 1 30 505)\" font-size=\"18\">0,5v</text>\n\
<text id=\"la0\" transform=\"matrix(1 0 0 1 95 572)\" font-size=\"15\">  A0=X</text>\n\
<text id=\"la1\" transform=\"matrix(1 0 0 1 170 572)\" font-size=\"15\">  A1=X</text>\n\
<text id=\"la2\" transform=\"matrix(1 0 0 1 245 572)\" font-size=\"15\">  A2=X</text>\n\
<text id=\"la3\" transform=\"matrix(1 0 0 1 320 572)\" font-size=\"15\">  A3=X</text>\n\
<text id=\"la4\" transform=\"matrix(1 0 0 1 395 572)\" font-size=\"15\">  A4=X</text>\n\
<text id=\"la5\" transform=\"matrix(1 0 0 1 472 572)\" font-size=\"15\">  A5=X</text>\n\
<rect id=\"d0\" x=\"44\" y=\"600\" fill=\"#7FFFD4\" width=\"68\" height=\"55\"/>\n\
<rect id=\"d1\" x=\"121\" y=\"600\" fill=\"#7FFFD4\" width=\"68\" height=\"55\"/>\n\
<rect id=\"d2\" x=\"200\" y=\"600\" fill=\"#7FFFD4\" width=\"68\" height=\"55\"/>\n\
<rect id=\"d3\" x=\"278\" y=\"600\" fill=\"#7FFFD4\" width=\"68\" height=\"55\"/>\n\
<rect id=\"d4\" x=\"356\" y=\"600\" fill=\"#7FFFD4\" width=\"68\" height=\"55\"/>\n\
<rect id=\"d5\" x=\"433\" y=\"600\" fill=\"#7FFFD4\" width=\"68\" height=\"55\"/>\n\
<rect id=\"d6\" x=\"510\" y=\"600\" fill=\"#7FFFD4\" width=\"68\" height=\"55\"/>\n\
<text id=\"ld0\" transform=\"matrix(1 0 0 1 55 630)\" font-size=\"18\">   X</text>\n\
<text id=\"ld1\" transform=\"matrix(1 0 0 1 133 630)\" font-size=\"18\">   X</text>\n\
<text id=\"ld2\" transform=\"matrix(1 0 0 1 212 630)\" font-size=\"18\">   X</text>\n\
<text id=\"ld3\" transform=\"matrix(1 0 0 1 290 630)\" font-size=\"18\">   X</text>\n\
<text id=\"ld4\" transform=\"matrix(1 0 0 1 368 630)\" font-size=\"18\">   X</text>\n\
<text id=\"ld5\" transform=\"matrix(1 0 0 1 445 630)\" font-size=\"18\">   X</text>\n\
<text id=\"ld6\" transform=\"matrix(1 0 0 1 521 630)\" font-size=\"18\">   X</text>\n\
</svg>" );
    #ifdef DEBUG
      Serial << F("Sent Image\n");
    #endif
  return true;
}

boolean has_ip_address = false;
MyTinyWebServer web = MyTinyWebServer(handlers, NULL);

const char* ip_to_str(const uint8_t* ipAddr)
{
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

void setup() {
  
  #ifdef DEBUG
    Serial.begin(9600);
    Serial << F("Free RAM: ") << FreeRam() << "\n";
  #endif
  
  pinMode(SS_PIN, OUTPUT);	// set the SS pin as an output
         // (necessary to keep the board as
         // master and not SPI slave)
  digitalWrite(SS_PIN, HIGH);	// and ensure SS is high

  // Ensure we are in a consistent state after power-up or a reset
  // button These pins are standard for the Arduino w5100 Rev 3
  // ethernet board They may need to be re-jigged for different boards
  pinMode(ETHER_CS, OUTPUT);	// Set the CS pin as an output
  digitalWrite(ETHER_CS, HIGH);	// Turn off the W5100 chip! (wait for
         // configuration)
  pinMode(SD_CS, OUTPUT);	// Set the SDcard CS pin as an output
  digitalWrite(SD_CS, HIGH);	// Turn off the SD card! (wait for
         // configuration)

  for (byte i=0; i < ioportsnum; i++) {
    pinMode(ioports[i], INPUT_PULLUP);
  }
    
  // Initialize the Ethernet.
  #ifdef DEBUG
    Serial << F("Setting up the Ethernet card...\n");
  #endif
  
  Ethernet.begin(mac, ip, gateway, subnet);

  // Start the web server.
  #ifdef DEBUG
    Serial << F("Web server starting...\n");
  #endif  
  web.begin();
  #ifdef DEBUG
    Serial << F("Ready to accept HTTP requests.\n\n");
  #endif
}

void loop() {
  ioportsStatus[actualread] = digitalRead(ioports[actualread]);
  actualread++;
  if (actualread >= ioportsnum) {
    actualread = 0;
  }
  
  unsigned int midread = 0;
  for (byte i=0; i < 10; i++) {
    midread = midread + analogRead(actualanalog);
  }
  analogValues[actualanalog] = midread / 10;
  actualanalog++;
  if (actualanalog > 5) {
    actualanalog = 0;
  }
  web.process();
}
