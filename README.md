
这里是[Computer Graphics GAMES101](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html)的作业  
我对文件(比如CMakeLists.txt)和文件结构(新建了include和src路径, 将头文件和库文件分别放进对应的路径下)等做了一些改变, 更符合我自己的习惯  
我的开发环境是mac, 所以一些说明和配置是适配mac的, 请注意

#### 编译

编译采用cmake, 根据各个作业里的CMakeLists.txt来进行编译  
- 在各作业的路径下, 新建一个编译文件的路径: `mkdir build`
- 跳转到build路径下: `cd build`
- 预处理: `cmake ..`
- 编译: `make`
- 执行: 在pa0路径下: `./<可执行文件>`

#### 测试

面对这里的代码, vscode的debug很慢, 原因使query所有的variables, 所以可以用lldb或者gdb命令行来进行debug