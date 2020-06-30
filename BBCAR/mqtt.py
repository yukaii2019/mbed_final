import paho.mqtt.client as paho
import time
import math
mqttc = paho.Client()

host = "localhost"
topic = "BBCAR_LOG"

LOG = []
tmp_log = ""
log_num = 0
def on_connect(self, mosq, obj, rc):
      print("Connected rc: " + str(rc))
def on_message(mosq, obj, msg):
    global tmp_log,log_num,LOG
    print("[Received] Topicddd: " + msg.topic + ", Message: " + str(msg.payload,encoding = "utf-8") + "\n")
    tmp_log = str(msg.payload,encoding = "utf-8")
    if tmp_log=="end":   #ending to disconnect
        print("disconnect")
        mqttc.disconnect()
    else:
        LOG.append(tmp_log)
        log_num = log_num+1

def on_subscribe(mosq, obj, mid, granted_qos):
      print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
      print("Unsubscribed OK")

mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)

mqttc.loop_forever()

for i in range (0,log_num):  #PRINT TIME AND VELOCITY
    print(LOG[i])

filename = "BBCAR_LOG"+"_"+str(time.strftime("%Y%m%d%H%M%S"))+".txt"
with open(filename, "w",errors = 'ignore') as f:
    for line in LOG:
        f.write(line+"\n")
print("Save file in", filename)