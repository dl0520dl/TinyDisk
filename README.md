# TinyDisk
基于QT实现的一个简单的网盘系统，分为好友操作和文件操作两部分。

## 环境：
#### 数据库：SQLite3
#### QT：5.14.2

## 好友操作实现的内容：
#### 好友的添加/删除/查找/私聊/群发消息（群聊）...
![image](https://github.com/dl0520dl/TinyDisk/assets/143736335/a4301d8b-b7ee-4f7d-b99a-a8ab48c63d14)


## 文件操作实现的功能：
#### 文件的上传/下载/移动/分享/重命名...
![image](https://github.com/dl0520dl/TinyDisk/assets/143736335/d9390287-f7c4-410d-af52-9c4993c88bf8)

## Tips
每一个用户注册时都会生成一个同名的根文件夹，其位置在 盘符:\Qt安装路径\build-tcpServer-Desktop_Qt_5_14_2_MinGW_32_bit-Debug中，后续该用户所有的网盘文件/文件夹都存储在此。
