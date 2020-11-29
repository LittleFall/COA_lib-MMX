# COA_lib-MMX
西安交通大学-软件工程-计算机组成原理课内实验

实现图片淡入淡出效果，要求分别使用普通方法和MMX指令集。

编译环境 windows10 cl.exe (MSVC)，配置见 https://blog.csdn.net/m0_37809890/article/details/85103109

主文件 /src/main.cpp

图片文件 /pic/back-1920-1080-32.bmp,black-1920-1080-32.bmp

使用方法 编译运行后迅速返回桌面，图像会出现淡入淡出两次，分别使用的是普通方法和MMX方法，会有明显的速度差异。显示完成后会黑屏，切换窗口即可。

详细介绍见 https://blog.csdn.net/m0_37809890/article/details/85111091



注：不要在汇编代码中添加以 LF 为行结尾的单行注释！
