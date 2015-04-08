#<center>ptarce</center>



##概述
ptrace是process和trace的简写，直译为进程跟踪。它提供了一种使父进程得以监视和控制其子进程的方式，它还能够改变子进程中的寄存器和内核映像，因而可以实现断点调试和系统调用的跟踪。


##基础知识
###进程
  程序是一个静态的概念,它只是一些预先编译好的指令和数据集合的一个文件;而进程是一个动态的概念,它是程序运行时的一个过程。
#####创建进程
<table border="1">
<tr>
<td> #include &lt;unistd.h><br>
      pid_t fork(void);<br>
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;返回值:子进程返回0,父进程返回子进程ID,出错返回-1</td>
</tr>
</table>
fork函数被调用一次，却能够返回两次，它可能有三种不同的返回值：<br>
    &nbsp;&nbsp;&nbsp;&nbsp;1）在父进程中，fork返回新创建子进程的进程ID；<br>
    &nbsp;&nbsp;&nbsp;&nbsp;2）在子进程中，fork返回0；<br>
    &nbsp;&nbsp;&nbsp;&nbsp;3）如果出现错误，fork返回-1；

在fork函数执行完毕后，如果创建新进程成功，则出现两个进程，一个是子进程，一个是父进程。可以通过fork返回的值来判断当前进程是子进程还是父进程。
###系统调用
&nbsp;&nbsp;&nbsp;&nbsp;在现代的操作系统里,程序运行的时候，本身是没有权利访问多少系统资源的。由于系统有限的资源有可能被多个不同的应用程序同时访问，因此，如果不加以保护，那么各个应用程序难免产生冲突。所以现代操作系统都将可能产生冲突的系统资源给保护起来，阻止应用程序直接访问。这些资源包括文件、网络、IO、各种设备等。
每个操作系统都会提供一套接口，来封装对系统资源的调用，这套接口就是系统调用。<br>

&nbsp;&nbsp;&nbsp;&nbsp;操作系统把进程空间分为了用户空间和内核空间，系统调用是运行在内核空间的，而应用程序基本都是运行在用户空间的。应用程序想要访问系统资源，就必须通过系统调用。用户空间的应用程序要想调用内核空间的系统调用，就需要从用户空间切换到内核空间，这一般是通过中断来实现的。什么是中断呢？中断是一个硬件或者软件发出的请求，要求CPU暂停当前的工作转手去处理更加重要的事情。中断一般具有两个属性，中断号和中断处理程序。在内核中，有一个叫做中断向量表的数组来存放中断号和中断处理程序。当中断到来的时候，CPU会根据中断号找到对应的中断处理程序，并调用它。中断处理程序执行完成后，CPU会继续执行之前的代码。
<br>

&nbsp;&nbsp;&nbsp;&nbsp;通常意义上，中断有两种类型，一种称为硬件中断，这种中断来自于硬件的异常或其他事件的发生；另一种称为软件中断，软件中断通常是一条指令(i386下是int),带有一个参数记录中断号。linux系统使用int 0x80来触发所有的系统调用，和中断一样，系统调用带有一个系统调用号，这个系统调用就像身份标识一样来表明是哪一个系统调用，这个系统调用号会放在eax寄存器中。如果系统调用有一个参数，那么参数通过ebx寄存器传入，x86下linux支持的系统调用参数至多有6个，分别使用6个寄存器来传递，它们分别是ebx、ecx、edx、esi、edi和ebp。
<br>

&nbsp;&nbsp;&nbsp;&nbsp;触发系统调用后，CPU首先需要切换堆栈，当程序的当前栈从用户态切换到内核态后，会找到系统调用号对应的调用函数，它们都是以"sys_"开头的，当执行完调用函数后，返回值会存放在eax寄存器返回到用户态。



