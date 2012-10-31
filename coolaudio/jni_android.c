#include "jni_android.h"
#include "libnge2.h"

static JavaVM* javaVM = NULL;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	javaVM = vm;
	return JNI_VERSION_1_2;
}

JavaVM* GetJaveVM()
{
	if(javaVM)
		return javaVM;
	nge_print("JNI_OnLoad error,JavaVm is NULL!");
	return NULL;
}

