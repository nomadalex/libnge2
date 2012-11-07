#ifndef NGE_UTILSKIT_H_
#define NGE_UTILSKIT_H_
#include "nge_common.h"

typedef void (*PayFeedBackCall)(int error,char* errorStr,void* userdata);

#ifdef __cplusplus
extern "C"{
#endif

NGE_API void UtilsKitDefaultInit();


NGE_API void UtilsKitDefaultFini();

//for ad
NGE_API int  CreateAdHandle(const char* adKey,const char* adType);

NGE_API void SetAdPos(int adHandle,int x,int y);

NGE_API void ShowAd(int adHandle,int bShow);

NGE_API void DestroyAdHandle(int adHandle);

//for payment
NGE_API int CreatePayHandle(const char* payType,const char* partner,const char* seller,const char* privateRSA,const char* publicRSA);

NGE_API int Pay(int payHandle,const char* title,const char* body,const char* total,PayFeedBackCall pCall,void* userData);

// for share
NGE_API int Share(int shareHandle,const char* str,const char* imgName);

//for more game
NGE_API void MoreGame(int type);

#ifdef __cplusplus
}
#endif

#endif