###信号
&nbsp;&nbsp;&nbsp;&nbsp;信号是在软件层次上对中断机制的一种模拟，它是一种进程间异步通信的机制。<br>

 &nbsp;&nbsp;&nbsp;&nbsp;一个进程要发信号给另一个进程，可以使用这些函数：kill()、raise()、 sigqueue()、alarm()、setitimer()以及abort()。它其实是通过系统调用把信号先发给内核。当另一个进程从内核态回用户态的时候，它会先去找一下有没有发给自己的信号，如果有，就处理掉。<br>
 
 进程可以通过三种方式来响应一个信号：<br>
 （1）忽略信号，即对信号不做任何处理;<br> （2）捕捉信号。通过signal()定义信号处理函数，当信号发生时，执行相应的处理函数；<br>
 （3）执行缺省操作，Linux对每种信号都规定了默认操作。
 

##ptrace函数详解
函数原型如下：

	#include <sys/ptrace.h>
    long ptrace(enum __ptrace_request request, pid_t pid,
                   void *addr, void *data);
                   
ptrace有四个参数: <br>
&nbsp;&nbsp;1. enum \_\_ptrace\_request request：指示了ptrace要执行的命令。<br>
&nbsp;&nbsp;2. pid\_t pid: 指示ptrace要跟踪的进程。<br>
&nbsp;&nbsp;3. void \*addr: 指示要监控的内存地址。<br>
&nbsp;&nbsp;4. void \*data: 存放读取出的或者要写入的数据。<br>

request参数决定了ptrace的具体功能：

1.PTRACE_TRACEME


 
      ptrace(PTRACE_TRACEME,0 ,0 ,0)
      
描述：本进程被其父进程所跟踪。


<br>
2.PTRACE\_PEEKTEXT, PTRACE\_PEEKDATA



      ptrace(PTRACE_PEEKDATA, pid, addr, data)

描述：从内存地址中读取一个字，数据地址由函数返回,pid表示被跟踪的子进程，内存地址由addr给出，data参数被忽略。

 <br>
 3.PTRACE\_POKETEXT, PTRACE\_POKEDATA



     ptrace(PTRACE_POKEDATA, pid, addr, data)

描述：往内存地址中写入一个字。pid表示被跟踪的子进程，内存地址由addr给出，data为所要写入的数据地址。

 <br>
4.PTRACE_PEEKUSR


      
     ptrace(PTRACE_PEEKUSR, pid, addr, data)

描述：从 USER区域中读取一个字节，pid表示被跟踪的子进程，addr表示读取数据在USER区域的偏移量，返回值为函数返回值，data参数被忽略。

 <br>
5.PTRACE_POKEUSR


     
     ptrace(PTRACE_POKEUSR, pid, addr, data)

描述：往USER区域中写入一个字节，pid表示被跟踪的子进程，USER区域地址由addr给出，data为需写入的数据。
 <br>
6.PTRACE_CONT

    ptrace(PTRACE_CONT, pid, 0, signal)

描述：继续执行。pid表示被跟踪的子进程，signal为0则忽略引起调试进程中止的信号，若不为0则继续处理信号signal。

 <br>
7.PTRACE_SYSCALL

    ptrace(PTRACE_SYS, pid, 0, signal)

描述：继续执行。pid表示被跟踪的子进程，signal为0则忽略引起调试进程中止的信号，若不为0则继续处理信号signal。与PTRACE_CONT不同的是进行系统调用跟踪。在被跟踪进程继续运行直到调用系统调用开始或结束时，被跟踪进程被中止，并通知父进程。

 <br>
8.PTRACE_KILL

    ptrace(PTRACE_KILL,pid)

描述：杀掉子进程，使它退出。pid表示被跟踪的子进程。

 <br>
9.PTRACE_SINGLESTEP

    ptrace(PTRACE_KILL, pid, 0, signle)

描述：设置单步执行标志，单步执行一条指令。pid表示被跟踪的子进程。signal为0则忽略引起调试进程中止的信号，若不为0则继续处理信号signal。当被跟踪进程单步执行完一个指令后，被跟踪进程被中止，并通知父进程。

 <br>
 <br``>
10.PTRACE_ATTACH

    ptrace(PTRACE_ATTACH,pid)

描述：跟踪指定pid 进程。pid表示被跟踪进程。被跟踪进程将成为当前进程的子进程，并进入中止状态。

 <br>
