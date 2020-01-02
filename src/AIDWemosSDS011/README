In Domoticz create three virtual Sensors.
Two 'Custom' virtual sensors for PM10 and PM25 (IDX1 and IDX2).
One 'Temp+Hum' virtual sensor for HTU21D Temp/Hum sensor (IDX3).

For Apeldoorn in data you need to know your AIDid and AIDcode.
If you already have a TTN (The Things Network) device present under the 'ttn_apld_dust_2018' application, 
you can find the id's via:
- AIDid:
  Device EUI (0000000000000099 wil be 99 for AIDid).
- AIDcode:
  Check your node's URL 
  f.i. https://apeldoornindata.nl/data/node.php?id=123 
  Here, the AIDcode will be 123.
  

Update your AIDWemosSDS011.ino with following:

For Domoticz server and virtual sensors:

String DomoticzServer = "<IP address>";
String DSPort = "<Port>";
String PM10IDX = "<IDX1>";
String PM25IDX = "<IDX2>";
String TempHumIDX = "<IDX3>";

For Apeldoorn in Data:

String AIDid = "<ID>";
String AIDcode = "<CODE>";
String AIDlat = "<LAT>";
String AIDlon = "<LON>";
