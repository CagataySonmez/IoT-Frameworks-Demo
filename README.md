# IoT-Frameworks-Demo
This repository is prepared for CMPE 490 Internet of Things lecture:
https://www.cmpe.boun.edu.tr/courses/cmpe490

In my lecture (04.04.2017), AllJoyn and IoTivity open source IoT programming frameworks are explained.
You can find source code of the example applications explained in the lecture in this repository.

_**Compilation instruction for AllJoyn Android app**_

   `1- Open Android Studio`  
   `2- Apply File -> New -> Import Project`  
   `3- Chose simple_client_alljoyn folder`  

Please note that you should download AllJoyn sorce code and compile it for Android to generate Android bindigs (alljoyn.jar & liballjoyn_java.lib)
In this example, it is already compiled for Android platform.
If you want to compile it again, please read instructions given in the below address:
https://allseenalliance.org/framework/documentation/develop/building/android/build-source


_**Compilation instruction for AllJoyn Java app**_

   `1- Open Eclipse`  
   `2- Apply File -> New -> Java Project`  
   `3- uncheck Ese Default Location option`  
   `4- Chose simple_service_alljoyn folder`  

In order to configure required libraries, follow the below steps:

   `1- Go to the sample project properties`  
   `2- Select Java Build Path`  
   `3- Select the Libraries tab`  
   `4- Click on the line "Native library location"`  
   `5- Click the "Edit..." button`  
   `6- For windows, put in the path for the alljoyn_java.dll file`  
   `7- For linux,  put in the path for the liballjoyn_java.so file`  

Please note that you should download AllJoyn sorce code and compile it for java to generate java bindigs (alljoyn.jar & liballjoyn_java.lib)
In this example, it is already compiled for x86_64 platform.
If you want to compile it again, please read instructions given in the below address:
https://allseenalliance.org/framework/documentation/develop/building/linux/build-source


_**Compilation instruction for IoTivity Linux apps**_

   `1- Open terminal`  
   `2- go Linux\simple_client-service_iotivity\app`  
   `3- Type make`  

Please note that you should download IoTivity sorce code and compile it for Linux to generate related libraries.
In this example, it is already compiled for Linux x86_64 and x86_32 platforms.
If you want to compile it again, please read instructions given in the below address:
https://wiki.iotivity.org/build_iotivity_with_ubuntu_build_machine

Good luck.