11.PTRACE_DETACH

    ptrace(PTRACE_DETACH,pid)

描述：结束跟踪。 pid表示被跟踪的子进程。结束跟踪后被跟踪进程将继续执行。

 <br>
12.PTRACE_GETREGS

    ptrace(PTRACE_GETREGS, pid, 0, data)

描述：读取寄存器值，pid表示被跟踪的子进程，data为用户变量地址用于返回读到的数据。

 <br>
13.PTRACE_SETREGS

    ptrace(PTRACE_SETREGS, pid, 0, data)

描述：设置寄存器值，pid表示被跟踪的子进程，data为用户数据地址。


以上列出的都是一些较常用的功能介绍，更多详细的信息可以参考linux下man手册:
<a herf="http://linux.die.net/man/2/ptrace">man ptrace</a>。
##ptrace函数示例
&nbsp;&nbsp;&nbsp;&nbsp;以上部分我们对ptrace函数做了概念上的介绍，现在我们通过几个小示例来应用ptarce，加深对概念的理解。
###读取系统调用号

hello.c

  
  
    #include <stdio.h>
    int main(){
    printf("Hello, world!\n");
    return 0;
    }

编译，然后测试运行：

    robin@ubuntu:~/work/ptrace$ gcc hello.c -o hello
    robin@ubuntu:~/work/ptrace$ ./hello
    Hello, world!
    
tracer1.c

    #include <sys/ptrace.h>
    #include <sys/wait.h>
    #include <unistd.h>
    #include <sys/reg.h>
    #include <stdio.h>
    #include <stdlib.h>

  
    /**
     * 获取子进程系统调用号
     * */
    int main() {

	pid_t child;
	child = fork();
    if (child < 0) {
		perror("fork error");
	} else if (child == 0) {
		//子进程执行
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl("hello", "hello", NULL);
	} else {
		//父进程执行
		long orig_eax;
		int status;
		while (1) {
			//等待子进程信号
			wait(&status);
			if (WIFEXITED(status))//子进程发送退出信号，退出循环
				break;
            //调用ptrace从子进程取数据
			orig_eax = ptrace(PTRACE_PEEKUSER, child, 4 * ORIG_EAX, NULL);
			printf("orig_eax = %ld \n", orig_eax);
			//让子进程继续执行
			ptrace(PTRACE_SYSCALL, child, NULL, NULL);
		}
	}
	return 0;
}
  


编译并运行，输出如下:

    robin@ubuntu:~/work/ptrace$ gcc tracer1.c -o tracer1
    robin@ubuntu:~/work/ptrace$ ./tracer1 
    orig_eax = 11
       ...
       ...
    orig_eax = 4
    Hello, world!
    orig_eax = 4
    orig_eax = 252
    
 

篇幅所限，输出内容只截取了第一个和最后几个，中间部分输出省略了。<br>
####示例说明

&nbsp;&nbsp;&nbsp;&nbsp;hello.c不用多说，就是一个最简单的C程序，编译成一个名为hello的可执行文件，给tracer.c调用执行。

&nbsp;&nbsp;&nbsp;&nbsp;在tracer.c的main方法里，通过fork()函数来创建一个子进程，在子进程中，先调用 PTRACE_TRACEME让父进程跟踪自己，然后调用execl函数执行hello。<br>

&nbsp;&nbsp;&nbsp;&nbsp;在父进程里，循环调用wait函数，等待子进程信号直到收到子进程退出信号。当收到子进程非退出信号后开始通过ptrace函数来操作子进程。这里通过PTRACE\_PEEKUSER来读取子进程里的数据,这里传入 4 * ORIG\_EAX作为数据地址，ORIG\_EAX是一个宏值定义，查看源码，可以知道它的值是0x24,它表示在系统堆栈中相对栈顶指针的一个偏移量，当发生系统调用时，它其实存放的系统调用号。在32位的机器上，系统调用表中的表项是以32位(4字节)类型存放的，所以这里需要将给定的系统调用号乘以4。第一个返回的结果是11，它是子进程执行的第一个系统调用，如果想查看一下各个系统调用编号对应的名字，可以参考头文件：/usr/include/asm/unistd.h。每次获取系统调用号之后，调用PTRACE\_SYSCALL让子线程继续执行，直到发生下一次系统调用。<br>

