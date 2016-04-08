CC=/home/lubuntu/rpi/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi-gcc
CCXX=/home/lubuntu/rpi/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/bin/arm-bcm2708hardfp-linux-gnueabi-g++
APP_BINARY=SmartCar
APP_ARGS=''
#REMOTE_SERVER=pi@robot11d
REMOTE_SERVER=pi@192.168.1.11
REMOTE_PATH=/home/pi
#REMOTE_PASS='raspberry'
REMOTE_PASS='thomas'
ML=/home/lubuntu/minimod/Libs/
ML_ROB=/home/lubuntu/robotica/lib/
ML_RPI=/home/lubuntu/rpi-cp/
LOGFILE='log.csv'

CFLAGS += -g -DLINUX -Wall -I$(ML) -I$(ML_ROB) -I. -I$(ML_RPI)usr/local/include
LFLAGS += -lpthread -Wl,-unresolved-symbols=ignore-in-shared-libs,-rpath,/usr/local/lib,-rpath,$(ML_RPI)usr/local/lib,-rpath,$(ML_RPI)usr/local/arm-linux-gnueabihf -L$(ML) -L/usr/local/lib -L$(ML_RPI)usr/local/lib -lopencv_core -lopencv_features2d -lopencv_highgui -lrt -lopencv_imgproc -lopencv_photo -lopencv_ml -lopencv_imgcodecs -lopencv_video -lopencv_videoio -lcurl

OBJECTS=main.o robotapp.o smartcore.o $(ML_ROB)timestep.o $(ML_ROB)TimeSupport.o $(ML)lin-rpi-serlib.o $(ML)lin-rs485client.o $(ML)crc.o $(ML)rf-cc1101.o $(ML)lego-motor.o $(ML)lego-sensor.o $(ML)lin-delay.o $(ML)lin-gpio.o $(ML)lin-lego-motor-log.o $(ML)pwr-liion1a.o $(ML)ansi.o $(ML)imu.o travel.o map.o dijkstra.o drivequeue.o msgqueue.o drive.o trafficlightdetect.o camera.o restInterface.o json.o watchdog.o speaker.o

all: SmartCar
Debug: SmartCar
SmartCar: $(OBJECTS)
	$(CCXX) $(CFLAGS) $(LFLAGS) $(OBJECTS) -o $(APP_BINARY)

-include $(OBJECTS:.o=.d)

#Compile rule for C-code files
%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d

#Compile rule for C++-code files
%.o: %.cpp
	$(CCXX) -c $(CFLAGS) $*.cpp -o $*.o
	$(CCXX) -MM $(CFLAGS) $*.cpp > $*.d

cleanDebug: clean
clean:
	rm -f $(OBJECTS)
	rm -f $(OBJECTS:.o=.d)
	rm -f $(APP_BINARY)
install:
	chmod +x $(APP_BINARY)
	sshpass -p $(REMOTE_PASS) scp -o StrictHostKeyChecking=no  $(APP_BINARY) $(REMOTE_SERVER):$(REMOTE_PATH)
rundebug:
	sshpass -p $(REMOTE_PASS) ssh -t $(REMOTE_SERVER)  "sudo gdbserver localhost:5000 $(REMOTE_PATH)/$(APP_BINARY) $(APP_ARGS)"
runnodebug:
	sshpass -p $(REMOTE_PASS) ssh -t $(REMOTE_SERVER) sudo $(REMOTE_PATH)/$(APP_BINARY) $(APP_ARGS)
plotlog:
	rm -f $(LOGFILE)
	sshpass -p $(REMOTE_PASS) scp -o StrictHostKeyChecking=no  $(REMOTE_SERVER):$(REMOTE_PATH)/$(LOGFILE) .
	octave --persist logplot.m
plotdelete:
	sshpass -p $(REMOTE_PASS) ssh -t $(REMOTE_SERVER) rm -f $(REMOTE_PATH)/$(LOGFILE)


