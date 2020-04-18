代码位于github：https://github.com/ActivePeter/esp32_flappy_bird

硬件平台 ttgo-t-display (esp32)

软件平台platform io in vscode

### （一）驱动部分

> 显示屏
>
> ```c
> #include <TFT_eSPI.h>
> ```
>
> 按键
>
> ```c
> #include <Button2.h>
> ```

#### 对显示屏部分做出了一定的修改

加入了显示信息缓冲区`unsigned short buffer[32400]={0};`

加入了往显示屏缓冲区绘制长方体的函数

```c
void drawRectToBuffer(int x,int y,int w,int h,u_short color){
  int i,j;
  if(x<0){
    w=w+x;
    x=0;
  }
  if(y<0){
    h=h+y;
    y=0;
  }
  if(x+w>ScreenW){//修正超出范围
    w=ScreenW-x;
  }
  if(y+h>ScreenH){//修正超出范围
    h=ScreenH-y;
  }
  for(i=0;i<h;i++){
    for(j=0;j<w;j++){
      buffer[(j+x)*ScreenH+(i+y)]=color;

    }
  }

}
```

#### 初始化显示屏

```c
void initTFT(){
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  

  if (TFT_BL > 0)
  {                          // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
    digitalWrite(TFT_BL, 1);
                             // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
  }
}
```



------

### （二）逻辑实现部分

#### 两个结构体

```c
typedef struct{
  float x;
  u_char height;

} Pillar;
typedef struct{
  float y;
  float speed;

} Bird;
```

用来描述小鸟和障碍物。

------

#### 整体结构

1. 循环开头，清空显示屏缓冲区
2. 更新数据
3. 然后将新的数据对应的图形绘制到缓冲区。
4. 将缓冲区内容刷新到显示屏

#### 最主要的逻辑部分就在于更新数据

```c
 if(!dead){
    drawRectToBuffer(birdX,(int)bird.y,birdW,birdW,0xFFE0);
    updatePillars();
    updateBird();
    judgeColid();

  }else{
    drawRectToBuffer(birdX,(int)bird.y,birdW,birdW,0xF800)
  }
```

##### （1）更新障碍物，每次增加的x值的大小决定了速度。

然后做一个判断。有没有超出屏幕范围。如果超出了。就把他放到最前面。

```c
void updatePillars(){
  pillars[0].x+=0.6f;
  pillars[1].x+=0.6f;
  pillars[2].x+=0.6f;
  if(pillars[0].x>=ScreenW){
    pillars[0].x=pillars[2].x-PillarSpace-PillarW;
    pillars[0].height=rand()%160+20;
  }
  if(pillars[1].x>=ScreenW){
    pillars[1].x=pillars[0].x-PillarSpace-PillarW;
    pillars[1].height=rand()%160+20;
  }
  if(pillars[2].x>=ScreenW){
    pillars[2].x=pillars[1].x-PillarSpace-PillarW;
    pillars[2].height=rand()%160+20;
  }
}
```

##### （2）更新小鸟

1.每次速度都增加一定值（模拟重力）

2.再给速度减掉一个（t*速度^2）（模拟阻力）

3.将速度加到小鸟的y坐标上

```c
void updateBird(){
  bird.speed+=0.15f;
  bird.speed-=0.015f*(bird.speed*abs(bird.speed));
  bird.y+=bird.speed;
}
```

##### （3）判断碰撞

判断x方向上，小鸟方块是否和障碍物有交集

如果有交集就给curpillar赋值。

然后在判断有交集的情况下y方向上有没有碰撞就行了。

------

同时记录lastpillar

如果上一次pillar值记录为1或2或3

这一次记录为0

对应的就是鸟从上一个障碍物中飞出，

此时对应的score++

![](https://img-blog.csdnimg.cn/20200418231549197.png)

```c
void judgeColid(){
  u_char curPillar=0;

  if(pillars[0].x+PillarW>=birdX&&pillars[0].x<=birdX||pillars[0].x+PillarW>=birdX+birdW&&pillars[0].x<=birdX+birdW){
    curPillar=1;
  }else if(pillars[1].x+PillarW>=birdX&&pillars[1].x<=birdX||pillars[1].x+PillarW>=birdX+birdW&&pillars[1].x<=birdX+birdW){
    curPillar=2;
  }else if(pillars[2].x+PillarW>=birdX&&pillars[2].x<=birdX||pillars[2].x+PillarW>=birdX+birdW&&pillars[2].x<=birdX+birdW){
    curPillar=3;
  }
  if(lastPillar&&(!curPillar)){
    score++;
  }
  lastPillar=curPillar;
  if(curPillar){
    if(bird.y<=ScreenH - pillars[curPillar-1].height -KongxiHeight){
      dead=true;
    }else if(bird.y+birdW>=ScreenH - pillars[curPillar-1].height){
      dead=true;
    }
  }else{
    if(bird.y<0||bird.y+birdW>=ScreenH){
      dead=true;
    }
  }
}
```