&nbsp;&nbsp;&nbsp;&nbsp;仅仅只是一个简单的printf调用，为什么发生了这么多次系统调用？默认情况下，Linux中的gcc编译器会动态链接到C运行时库。这意味着任何程序在运行时首先要做的事情是加载动态库,这需要很多代码实现。我们这个tracer程序追踪的是整个进程，而不仅仅是main函数，所以会发生很多次系统调用。<br>
&nbsp;&nbsp;&nbsp;&nbsp;如果我们把hello.c编译成一个静态库，执行如下命令编译:
   
    robin@ubuntu:~/work/ptrace$ gcc -static hello.c -o hello
 这时候再执行tarcer就会少一些系统调用，因为编译时已经把C运行时库链接到hello目标文件里面，运行的时候不需要再动态去链接C运行时库了。  <br>
 
 &nbsp;&nbsp;&nbsp;&nbsp;我们注意观察最后输出的几行,在输出Hello, world!前后输出了2次系统调用号4：
   
    orig_eax = 4
    Hello, world!
    orig_eax = 4
  查看源码,4这个系统调用号的作用是写文件。而printf函数的实现正是调用了wirte函数:
  
      #include <unistd.h>
      ssize_t write(int fd, const void *buf, size_t count);
 这里输出了2次系统调用号4的原因是，当程序执行到系统调用的时候，进程会从用户态进入内核态，并且把系统调用号放到eax寄存器，这时候产生一条信号，会通知到父进程; 在内核态执行完成后，进程会从内核态回到用户态，这时候又会产出信号。   <br>
 
###读取系统调用参数
 &nbsp;&nbsp;&nbsp;&nbsp;在tracer1这个简单程序中，我们读取了子进程执行过程中的系统调用号，并知道当系统调用号是4的时候会发生write系统调用，输出Hello, world!。这里我们更进一步，当发生wirite系统调用的时候看看怎么去获取对应的参数。<br>
####tracer2.c部分代码：
  
         if (orig_eax == SYS_write) {
				if (insyscall == 0) {
				/* Syscall entry */
				insyscall = 1;
                params[0] = ptrace(PTRACE_PEEKUSER,child, 4 * EBX,NULL);
                params[1] = ptrace(PTRACE_PEEKUSER,child, 4 * ECX,NULL);
                params[2] = ptrace(PTRACE_PEEKUSER,child, 4 * EDX,NULL);
                printf("write called with %ld, %ld, %ld\n",params[0], params[1],params[2]);
             }
         }
         
 编译执行，输出如下:
   
    robin@ubuntu:~/work/ptrace$ gcc -o tracer2 tracer2.c
    robin@ubuntu:~/work/ptrace$ ./tracer2
    write called with 1, -1217363968, 14
    Hello, world!

x86 linux中，ebx, ecx, edx这3个寄存器是用来存放系统调用参数的，这里我们对照wirte函数申明,ebx存放的是fd,这里输出1正是标准输出的fd号;ecx存放的是字符串数据的地址;edx存放的是字符串长度,"Hello, world!"的长度正好是14。
 
###读取寄存器值
在tracer2这个例子中,我们用PTRACE\_PEEKUSER来获取子进程系统调用的参数,但是这是比较笨拙的方法。我们可以使用PRACE_GETREGS作为ptrace的第一个参数来调用，可以只需一次函数调用就取得所有的相关寄存器值。

####tracer3.c部分代码:
 
     struct user_regs_struct regs;

     ptrace(PTRACE_GETREGS, child,NULL, &regs);
     printf("Write called with %ld, %ld, %ld\n",regs.ebx, regs.ecx,regs.edx);
     
 编译执行,可以看到输出内容和tracer2相同。    

###改变子进程执行结果
 &nbsp;&nbsp;&nbsp;&nbsp;通过tracer2或tracer3我们可以读取子进程发生系统调用时的参数,在本示例中,我们去改变这个参数,从而影响子进程的执行结果。
