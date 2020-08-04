
#include <napi.h>
#include <chrono>

#include "interfaces.h"
#include "dcp.h"

#ifdef WIN32
  #include <winsock2.h>
#endif


/**
 * 
 */
static void init_npcap_dll_path(const Napi::Env& env)
{
#ifdef WIN32
	BOOL(WINAPI *SetDllDirectory)(LPCTSTR);
	char sysdir_name[512] = {0};
	int len = 0;

	SetDllDirectory = (BOOL(WINAPI *)(LPCTSTR)) GetProcAddress(GetModuleHandle("kernel32.dll"), "SetDllDirectoryA");
	if (SetDllDirectory == NULL) {
    throw Napi::Error::New(env, "Error in SetDllDirectory");
	}
	else {
		len = GetSystemDirectory(sysdir_name, 480);	//	be safe

		if (!len)
			throw Napi::Error::New(env, "Error in GetSystemDirectory");
		
    strcat(sysdir_name, "\\Npcap");
		
    if (SetDllDirectory(sysdir_name) == 0)
			throw Napi::Error::New(env, "Error in SetDllDirectory(\"System32\\Npcap\")");
	}
#endif
}


/**
 * 
 */
Napi::Object Init(Napi::Env env, Napi::Object exports) {

  init_npcap_dll_path(env);

	unsigned int seed = (unsigned int)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	srand(seed);

  exports.Set("listInterfaces", Napi::Function::New(env, ListInterfaces));
  exports.Set("dcpIdentify", Napi::Function::New(env, DcpIdentify));
  exports.Set("dcpGet", Napi::Function::New(env, DcpGet));
  exports.Set("dcpSet", Napi::Function::New(env, DcpSet));

  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
