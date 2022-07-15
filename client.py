import bluetooth as bt
import time
import serial

name = "HC-05"
port = 1
addr = ""
passkey = "1234"
done = False
nearby_devices = bt.discover_devices(lookup_names=True)
print("Found {} devices.".format(len(nearby_devices)))
print(nearby_devices)


for device in nearby_devices:
    if (device[1] == name):
        addr = device[0]

if (addr == ""):
    print("Target not found")
else:
    print("Target found ", addr)

try:
    s = bt.BluetoothSocket(bt.RFCOMM)
    s.connect((addr, port))
    print("Connected")
    s.send("init")
    
    while (not done):
        s.send("test")
        print("sending")
        time.sleep(5)




except bt.btcommon.BluetoothError as err:
    print("Bozo")
    pass