####tracer4.c完整代码:
      

      #include <sys/ptrace.h>
      #include <sys/types.h>
      #include <sys/wait.h>
      #include <unistd.h>
      #include <sys/user.h>
      #include <sys/syscall.h>
      #include <sys/reg.h>
      #include <stdio.h>
      #include <stdlib.h>
      #include <string.h>

    const int word_size = sizeof(int); //字长,本机系统是32位，所以字长应该为4字节

    /**
     * 反转str
     * */
    void reverse(char *str) {

	 	int i, j;
	   	char temp;
		for (i = 0, j = strlen(str) - 2; i <= j; ++i, --j) {
			temp = str[i];
			str[i] = str[j];
			str[j] = temp;
		}
    }

	 /**  
	 * 从子进程读数据
	 * */
	 void getdata(pid_t child, long addr, char *str, int len) {

	 	char *laddr;
	 	int i, j;

	 	union u {
			long val;
			char chars[word_size];
	 	} data; //联合体，val和chars指向同一块内存

	 	i = 0;
	 	j = len / word_size;

	 	laddr = str;

	 	while (i < j) {
			//每次读一个字，也就是读4byte
			data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 4, NULL);
			//拷贝到事先分配好的字符串地址
			memcpy(laddr, data.chars, word_size);
			++i;
			laddr += word_size;
	 	}

	 	//读最后小于4字节的几个字节内容
	 	j = len % word_size;
	 	if (j != 0) {
			data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 4, NULL);
			memcpy(laddr, data.chars, j);
	 	}
	 	//结束符
	 	str[len] = '\0';
	 }

    /**
     * 往子进程写数据
     * */
    void putdata(pid_t child, long addr, char *str, int len) {

	 	char *laddr;
		int i, j;
		union u {
			long val;
			char chars[word_size];
		} data;

		i = 0;
		j = len / word_size;
		laddr = str;

		while (i < j) {
			memcpy(data.chars, laddr, word_size);
			//每次写入一个字
			ptrace(PTRACE_POKEDATA, child, addr + i * 4, data.val);
			++i;
			laddr += word_size;
		}
		j = len % word_size;
		if (j != 0) {
			memcpy(data.chars, laddr, j);
			ptrace(PTRACE_POKEDATA, child, addr + i * 4, data.val);
		}
	}

    int main() {

	 	pid_t child;
		child = fork();
		if (child < 0) {
			perror("fork error");
		} else if (child == 0) {
			//子进程执行
			ptrace(PTRACE_TRACEME, 0, NULL, NULL);
			execl("hello", "hello", NULL);
		} else {
			//父进程执行
			long orig_eax;
			long params[3];
			int status;
			char *str, *laddr;
			int toggle = 0;
			while (1) {
				//等待子进程信号
				wait(&status);
				if (WIFEXITED(status)) //遇到子进程退出信号，退出循环
					break;
				orig_eax = ptrace(PTRACE_PEEKUSER, child, 4 * ORIG_EAX, NULL);
				if (orig_eax == SYS_write) {
					if (toggle == 0) {
						toggle = 1;
						params[0] = ptrace(PTRACE_PEEKUSER, child, 4 * EBX, NULL); 
						params[1] = ptrace(PTRACE_PEEKUSER, child, 4 * ECX, NULL); 
						params[2] = ptrace(PTRACE_PEEKUSER, child, 4 * EDX, NULL); 
						
						str = (char *) calloc((params[2] + 1), sizeof(char));
						getdata(child, params[1], str, params[2]);
						reverse(str);
						putdata(child, params[1], str, params[2]);
					} else {
						toggle = 0;
					}
				}
				ptrace(PTRACE_SYSCALL, child, NULL, NULL);
			}
		}
		return 0;
	}

编译并运行,输出结果如下:

    robin@ubuntu:~/idework/eclipsecdt/ptraceDemo$ ./tracer4
    !dlrow ,olleH

