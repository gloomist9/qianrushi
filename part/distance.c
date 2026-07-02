#include "main.h"
#include "distance.h"
#define buffer 64

uint8_t  distance_buffer[buffer];
uint16_t  distance;
uint16_t Get_distance()
{
	static uint8_t start = 0;
	static uint8_t start_count = 0;
	static uint8_t baiwei = 0;
	static uint8_t shiwei = 0;
	static uint8_t gewei = 0;
	for(uint8_t count = 0;count<buffer;count++)
	{
		if(distance_buffer[count] == 0x3A )  //0x3A就是 ‘ ：’
		{
			start = 1;
			start_count = count;  //记录此时的count数字
			baiwei = 0;
		    shiwei = 0;
			gewei = 0;
		}
		if(start == 1)  //开始记录数据  两个空格，三个数据位    00128
		{																										// 12345
				
				if(count - start_count > 2) //第一，二位恒为空,跳过第一，二位
				{
					switch(count - start_count)
					{
						case 3:
						if(distance_buffer[count] == 0x20) baiwei = 0;
						else
						baiwei = distance_buffer[count] - '0';
						break;
						case 4:
						if(distance_buffer[count] == 0x20) shiwei = 0;
						else
						shiwei = distance_buffer[count] - '0';
						break;
					  case 5:
						if(distance_buffer[count] == 0x20) gewei = 0;
						else
						gewei = distance_buffer[count] - '0';
						break;
					}
		  	}
			  if(distance_buffer[count] == 0x0A)  //"0x0A"是/n 
				{
					start = 0;
					start_count = 0;
					distance =  baiwei * 100 + shiwei * 10 + gewei;
//					baiwei = 0;
//					shiwei = 0;
//					gewei  = 0;
					return distance;
			  }
			
		}
		
	}
	return distance;

}
uint8_t distancejudge(void)//判断距离是否小于设定值
{
	uint8_t standard_distance = 20;//设定的标准距离
	static uint8_t time = 0;//满三次才返回1
	if(distance <= standard_distance)
	{
		time++;
		if(time >= 3)
		{
			time = 0;
			return 1;
		}
	}
	else
	{
		time = 0;
	}
	return 0;
}
