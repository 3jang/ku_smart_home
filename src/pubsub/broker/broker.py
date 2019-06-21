import paho.mqtt.client as mqtt
import pymysql
import time
import sys
import threading
from datetime import datetime

if len(sys.argv) > 1:
    ip = sys.argv[1]
else:
    ip = '10.42.0.1'

cc_ = 0
 
def sum():
    global cc_
    publisher = mqtt.Client("buzzer_pub")
    publisher.connect(ip, 1883)
    while True:
        if cc_ == 1:
            publisher.publish("/buzzer", "1")
        time.sleep(0.4)

def on_connect(client, userdata, flags, rc):
    print ("Connect " + str(rc))
    client.subscribe("/temperature")
    client.subscribe("/alarm")
    client.subscribe("/request_temphum")
    client.subscribe("/idpw")
    client.subscribe("/light")
    client.subscribe("/buzzer")

def on_message(client, userdata, msg):
    global cc_
    print "Topic: ", msg.topic + '\nMessage: ' + str(msg.payload)
    if msg.topic == '/temperature':
        args = str(msg.payload).split(',')
        hm = int(args[0])
        tp = int(args[1])
        print tp, hm
        if tp > 0:
            now = datetime.now()
            print now
            sql = 'INSERT INTO TEMPHUM VALUES(%s, %s, %s)'
            cursor.execute(sql, (tp, hm, str(now)))
            db.commit()
        
    if msg.topic == '/alarm':
       args = str(msg.payload).split(',')
       if args[5] == '1':
           sql = 'INSERT INTO ALARM VALUES(%s,%s)'
           cursor.execute(sql, (args[3], args[4]))
           db.commit()
    if msg.topic == '/request_temphum':
        print "let's go"
        sql = 'SELECT * FROM TEMPHUM ORDER BY date DESC LIMIT 10'
        curs = db.cursor(pymysql.cursors.DictCursor)
        curs.execute(sql)
        rows = curs.fetchall()
        dstr = ''
        for row in rows:
            dstr += str(row['temperature']) + ',' + str(row['humidity']) + ','
        dstr = dstr[:-1]
        print dstr
        publisher.publish("/tv", dstr)
    if msg.topic == '/idpw':
        print "idpw"
        args = str(msg.payload).split(',')
        sql = 'SELECT * FROM ACCOUNT WHERE id=%s and pw=%s'
        cursor.execute(sql, (args[0], args[1]))
        row = cursor.fetchone()
        rcc = cursor.rowcount
        dstr = ""
        if rcc >= 1 :
            dstr = "1,"+str(row[2])
            publisher.publish("/answer", dstr)
        else :
            dstr = "0,0"
            publisher.publish("/answer", dstr)
        print dstr
    if msg.topic == '/light':
        now = datetime.now()
        print now
        sql = 'INSERT INTO LIGHT VALUES(%s, %s)'
        cursor.execute(sql, (int(msg.payload), str(now)))
        db.commit()
    if msg.topic == '/buzzer':
        if msg.payload == '0':
            cc_ = 0
        elif msg.payload == '2':
            cc_ = 1

publisher = mqtt.Client("tv_pub")
publisher.connect(ip, 1883)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(ip, 1883, 60)
ledon = 0
fivesec = 0

t1 = threading.Thread(target=sum)
t1.daemon = True
t1.start()

db = pymysql.connect(host=ip, port=3306, user='jins_hot', passwd='jins1212', db='jins_db', charset='utf8')
cursor = db.cursor()

while True :
    now = datetime.now()
    sql = 'SELECT * FROM TEMPHUM ORDER BY date DESC LIMIT 10'
    curs = db.cursor(pymysql.cursors.DictCursor)
    curs.execute(sql)
    rows = curs.fetchall()
    dstr = ''

    tempsum = 0
    humsum = 0
    for row in rows:
        dstr += str(row['temperature']) + ',' + str(row['humidity']) + ','
        tempsum += row['temperature']
        humsum += row['humidity']
    dstr = dstr[:-1]
    #print dstr
    publisher.publish("/tv", dstr)
    if fivesec == 5 :
        tempavg = tempsum/10
        humavg = humsum/10

        sql = 'SELECT * FROM LIGHT ORDER BY date DESC LIMIT 5'
        cursor.execute(sql)
        rows = cursor.fetchall()
       
        lightsum = 0
        for row in rows:
            lightsum += row[0]
        lightavg = lightsum/5
        
        print tempavg, humavg, lightavg
         
        if tempavg < 150:
            goodhum = 700
        elif tempavg < 200:
            goodhum = 600
        elif tempavg < 230:
            goodhum = 500
        else:
            goodhum = 400
        
        lst = 0
        if lightavg < 1500:
            lst = -1
        publisher.publish("/state", str(goodhum - humavg)+','+str(lst))
        fivesec = 0
    fivesec += 1
    sql = 'SELECT * FROM ALARM WHERE hour = %s and minute = %s'
    cursor.execute(sql, (now.hour, now.minute))
    print (cursor.rowcount)
    if cursor.rowcount > 0 :
        print "buzzer on"
        cc_ = 1
        sql = 'delete FROM ALARM WHERE hour = %s and minute = %s'
        cursor.execute(sql, (now.hour, now.minute))
        db.commit()
    client.loop(1)

db.close()