可以看到，"Hello, world!"字符串被反转输出了。
   
 
 
   
###GDB调试
####GDB概述
&nbsp;&nbsp;&nbsp;&nbsp;GDB是GNU开源组织发布的一个强大的UNIX下的程序调试工具。一般来说，GDB主要帮忙你完成下面四个方面的功能：
<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1、启动你的程序，可以按照你的自定义的要求随心所欲的运行程序。<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2、可让被调试的程序在你所指定的调置的断点处停住。<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3、当程序被停住时，可以检查此时你的程序中所发生的事。<br>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4、动态的改变你程序的执行环境。<br>
   
####单步执行
&nbsp;&nbsp;&nbsp;&nbsp;调试器可以跟踪程序执行，监控程序每一条指令的运行，原理其实通过ptrace调用PTRACE\_SINGLESTEP来实现的。下面通过一个小示例来看看PTRACE\_SINGLESTEP单步功能。
#####step.c代码如下:
	#include <stdio.h>
	#include <stdarg.h>
	#include <stdlib.h>
	#include <signal.h>
	#include <syscall.h>
	#include <sys/ptrace.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <sys/reg.h>
	#include <sys/user.h>
	#include <unistd.h>
	#include <errno.h>

	/**
	 * 演示PTRACE_SINGLESTEP
	 * */
	int main(int argc, char** argv) {
	if (argc < 2) {
		perror("set a progarm to exec");
		return -1;
	}
	pid_t child_pid = fork();
	if (child_pid == 0) {
		//子进程执行
		if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
			perror("ptrace_traceme error");
			return -1;
		}
		execl(argv[1], argv[1], 0);
	} else if (child_pid > 0) {
		//父进程执行
		int counter = 0;
		int status;
		while (1) {
			//等待子进程信号
			wait(&status);
			if (WIFEXITED(status)) //子进程发送退出信号，退出循环
				break;
			counter++;
			//调用ptrace从子进程取数据
			struct user_regs_struct regs;
			//取eip寄存器，这里存放的是cpu将要执行的指令地址
			ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
			//取指令内容
			unsigned instr = ptrace(PTRACE_PEEKTEXT, child_pid, regs.eip, 0);
			printf("counter = %u,EIP = 0x%08x,instr = 0x%08x\n", counter,
					regs.eip, instr);
			//重新启动子进程，当子进程执行了下一条指令后再将其停止
			if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
				perror("ptrace singlestep error");
				return -1;
			}
		}
	} else {
		perror("fork error");
		return -1;
	}

	return 0;
	}
该程序fork一个子进程来运行给定程序，然后在父进程里面通过ptrace来控制子进程，我们可以调试一下helloworld,可以看到输出了近10万条记录，在上面的tracer1也有提到，因为要引用C基础库的东西，所以会执行很多指令。为了方便我们观察调试,这里使用汇编语言来编写一个调试目标程序:

    .section .data #数据段
    output:.string "hello,mini\n" #定义字符串变量
    .section .text #代码段
	.globl _start #指定入口
	_start:
	# ssize_t write(int fd, const void *buf, size_t count);
	movl $4, %eax #系统调用号
	movl $1, %ebx #fd
	movl $output, %ecx #buf
	movl $11, %edx #count
	int $0x80
	movl $1, %eax #返回值
	int $0x80
	
把上面这段代码保存为mini.s,通过下面2条命令来编译:<br>

	as mini.s -o mini.o
	ld -o mini mini.o
