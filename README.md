RobotDriver
============

The Robot Driver is the low-level driver program developed in C which runs on the SmartCars in the SmartCity.
This driver module is a part of the SmartCity Project. The SmartCar is an embedded robot vehicle designed and
build by Professor Paillet Eric of the University of Antwerp. The SmartCar is equiped with Lego Mindstorm
actuators and sensors. These are connected with custom PCBs to a Raspberri Pi for processing.

The low-lever driver offers an interface to receive commands, called tasks to execute. All actions are asynchronously
which allows to perform all tasks in parallel or to interrupt a running task. The SmartCars currently supports the
following commands: Drive, Camera, Tag-Reader, Lift and Speaker commands. A detailed list of all commands can be
found in the instructionset file in the repository.


Developed by
============

Huybrechts Thomas,
Janssens Arthur,
Vervliet Niels

University of Antwerp - 2016
