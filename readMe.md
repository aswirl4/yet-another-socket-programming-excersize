# yet another 计算机网络原理 socket编程作业
一个同时支持ipv4和ipv6的web服务端.  
仅支持运行在linux系统上.

运行后监听所有地址的访问.  
监听端口定义在`config.txt`中.
配置文件: `config.txt文件`(可在`main.cpp`中修改)
配置文件内容格式:
```
[端口号]
[聊天记录生命周期] (功能未实现)
[发信频率] (功能未实现)
```

web路由:
```
/chatRoom
聊天室页面
```

```
/filesSharing
文件共享页面.
默认暴露了运行目录下的./filesSharing目录
用户可访问/filesSharing/文件名来下载文件
***警告***:未实现任何web安全功能, 请勿将此服务暴露在公网上.
```