编译完成后,我们再使用step来调试一下mini:<br>


	robin@ubuntu:~/idework/eclipsecdt/ptraceDemo/gdb$ ./step mini
	counter = 1,EIP = 0x08048074,instr = 0x000004b8
	counter = 2,EIP = 0x08048079,instr = 0x000001bb
	counter = 3,EIP = 0x0804807e,instr = 0x049091b9
	counter = 4,EIP = 0x08048083,instr = 0x00000bba
	counter = 5,EIP = 0x08048088,instr = 0x01b880cd
	hello,mini
	counter = 6,EIP = 0x0804808a,instr = 0x000001b8
	counter = 7,EIP = 0x0804808f,instr = 0x656880cd

 OK，所以现在我们可以看到指令指针以及每一步的指令。如何验证这是否正确呢？可以通过在可执行文件上执行objdump –d来实现：  
 
	robin@ubuntu:~/idework/eclipsecdt/ptraceDemo/gdb$ objdump -d 	mini

	mini:     file format elf32-i386


	Disassembly of section .text:

	08048074 <_start>:
	8048074:	b8 04 00 00 00       	mov    $0x4,%eax
	8048079:	bb 01 00 00 00       	mov    $0x1,%ebx
	804807e:	b9 91 90 04 08       	mov    $0x8049091,%ecx
	8048083:	ba 0b 00 00 00       	mov    $0xb,%edx
	8048088:	cd 80                	int    $0x80
	804808a:	b8 01 00 00 00       	mov    $0x1,%eax
	804808f:	cd 80                	int    $0x80
用这份输出对比我们的跟踪程序输出，应该很容易观察到相同的地方。

###断点
####int 3指令
调试器实现断点功能是通过int 3指令来实现的。<br>
int 3指令产生一个特殊的单字节操作码（CC），这是用来调用调试异常处理例程的。（这个单字节形式非常有价值，因为这样可以通过一个断点来替换掉任何指令的第一个字节，包括其它的单字节指令也是一样，而不会覆盖到其它的操作码）。
####手动设置断点
现在展示如何在程序中设定断点。用于这个示例的目标程序如下：
#####target.s:
       
	.section .data

	hello:.string "hello\n"
	world:.string "world\n"

	.section .text
	.globl _start

	_start:

	#print hello
	movl $4, %eax
	movl $1, %ebx
	movl $hello, %ecx
	movl $6, %edx
	int $0x80

	#print world
	movl $4, %eax
	movl $1, %ebx
	movl $world, %ecx
	movl $6, %edx
	int $0x80

	movl $1, %eax
	int $0x80
	
这里还是使用汇编语言，这样能够避免使用C语言时涉及到的编译和符号的问题。上面列出的程序功能就是在一行中打印“Hello，”，然后在下一行中打印“world”。这个例子与上一个例子很相似。

我们希望设定的断点位置应该在第一条打印之后，但恰好在第二条打印之前。我们就让断点打在第一个int 0×80指令之后吧，也就是movl $4, %eax。首先，需要知道这条指令对应的地址是什么。运行objdump –d：
	
	robin@ubuntu:~/idework/eclipsecdt/ptraceDemo/gdb$ objdump -d target

	target:     file format elf32-i386


	Disassembly of section .text:

	08048074 <_start>:
	8048074:	b8 04 00 00 00       	mov    $0x4,%eax
	8048079:	bb 01 00 00 00       	mov    $0x1,%ebx
	804807e:	b9 a7 90 04 08       	mov    $0x80490a7,%ecx
	8048083:	ba 06 00 00 00       	mov    $0x6,%edx
	8048088:	cd 80                	int    $0x80
	804808a:	b8 04 00 00 00       	mov    $0x4,%eax
	804808f:	bb 01 00 00 00       	mov    $0x1,%ebx
	8048094:	b9 ae 90 04 08       	mov    $0x80490ae,%ecx
	8048099:	ba 06 00 00 00       	mov    $0x6,%edx
	804809e:	cd 80                	int    $0x80
	80480a0:	b8 01 00 00 00       	mov    $0x1,%eax
	80480a5:	cd 80                	int    $0x80
通过上面的输出，我们知道要设定的断点地址是0×804808a。

####通过int 3指令在调试器中设定断点

要在被调试进程中的某个目标地址上设定一个断点，调试器需要做下面两件事情：

1.  保存目标地址上的数据

2.  将目标地址上的第一个字节替换为int 3指令

然后，当调试器向操作系统请求开始运行进程时，进程最终一定会碰到int 3指令。此时进程停止，操作系统将发送一个信号。这时就是调试器再次出马的时候了，接收到一个其子进程（或被跟踪进程）停止的信号，然后调试器要做下面几件事：

1.  在目标地址上用原来的指令替换掉int 3

2.  将被跟踪进程中的指令指针向后递减1。这么做是必须的，因为现在指令指针指向的是已经执行过的int 3之后的下一条指令。

