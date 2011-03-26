#include "nge_misc.h"
#include "nge_charsets.h"
#include <time.h>

//一些有用的公共函数
//生成一个min---max的浮点数
float rand_float(float min, float max)
{
	static int g_seed ;
	static int inited = 0;
	if(inited == 0){
		g_seed = time(NULL);
		inited = 1;
	}
	g_seed=214013*g_seed+2531011;
	return min+(g_seed>>16)*(1.0f/65535.0f)*(max-min);
}

//生成一个(min,max)的整数
int rand_int(int min, int max)
{
	static int g_seed ;
	static int inited = 0;
	if(inited == 0){
		g_seed = time(NULL);
		inited = 1;
	}
	g_seed=214013*g_seed+2531011;
	return min+(g_seed ^ g_seed>>15)%(max-min+1);
}

int gbk_to_unicode(uint16* out,const char* in,int n)
{
	int i=0;
	int j=0;
	uint16 gbcode = 0;
	while(n-1>=i){
		if (in[i]&0x80) {
			if(n-i>1){
				gbcode=(in[i+1]<<8&0xff00)+(in[i]&0x00ff);
				out[j]=charsets_gbk_to_ucs((uint8 *)&gbcode);
				j++;
				i=i+2;
			}
			else{
				i++;
				continue;
			}
		}
		else{
			gbcode = in[i]&0x00ff;
			out[j] = gbcode;
			i++;
			j++;
		}

	}
	return j;
}
