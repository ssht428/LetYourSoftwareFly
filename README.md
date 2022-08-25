# LetYourSoftwareFly   
Optimization of software/algorithm.This is a translation of an article "让你的软件飞起来",and corresponding demo test 。   

The file "让你的软件飞起来.pdf" is the original article. I saw it is over 15years ago,and think that's what i need to do when i code some "fancy" application.   

Nowadays,the software/APP are get more and more big and choppy, there are too many "programmer" just realizes basic functions use default library no matter how slow it is.   

So I upload and translate it for someone who maybe needed.    
Mine demo test is coding with VS2015 on Windows10,Hardware is Thinkbook 14s(CPU:i7-1065G7@1.30GHz,Memory:16G).   
Demo test is convert a RGB bmp image to gray scale. 
Maybe the CPU is really fast,so optimization is not significant as article write.

simple float operation use about 2680 - 3000us, search table and parallel execution use about 610 - 650us,
it's about 4.5 times faster.

Test result is : 
！[CodeRunTime](/LetYourSoftwareFly_result.png)
Test image is :
！[TestImage](/lena.bmp)
Gray image is :
！[GrayImage](/lena_gray.jpg)

Embedded system still not test,maybe later will test on ARM Cortex-M0 STM32F0 MCU.   

Downside is simple translation of original article.   

===========================================================================================================  

Let Your Software Fly

 conquer_2007@163.com   2005.01.13   

Speed depend on Algorithm   
Different results from different methods. Such as auto-engines faster than horse,but can't faster than the speed of sound; Turbine-engines can easily break the sound barrier,but can't fly out of the earth; Rocket-engines can escape the pull of the Earth's gravity,and even get out of of the solar system.   

Speed of code depends on some aspects,such as:   
1. Complexity of algorithm, exp. MPEG is complex than JPEG, JPEG is complex than BMP's encoding.    
2. CPU's speed, bus-width and architecture.   
3. Your code implementation.   
This article mainly on introduce how to optimize your code to speed up application.   

First check the project's requirements:      
We have a project of image pattern recognition,which need first convert RGB image to gray format. The formula of transformation is :   
Y = 0.299 * R + 0.587 * G + 0.114 * B;   
Here we choose the well-known image "lena.bmp", it's size is 512 * 512 * 24bit, and read all data into memory.   

First optimization:   

#define XSIZE 512   
#define YSIZE 512   
#define IMGSIZE XSIZE*YSIZE   
typedef struct   
{    
 unsigned char R;   
 unsigned char G;   
 unsigned char B;   
}RGB;    
RGB in[IMGSIZE];    
unsigned char out[IMGSIZE];    

Can you see where is the optimization?    

^_^,The answer is: image is a 2D array,here i use a 1D array to store it's data. Most compiler deal with 1D array is faster/more efficient than 2D array.   

 Let's write first implementation function code:   
 void calc_lum()   
 {  
    int i;  
    for(i=0;i<IMGSIZE;i++)   
    {  
        double r,g,b,y;  
        unsigned char yy;  
        r=in[i].r;g=in[i].g;b=in[i].b;  
        y=0.299*r+0.587*g+0.144*b;  
        yy=y; out[i]=yy;  
    }  
 }  
This maybe the simplest(first-thought) way,and also there is no bug.   
OK,let build and run it.  
This code is compiled with VC6.0 and GCC,generate 2 version which runs on PC and mine embedded system.  
How is the speed? It will scare you to death <_<.  
On PC,Because of the existence of hardware float processor and high frequency CPU, image convert time is 20 seconds.  
On mine embedded system, without hardware float processor,all float operation are splitted to integer commands by compiler, final convert time is about 120s.  
And it's just one image processing time!  

Get rid of float operation.  
The upside code will cost lots of time even before run it,because there are massive float operation. It will definitely faster as long as no float computation.  
y=0.299*r+0.587*g+0.144*b;  
So how to use integer to replace float computation in upside formula?  

Simplest way is multiply 1000 to all ratios:  
Y = (R*299 + G*587 + B*114)/1000;  

