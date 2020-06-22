import serial
import time

import paho.mqtt.client as paho
mqttc = paho.Client()


serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)
s.write("+++".encode())
char = s.read(2)
print("Enter AT mode.")
print(char.decode())
s.write("ATMY 0x165\r\n".encode())
char = s.read(3)
print("Set MY 0x165.")
print(char.decode())
s.write("ATDL 0x265\r\n".encode())
char = s.read(3)
print("Set DL 0x265.")
print(char.decode())
s.write("ATWR\r\n".encode())
char = s.read(3)
print("Write config.")
print(char.decode())
s.write("ATMY\r\n".encode())
char = s.read(4)
print("MY :")
print(char.decode())
s.write("ATDL\r\n".encode())
char = s.read(4)
print("DL : ")
print(char.decode())
s.write("ATCN\r\n".encode())
char = s.read(4)             
print("Exit AT mode.")
print(char.decode())

host = "localhost"
topic= "BBCAR_LOG"
port = 1883
# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)

tmp_log = ""
log = []
datanum = 0

while(True):
    recv = (s.read(1)).decode()
    if(recv != '#'):
        tmp_log+=recv
    elif(recv == '$'):
        break
    else:
        ttt = time.localtime()
        tmp_log = tmp_log + "     Time:" +str(time.asctime(ttt))
        log.append(tmp_log)
        tmp_log = ""
        mqttc.publish(topic, log[datanum])
        print(log[datanum])
        datanum = datanum+1
mesg = "end"  #ending to disconnect
mqttc.publish(topic, mesg)
s.close()
#read = s.read(6)

'''
while(True):
    print("aa")
    read = s.read(6)
    #x.append(float(read.decode()))
    print(read.decode())
    read = s.read(6)
    #y.append(float(read.decode()))
    print(read.decode())
    read = s.read(6)
    #t.append(float(read.decode()))
    print(read.decode())
    #print("x = %f , y = %f , t = %f"%(x[datanum],y[datanum],t[datanum]))
    datanum = datanum+1
'''

    