# TC_Tummies_21
# Greenhouse Monitoring system

This Github repository was made to host the software side of the technical solution for TC team Tummies for the SS2021 session of tech challenge organized by UnternehmerTum

## Description

The main script is a OPC UA server collecting sensor data using I2C and MQTT and making them available to read for clients which is the SUSHI device in the case of this challenge.
## Getting Started

### Dependencies

This project uses the following libraries:
* [open62541](https://github.com/open62541/open62541) for OPC UA
* wiringPi (included by default in the Raspberry Pi OS) for I"C communication
* [mosquitto](https://github.com/eclipse/mosquitto) for mqtt
* [jsmn](https://github.com/zserge/jsmn) for JSON messages parsing
All libraries must be installed for the program to work with the jsmn library to be cloned in the same folder where the files in this repo are

### Compiling

To compile the code all you need to do is compile the myServer.c script (using GCC for example)
```
gcc myServer.c -o myServer -lopen62541 -lwiringPi -lmosquitto
```


### Executing program

To exectute the program just run the previously compiled script
```
./myServer
```

## Authors

[Wafa Laroussi](mailto:wafalaroussi@gmail.com)
