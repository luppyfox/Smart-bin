
import threading , cv2  , serial , sqlite3 , torch , torch.hub , ultralytics , time , uuid , requests , json



import numpy as np
#from tkinter import *
#from PIL import ImageTk, Image

from pydrive.auth import GoogleAuth
from pydrive.drive import GoogleDrive


from findSerial import *
import serial
import mysql.connector
from datetime import datetime

#327.5 327.5 349.5 319.5
class mainX:
    #global

    def __init__(self) :
        #connect google drive
        #gauth = GoogleAuth()
        #gauth.LocalWebserverAuth()  # ใช้แบบการตั้งค่าผ่าน webserver

        #self.drive = GoogleDrive(gauth)
        self.channel_access_token = 'I2esgmaqd8+0jy1gKla42vg5r8g0yo5lDhQAqSSZWrlG1jTk2piMZxoc361WXY3OKW/+jt6367njNox1vzL7uFFAwgnBibFYXOriRfz6vi9h5zroK1ovHI++y9lfkEhOvmJ6nQ7VbgO8fl1yYRf7mwdB04t89/1O/w1cDnyilFU='
        self.url1= 'https://api.line.me/v2/bot/message/push' #ส่งข้อความให้เฉพาะ admin

        self.target_id = 'U6b9fdd2ef99230e31ec1d90f38eaceca' #ตั้งไอดีไทให้เป็น admin


        # show qrcode
        self.window_title = 'Qrcode'
        self.imgqrcode = cv2.imread('C:/Users/User/Desktop/project/qr.png')
        self.imgqrcode2 = cv2.imread('C:/Users/User/Desktop/project/qr2.jpg')

        #cv2.imshow('Qrcode',self.imgqrcode )

        #SQL 
        try:

            self.mydb = mysql.connector.connect(
                host="localhost",
                user="root",
                password="12345678",
                database="project"
            )

            self.mycursor = self.mydb.cursor()
        except:
            print("ไม่เจอ sql")

        #OBJECT
        self.sqlblock = 0
        #วัตถุแบบที่
        self.numclass = 0
        self.findobject = False 
        self.timecheck = 0

        #ปิด qrcode
        self.closeqr = 0

        #ขนาดถัง
        self.Exit = False
        self.sizebin = False
        self.numbefore = 0

        #COM => Find => COM5
        serialInst = FindOutPort()
        try:
            for numport in serialInst:
                new_value = str(numport[0])
            
            print(new_value)
        
        
            if new_value != '':
                self.Ser = serial.Serial(new_value,9600,timeout=0)

            threading.Thread(target=self.main).start()
            threading.Thread(target=self.show_qr).start()
            

            

        except:
            print("ไม่เจอ port")

        threading.Thread(target=self.capstart()).start()

    def insert_data(self , ID , last_trash_number = 1):
        if self.sqlblock == 0: 
            print("Update Sql")
            
            self.mycursor.execute(f"SELECT MAX(trash_number) FROM trash_data")

            last_trash_number = self.mycursor.fetchone()[0]
            if last_trash_number is None:
                last_trash_number = 1
            else:
                last_trash_number += 1 


            sqlFormula = "INSERT INTO trash_data (trash_number, categories, created_at) VALUES (%s, %s, %s)"
            data_to_insert = (last_trash_number, ID, datetime.now())

            self.mycursor.execute(sqlFormula, data_to_insert)

            self.mydb.commit()

            self.sqlblock = 1 #block 

    #main1 
    def main(self):
        
        message1 = {
            "to": self.target_id,
            "messages": [
                {
                    "type": "text",
                    "text": "ถังขยะ 1 เต็มเเล้ว!"
                }
            ]
        }

        message2 = {
            "to": self.target_id,
            "messages": [
                {
                    "type": "text",
                    "text": "ถังขยะ 2 เต็มเเล้ว!"
                }
            ]
        }

        message3 = {
            "to": self.target_id,
            "messages": [
                {
                    "type": "text",
                    "text": "ถังขยะ 3 เต็มเเล้ว!"
                }
            ]
        }

        message4 = {
            "to": self.target_id,
            "messages": [
                {
                    "type": "text",
                    "text": "ถังขยะ 4 เต็มเเล้ว!"
                }
            ]
        }

        message5 = {
            "messages": [
                {
                    "type": "text",
                    "text": "ขออภัยในความไม่สะดวก ทางเรากำลังรีบแก้ไขปัญหาเครื่องคัดแยกขยะให้เร็วที่สุด"
                }
            ]
        }

        message6 = {
            "messages": [
                {
                    "type": "text",
                    "text": "ขณะนี้เครื่องคัดแยกขยะได้เปิดให้ใช้งานได้ปกติแล้วครับ ขอบคุณที่รอคอยและขออภัยในความไม่สะดวกที่เกิดขึ้นครับ"
                }
            ]
        }

        headers = {
            'Content-Type': 'application/json',
            'Authorization': f'Bearer {self.channel_access_token}'
        }
        while True:
            #print(self.findobject)
        
            #รับค่ามาจาก arduino
            pyau = self.Ser.readline().decode('utf-8').strip()


            if pyau != "":
                print(pyau)

            #ยืนยัน Ai ทำงาน
            if pyau == "F":
                self.findobject = True

            #RESET ค่า
            if pyau == "R":
                self.findobject = False 

            #จบการทำงาน
            if pyau == "E":
                self.Exit = True

            if self.Exit == True:
                try:
                    self.sizebin = int(pyau)

                    if self.sizebin < 30 or self.sizebin > 10:
                        if self.numbefore == 1:
                            response = requests.post(self.url1, headers=headers, data=json.dumps(message1))
                            if response.status_code == 200:
                                print("Broadcast message sent successfully")
                            else:
                                print("Failed to send broadcast message. Status code:", response.status_code)

                        elif self.numbefore == 2:
                            response = requests.post(self.url1, headers=headers, data=json.dumps(message2))
                            if response.status_code == 200:
                                print("Broadcast message sent successfully")
                            else:
                                print("Failed to send broadcast message. Status code:", response.status_code)

                        elif self.numbefore == 3:
                            response = requests.post(self.url1, headers=headers, data=json.dumps(message3))
                            if response.status_code == 200:
                                print("Broadcast message sent successfully")
                            else:
                                print("Failed to send broadcast message. Status code:", response.status_code)

                        elif self.numbefore == 4:
                            response = requests.post(self.url1, headers=headers, data=json.dumps(message4))
                            if response.status_code == 200:
                                print("Broadcast message sent successfully")
                            else:
                                print("Failed to send broadcast message. Status code:", response.status_code)

                    self.Exit = False
                except:
                    print("Not a integer")
                    self.Exit = False
            
            #ส่งค่า Detection
            if self.numclass != 0:
                #เก็บไว้ใช้กับตัวอื่น
                self.numbefore = self.numclass
                #เคลียข้อมูลให้เป็น defalt
                self.Ser.write(bytes(str(self.numclass),'utf-8'))
                time.sleep(2)
                self.numclass = 0
                self.sqlblock = 0
                self.Ser.write(bytes(str(self.numclass),'utf-8')) 
            
            if self.numclass == 0 and pyau == "E":
                self.closeqr = 1

            #if self.numbefore != 0:
            #    time.sleep(10)
            #    self.closeqr = 1

    
    #main2
    def capstart(self):
        model = torch.hub.load('C:\\Users\\User\\Desktop\\project\\yolov5', 'custom','C:\\Users\\User\\Desktop\\project\\13-09.pt', source = 'local')
        print("START")
        cap = cv2.VideoCapture(0)
 
        
        while(True):
            ret, frame = cap.read()
            if not ret:
                break
            
            
            # Detect objects in the captured frame

            results = model(frame)

            # Process the detected objects (e.g., draw bounding boxes)
            detection = results.pandas().xyxy[0]

            # Display the processed frame

            for index, row in detection.iterrows():
                numberclass = row['class']
                nameclass = row['name']
                conf = row['confidence']
                conf = float(conf)
                conf = '{:.2f}'.format(conf)
                x1 = int(row['xmin'])
                y1 = int(row['ymin'])
                x2 = int(row['xmax'])
                y2 = int(row['ymax'])

                if float(conf) > 0.4:
                    cv2.rectangle(frame, (int(x1),int(y1)), (int(x2),int(y2)),(0,255,0), 2)
                    cv2.putText(frame,(f'{str(conf)} {str(nameclass)} {str(numberclass)}'), (int(x1), int(y1)+20), 1, 1, (179, 55, 0),2)

                if float(conf) > 0.4 :#and self.findobject == True: # <---- กรุงเทพ
                    if numberclass == 0:
                        self.numclass = 2
                        self.insert_data(103001)

                        
                        
                    if numberclass == 1:
                        self.numclass = 3
                        self.insert_data(102001)
                        
                        
                    if numberclass == 2:
                        self.numclass = 4
                        self.insert_data(101001)
              
                        
                       

            if self.timecheck != 10 and self.numclass <= 0 and self.findobject == True:
                self.timecheck = self.timecheck + 1

            if self.timecheck == 1 and self.numclass == 0 and self.findobject == True:
                self.insert_data(104001)

                cutframe = cv2.resize(frame, (320,320))     
                uid = uuid.uuid4()

                img_name = "C:/Users/User/Desktop/project/nonedection/" + "{}.jpg".format(uid)

                #file_drive = self.drive.CreateFile({'title': f"C:/Users/User/Desktop/project/nonedection/{uid}"})
                #file_drive.Upload()

                cv2.imwrite(img_name, cutframe)

                self.numclass = 1
                self.timecheck = 0
            
          
            cv2.imshow('Detection', frame)

            # Press 'q' to exit the loop
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    def show_qr(self):
        qrcheck = False

        while True:
            if self.numclass >= 2 and self.numclass != 3:
                qrcheck = True
                cv2.imshow('Qrcode',self.imgqrcode)
            if self.numclass == 3:
                qrcheck = True
                cv2.imshow('Qrcode',self.imgqrcode2)
            try:
                if self.closeqr == 1 and qrcheck == True: 
                    cv2.destroyWindow('Qrcode')
                    self.closeqr = 0
                    qrcheck = False
            except:
                pass

            cv2.waitKey(1)
        

if __name__ == '__main__':
    
    
    mainX()