Now how faster will it be?  
PC : 2s  
MCU : 45s  

Let simplify the formula further:  
0.299 = 299/1000 = 1224/4096  
so  
Y = (R*1224 + G*2440 + B*467)/4096  
here we can replace the division /4096 with right-shift 12bit :-)  
y=(1224*r+2440*g+467*b) >> 12;  
The speed is about 20% faster.  

Still Too Slow!  
Although it is much faster than first implementation,but it still can't acceptable with 20s to process one image.  

But it seems there is no more optimization of the formula. so if you want break the sound barrier, you must throw the piston engine, install a turbine engine.  

Let's see the formula again:  
y=0.299*r+0.587*g+0.144*b;  
Y = D + E + F;  
D=0.299*R; E=0.587*G; F=0.144*B;  
We already know rgb's value is in range 0~255, so can we pre-calculate all D/E/F 's value and then use search table to calculate final result?  
We can use 3 arrays to store D/E/F's 256 values,and ....  

Initialize Table Array.  
int D[256],E[256],F[256];  
void table_init()  
{  
  int i;  
  for(i=0;i<256;i++)  
  {  
    D[i]=i*1224;  D[i]=D[i]>>12;  
    E[i]=i*2404;  E[i]=E[i]>>12;  
    F[i]=i*467;  F[i]=F[i]>>12;  
  }  
}  

//use search table   
void calc_lum()  
{  
  int i;  
  for(i=0;i<IMGSIZE;i++)  
  {  
     int r,g,b,y;  
     r=D[in[i].r];  
     g=E[in[i].g];  
     b=F[in[i].b];  
     y=r+g+b;  
     out[i]=y;  
  }  
}  

Break Sound Barrier!  
This time the result is really scared me. Execute time is improved from 30s to 2s on MCU,it's more than 15times.  

Is there any possible to run much faster?  
There are at least 2ALU in most 32bit embedded system MCU,so let those 2 ALU run parallel:  
void calc_lum()  
{  
  int i;  
  for(i=0;i<IMGSIZE;i+=2)   // process 2 data in one cycle  
  {  
     int r,g,b,y,   r1,g1,b1,y1;  
     r=D[in[i].r];     g=E[in[i].g];     b=F[in[i].b];  
     y=r+g+b;     out[i]=y;  
     
     r1=D[in[i+1].r];     g1=E[in[i+1].g];     b1=F[in[i+1].b];  
     y1=r1+g1+b1;     out[i+1]=y1;  
  }  
}  

Final result is : 1 second.  

Come on! Make it faster!  
After we repeated-trials, we can change  
int D[256],E[256],F[256];  
to  
unsigned shor D[256],E[256],F[256];  

This is because the compilation performance is different between int and unsigned short type.  

So modify the code to:  

unsigned shor D[256],E[256],F[256];  
inline void calc_lum()  
{  
  int i;  
  for(i=0;i<IMGSIZE;i+=2)   // process 2 data in one cycle  
  {  
     int r,g,b,y,   r1,g1,b1,y1;  
     r=D[in[i].r];     g=E[in[i].g];     b=F[in[i].b];  
     y=r+g+b;     out[i]=y;  
     
     r1=D[in[i+1].r];     g1=E[in[i+1].g];     b1=F[in[i+1].b];  
     y1=r1+g1+b1;     out[i+1]=y1;  
  }  
}  

By declare a function "inline", it integrate that function’s code into the code for its callers. This makes execution faster by eliminating the function-call overhead;   

After those 2 improvements,the execution time is : 0.5S.  

Now it finally meet customer requirements ^*^  

Actually,we can fly out the earth.  
It will much faster if we :  
1. Put the table into CPU's high speed cache.  
2. Use assemble code the calc_lum() function.   

The cpu's potential ability is more than you think.  
1. Don't complain your CPU, remember: Bricks can fly as long as power is enough.  
2. Same requirement,different way, run time can change from 120s to 0.5s, so it depends on how to dig out the cpu's potential ability.  
3. If microsoft's engineers can optimize all code like me,I think it can run windows XP on 486.  














