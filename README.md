26/01/18:
Current sensors'state : 
-Uno sensor which represents the node : 
  * first the node is waiting for date and hour synchronization : when the node is plugged to battery a msg is sent to be synchronized and nothing will work until the date hasn't been received. 
  * Once synchronized, every 30sec the node measures its temperature and its gaz level. 
  * After 2 measures so every 1 min the node sends its msg through LoRA to the well.
  * Delays and number of measures can be changed but one must be careful because LoRa msg must be really short
  
-Yun sensor which represents the well : 
  * first the well needs to connect to a wifi network to send data to the server, right now the yun is connected to Lisa's phone. After few failing attemps the wifi of the yun will get back to it's original configuration : to change this configuration and connect the yun to another wifi you must access the yun web page arduino.local and enter the passord :"arduino".
  * once the yun is connected to a wifi, the well waits for any enquiry from a node through LoRa protocol. It can be either a synchronized request or data sent. 
  * Once data are received from the node, data are first parsed and checked before creating the json msg to send to the server
  * Many tests have been made to create correct Json from data received by hand or through arduino libraries but unfortunately several issues have been encountered and no hypotheses could have been found to solve them. So that's why each json countains only one measure to send to the server either one temperature or either one atmosphere measure. From the yun if several measures are realized in a loop, a json containg an array with all these measures can be sent without any problem. However it appears that when the data is coming from LoRa msg the json containing an array with several temperatures measures isn't sent correctly or isn't received by the server correctly.
  * The current code is pretty shitty we admit but at meast it works as we need to. 
  * On yun it has been impossible to plug the gaz sensor. We don't know why.
  
  
Improvements : 
- The most important one is to understand why the json containing several measures cannot be sent correctly.
_____________________________________________________________________________________________________________________
Hoomie Sensors code for arduino Uno and YUN :
Any Added Libraires are accessible in librairies directory.
Any code dedicated for the yun is accessible in yun directory.


______________________________________________________________________________________________________________________
Old sensors code on msp and raspberry are still accessible in oldSensors branch :


To launch the python file on the rapsberry : stay in this directory

1. connect the msp dongle
2. wait for dev : ttyACM0 to be mounted (you can check with ls /dev)
3. unmount module cdc_acm using : sudo rmmod cdc_acm
(you can use the cmd : dmesg to see the log)
4. you can test if ezconsole works using this cmd: sudo ./ezconsole/ezconsole_bin_raspberry
5. if you don't see any error the msp is fully recognized .

be careful if an error appears in ezconsole you need to close the terminal manually CTRL-C or CTRL-D don't work.
Finally try to not touch the keyboard you might break ezconsole or the transmission process through changing sensors ID.
