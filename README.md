
Hoomie Sensors code for arduino Uno and YUN :
Any Added Libraires are accessible in librairies folder.
Any code dedicated for the yun is accessible in yun folder.


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
