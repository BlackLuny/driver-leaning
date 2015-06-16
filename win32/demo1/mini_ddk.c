//_stdcall
/*
nt式驱动的头文件声明ntddk.h
DriverEntry 入口函数 相当于mian函数 DriverEntry下有两个参数
PDRIVER_OBJECT 此结构用来传递驱动的对象 有I/O管理器传递进来的驱动对象
PUNICODE_STRING 此结构用来指向此驱动复测的注册表 也就是驱动程序在注册表中的路径
*/
#include <ntddk.h>
#define   INITCODE code_seg("INIT")
#pragma INITCODE
VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject);//前置说明 卸载例程
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriveObject,IN PUNICODE_STRING RegisterPath) //typedef long NTSTAUS
{
	KdPrint(("驱动成功被加载_____________OK"));   //DgbPrint("Hello DDK");
	pDriveObject->DriverUnload=DDK_Unload;
	return STATUS_SUCCESS;
}
VOID DDK_Unload(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("驱动成功被卸载____________OK")); //sprintf,printf
}
/*
在所有的预处理指令中，#pragma 指令可能是最复杂的了，它的作用是设定编译器的状态或者是指示编译器完成一些特定的动作。#pragma 指令对每个编译器给出了一个方法,在保持与C和C++语言完全兼容的情况下,给出主机或操作系统专有的特征。依据定义,编译指示是机器或操作系统专有的, 且对于每个编译器都是不同的。 
其格式一般为: #Pragma Para 
其中Para 为参数，下面来看一些常用的参数。 

(

在编写程序的时候,我们经常要用到#pragma指令来设定编译器的状态或者是指示编译器完成一些特定的动作.
下面介绍了一下该指令的一些常用参数,希望对大家有所帮助!

一. message 参数。
message
它能够在编译信息输出窗
口中输出相应的信息，这对于源代码信息的控制是非常重要的。

其使用方法为：     #pragma message("消息文本")

当编译器遇到这条指令时就在编译输出窗口中将消息文本打印出来。
当我们在程序中定义了许多宏来控制源代码版本的时候，我们自己有可能都会忘记有没有正确的设置这些宏，此时我们可以用这条指令在编译的时候就进行检查。假设我们希望判断自己有没有在源代码的什么地方定义了_X86这个宏可以用下面的方法：
#ifdef _X86
#pragma message("_X86 macro activated!")
#endif
当我们定义了_X86这个宏以后，应用程序在编译时就会在编译输出窗口里显示
"_X86 macro activated!"
这样，我们就不会因为不记得自己定义的一些特定的宏而抓耳挠腮了。

二. 另一个使用得比较多的#pragma参数是code_seg。
格式如：

#pragma code_seg( [ [ { push | pop}, ] [ identifier, ] ] [ "segment-name" [, "segment-class" ] )
该指令用来指定函数在.obj文件中存放的节,观察OBJ文件可以使用VC自带的dumpbin命令行程序,函数在.obj文件中默认的存放节为.text节，如果code_seg没有带参数的话,则函数存放在.text节中。

push (可选参数) 将一个记录放到内部编译器的堆栈中,可选参数可以为一个标识符或者节名
pop(可选参数) 将一个记录从堆栈顶端弹出,该记录可以为一个标识符或者节名
identifier (可选参数) 当使用push指令时,为压入堆栈的记录指派的一个标识符,当该标识符被删除的时候和其相关的堆栈中的记录将被弹出堆栈
"segment-name" (可选参数) 表示函数存放的节名
例如:
//默认情况下,函数被存放在.text节中
void func1() {                   // stored in .text
}

//将函数存放在.my_data1节中
#pragma code_seg(".my_data1")
void func2() {                   // stored in my_data1
}

//r1为标识符,将函数放入.my_data2节中
#pragma code_seg(push, r1, ".my_data2")
void func3() {                   // stored in my_data2
}

int main() {
}

三. #pragma once (比较常用）
这是一个比较常用的指令,只要在头文件的最开始加入这条指令就能够保证头文件被编译一次

四. #pragma hdrstop表示预编译头文件到此为止，后面的头文件不进行预编译。
BCB可以预编译头文件以加快链接的速度，但如果所有头文件都进行预编译又可能占太多磁盘空间，所以使用这个选项排除一些头文件。
有时单元之间有依赖关系，比如单元A依赖单元B，所以单元B要先于单元A编译。你可以用#pragma startup指定编译优先级，如果使用了#pragma package(smart_init) ，BCB就会根据优先级的大小先后编译。

五. #pragma warning指令
该指令允许有选择性的修改编译器的警告消息的行为
指令格式如下:
#pragma warning( warning-specifier : warning-number-list [; warning-specifier : warning-number-list...]
#pragma warning( push[ ,n ] )
#pragma warning( pop )

主要用到的警告表示有如下几个:

once:只显示一次(警告/错误等)消息
default:重置编译器的警告行为到默认状态
1,2,3,4:四个警告级别
disable:禁止指定的警告信息
error:将指定的警告信息作为错误报告

如果大家对上面的解释不是很理解,可以参考一下下面的例子及说明

#pragma warning( disable : 4507 34; once : 4385; error : 164 )
等价于：
#pragma warning(disable:4507 34)   // 不显示4507和34号警告信息
#pragma warning(once:4385)         // 4385号警告信息仅报告一次
#pragma warning(error:164)         // 把164号警告信息作为一个错误。
同时这个pragma warning 也支持如下格式：
#pragma warning( push [ ,n ] )
#pragma warning( pop )
这里n代表一个警告等级(1---4)。
#pragma warning( push )保存所有警告信息的现有的警告状态。
#pragma warning( push, n)保存所有警告信息的现有的警告状态，并且把全局警告等级设定为n。
#pragma warning( pop )向栈中弹出最后一个警告信息，在入栈和出栈之间所作的一切改动取消。例如：
#pragma warning( push )
#pragma warning( disable : 4705 )
#pragma warning( disable : 4706 )
#pragma warning( disable : 4707 )
#pragma warning( pop )

在这段代码的最后，重新保存所有的警告信息(包括4705，4706和4707)

在使用标准C++进行编程的时候经常会得到很多的警告信息,而这些警告信息都是不必要的提示,所以我们可以使用#pragma warning(disable:4786)来禁止该类型的警告在vc中使用ADO的时候也会得到不必要的警告信息,这个时候我们可以通过#pragma warning(disable:4146)来消除该类型的警告信息

六. pragma comment(...)
该指令的格式为：   #pragma comment( "comment-type" [, commentstring] )
该指令将一个注释记录放入一个对象文件或可执行文件中,comment-type(注释类型):可以指定为五种预定义的标识符的其中一种。
五种预定义的标识符为:

1、compiler:将编译器的版本号和名称放入目标文件中,本条注释记录将被编译器忽略
如果你为该记录类型提供了commentstring参数,编译器将会产生一个警告
例如:#pragma comment( compiler )

2、exestr:将commentstring参数放入目标文件中,在链接的时候这个字符串将被放入到可执行文件中,当操作系统加载可执行文件的时候,该参数字符串不会被加载到内存中.但是,该字符串可以被dumpbin之类的程序查找出并打印出来,你可以用这个标识符将版本号码之类的信息嵌入到可执行文件中!

3、lib:这是一个非常常用的关键字,用来将一个库文件链接到目标文件中常用的lib关键字，可以帮我们连入一个库文件。
例如: 
#pragma comment(lib, "user32.lib")
该指令用来将user32.lib库文件加入到本工程中

4、linker:将一个链接选项放入目标文件中,你可以使用这个指令来代替由命令行传入的或者在开发环境中设置的链接选项,你可以指定/include选项来强制包含某个对象,例如:
#pragma comment(linker, "/include:__mySymbol")
你可以在程序中设置下列链接选项
/DEFAULTLIB
/EXPORT
/INCLUDE
/MERGE
/SECTION

这些选项在这里就不一一说明了,详细信息请看msdn!

5、user:将一般的注释信息放入目标文件中commentstring参数包含注释的文本信息,这个注释记录将被链接器忽略
例如:
#pragma comment( user, "Compiled on " __DATE__ " at " __TIME__ )
*/
