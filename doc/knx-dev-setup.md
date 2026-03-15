**ACHTUNG:**
Es handelt sich hier nicht um eine offizielle KNX ETS Firmware !!! 

Diese Firmware basiert im KNX Teil auf der Arbeit von: [https://github.com/thelsing](https://github.com/thelsing) und im EnOcean Teil auf der Konnekting Firmware [https://gitlab.com/konnekting/firmware/eo-gw1.1](https://gitlab.com/konnekting/firmware/eo-gw1.1)

Der Einsatz dieser Firmware entspricht in keinster Weise den Vorgaben der KNX-Association !!!  

--> Jeder baut sich seine Firmware selber und übernimmt damit selber die Verantwortung für sein Handeln

-------------------------------------------------

## How to build a knxprod for this firmware

Open [https://github.com/mumpf/multiply-channels/releases](https://github.com/mumpf/multiply-channels/releases)

Download the newest release of multiply-channels, currently it is version 2.1.2.

The executable is MultiplyChannels.exe

Save it to C:\Users\\\<username>\bin (usually you have to create bin directory)

If this is not your ETS-PC, install ETS5 on this PC (ETS5.7.x demo is sufficient, even any 5.6.x should do)

The next step you can do first after progessed the "Installation of dev-Environment for Enocean-Gateway":

Go to the Visual Studio Code instance, which is containing the knx-enocean-gateway project

Press Ctrl-Shift-P, enter "run test task" and click the appearing "Tasks: Run Test Task"

In the following dropdown select "**MultiplyChannels knx-enocean-gateway**"

Wait for the success message in the terminal window

The freshly build

* EnoceanGateway.knxprod

you will find in the release directory of the knx-enocean-gateway project

You can import this knxprod in your ETS (minimum 5.6) like any other knxprod.

# Installation of dev-Environment for Enocean-Gateway

Only tested on Windows 10!

Download and install git from [https://git-scm.com/downloads](https://git-scm.com/downloads) with default options

Download and install visual studio code from [https://code.visualstudio.com/download](https://code.visualstudio.com/download) (User installer, 64 bit)

Start visual studio code

Go to extensions (Ctrl-Shift-X)

Enter "platformio" in search field

Install "PlatformIO IDE" extension

Wait until installation is finished, do the necessary reload window afterwards (may take some time)

Click on the new PlatformIO-Icon on the left ![PIO-Icon](pic/PIO2.png)

In "Quick Access", choose open

In the new "PIO Home" tab, click on "New Project..."

In the upcoming dialog, provide the name "Test", Board "Seeeduino Xiao", Framework "Arduino" and Location "Use default location"

Click "Finish" and wait until finished. Visuals Studio Code will open the newly created project afterwards. The new project is just used to create default envoronment and can be deleted afterwards.

Click again the PlatformIO Icon ![PIO-Icon](pic/PIO2.png)

Again "Quick Access" appears, click "Miscellaneous->PlatformIO Core CLI"

A new terminal (within Visual Studio Code) appears, the path is home of the new test project. We don't need the test project, it was just used to create all necessary path for development.
From now on we work in this terminal window:

    cd .. 

You should be now in a directory ending with ...\Documents\PlatformIO\Projects

    git clone https://gitlab.com/DIY-Smarthome-Projekt/knx-enocean-gateway.git
    git clone https://github.com/mumpf/knx.git
    git clone https://github.com/mumpf/knx-common.git
    git clone https://github.com/mumpf/knx-logic.git

    cd knx
    git checkout release
    cd ..\knx-common
    git checkout release
    cd ..\knx-logic
    git checkout release
    cd ..\knx-enocean-gateway
    git checkout release
    code Enocean.code-workspace
      
Now a new instance of Visual Studio Code is started. You can close the other (previous) instance.

**create KNXprod**

Press Ctrl-Shift-P, enter "run test task" and click the appearing "Tasks: Run Test Task"

In the following dropdown select "MultiplyChannels knx-enocean-gateway"

Wait for the success message in the terminal window

The freshly build

* EnoceanGateway.knxprod

you will find in the release directory of the knx-enocean-gateway project (in the SRC- Folder)

**Firmware Build**

Press Ctrl-Shift-B, select the **Build PlatformIO** build task and press enter.

Now the compiler starts, this may take a while, there will be many yellow warnings, they can be ignored.

At the end, there should be a message like

    Linking .pio\build\build\firmware.elf
    Building .pio\build\build\firmware.bin
    Checking size .pio\build\build\firmware.elf
    Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
    RAM:   [==        ]  XX.X% (used XXXX bytes from 32768 bytes)
    Flash: [======    ]  XX.X% (used XXXXXX bytes from 262144 bytes)
    ============================ [SUCCESS] Took XX.XX seconds ======

Now you successfully build the Firmware for the Module.

Precompiled firmware versions are not released anymore, you have always to compile your own.

## How to upload the Firmware to your Hardware

Connect your device via USB to your PC

**Attention:** before you upload the code and the Serial Monitor is active, please always disable the "PlatformIO:Serial Monitor". for That press the "Plug" Symbol on the buttom blue line and press "Terminate Task" in the pop Up window.  

Press Ctrl-Shift-B, select **Upload USB** build task and press enter.

Wait until file is uploaded.

## Programming with ETS

This works the same way as with all other KNX devices. For the initial programming you should program the physical address (PA) first, then transfer the application program (do not use ETS function "PA + Application program").

Afterwards you can use partial programming as usual.
