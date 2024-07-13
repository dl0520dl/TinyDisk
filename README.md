# TinyDisk
#### 基于QT实现的一个简单的网盘系统，分为好友操作和文件操作两部分。<br><br>
## 一、环境：
##### 数据库：SQLite3
##### QT：5.14.2

## 二、好友操作实现的功能：
##### 好友的添加/删除/查找/私聊/群发消息（群聊）...
![image](https://github.com/dl0520dl/TinyDisk/assets/143736335/a4301d8b-b7ee-4f7d-b99a-a8ab48c63d14)

## 三、文件操作实现的功能：
##### 文件的上传/下载/移动/分享/重命名...
![image](https://github.com/dl0520dl/TinyDisk/assets/143736335/d9390287-f7c4-410d-af52-9c4993c88bf8)

## 四、运行方法：
##### 1.在tcpServer下opedb.cpp中，把cloud.db文件改成自己的路径：
![image](https://github.com/user-attachments/assets/3deb9bdb-261d-4346-a4a1-975f33fd6f30)
##### 2.先启动tcpServer（右击选择运行），再启动tcpClient。
##### 3.cloud.db数据库中已经创建了几个用户，可直接登录：
##### 用户名：jack 密码：jack
##### 用户名：tom 密码：tom
##### 用户名：sam 密码：sam


## Tips：
##### 1.每一个用户注册时都会生成一个同名的根文件夹，其位置在 盘符:\Qt安装路径\build-tcpServer-Desktop_Qt_5_14_2_MinGW_32_bit-Debug中，后续该用户所有的网盘文件/文件夹都存储在此。
##### 2.如要测试私聊/好友群发/分享文件等交互性功能，需要同时启动多个客户端并登录后才能实现。
