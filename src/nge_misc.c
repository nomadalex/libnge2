#include "nge_misc.h"
#include "nge_charsets.h"

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