3.  由于进程此时仍然是停止的，用户可以同被调试进程进行某种形式的交互。这里调试器可以让你查看变量的值，检查调用栈等等。

4.  当用户希望进程继续运行时，调试器负责将断点再次加到目标地址上（由于在第一步中断点已经被移除了），除非用户希望取消断点。

让我们看看这些步骤如何转化为实际的代码。

	/* 等待子进程信号 */
    wait(&wait_status);

    /* 取子进程寄存器 */
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
    /* eip是子进程执行的指令地址 */
    printf("Child started. EIP = 0x%08x\n", regs.eip);

    /* 读取地址0x804808a 对应的指令*/
    unsigned addr = 0x804808a;
    unsigned data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)addr, 0);
    printf("Original data at 0x%08x: 0x%08x\n", addr, data);
这里调试器从被跟踪进程中获取到指令指针，然后检查当前位于地址0×804808a处的字长内容。运行程序，将打印出：

	Child started. EIP = 0x08048074
	Original data at 0x0804808a: 0x000004b8


目前为止一切顺利，下一步：

    /* 把'int 3'替换进 0x804808a处 对应的指令*/
    unsigned data_with_trap = (data & 0xFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr, (void*)data_with_trap);

    /* 再读 地址0x804808a处 对应的指令*/
    unsigned readback_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)addr, 0);
    printf("After trap, data at 0x%08x: 0x%08x\n", addr, readback_data);
注意看我们是如何将int 3指令插入到目标地址上的。这部分代码将打印出：

	After trap, data at 0x0804808a: 0x000004cc

再一次如同预计的那样——0xb8被0xcc取代了。调试器现在运行子进程然后等待子进程在断点处停止住。

	/* 让子进程运行直到遇到断点 int 3*/
    ptrace(PTRACE_CONT, child_pid, 0, 0);

    wait(&wait_status);
	if (WIFSTOPPED(wait_status)) {
		printf("Child got a signal: %s\n", strsignal(WSTOPSIG(wait_status)));
	} else {
		perror("wait");
		return;
	}

    /* 看看子进程目前执行到哪了 */
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
    printf("Child stopped at EIP = 0x%08x\n", regs.eip);
    /*等 用户响应*/
    getchar();

这段代码打印出：
	
	hello
	Child got a signal: Trace/breakpoint trap
	Child stopped at EIP = 0x0804808b


注意，“Hello,”在断点之前打印出来了——同我们计划的一样。同时我们发现子进程已经停止运行了——就在这个单字节的陷阱指令执行之后。

按下enter键，继续执行:

	 /* 移除int 3,把原先的指令写回去*/
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr, (void*)data);
    /* 指令地址 -1*/
    regs.eip -= 1;
    ptrace(PTRACE_SETREGS, child_pid, 0, &regs);
    ptrace(PTRACE_CONT, child_pid, 0, 0);
    
    wait(&wait_status);
    if (WIFEXITED(wait_status)) {
    	printf("Child exited\n");
    } else {
    	printf("Unexpected signal\n");
    }
    
输出：

	world
	Child exited



这会使子进程打印出“world”然后退出，同之前计划的一样。

##小结
ptrace是一种比较底层的技术，它为程序调试，进程注入等应用技术提供了基础支持，因为它够底层，所有要想真正完全掌握，需要掌握一些linux系统知识。
##示例代码
所有示例代码已上传到我的github：<br>
<https://github.com/yangbean9/ptraceDemo>
##参考资料
<a href="http://www.linuxjournal.com/article/6100">Playing with ptrace, Part I</a>
<br>
<a href="http://www.linuxjournal.com/article/6210">Playing with ptrace, Part II</a>
<br>
<a href="http://eli.thegreenplace.net/2011/01/23/how-debuggers-work-part-1/">How debuggers work: Part 1 - Basics</a>
<br>
<a href="http://eli.thegreenplace.net/2011/01/27/how-debuggers-work-part-2-breakpoints">How debuggers work: Part 2 - Breakpoints</a>









      
      
      








