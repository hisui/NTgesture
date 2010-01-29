
#include "ntg.h"
#include "npp.h"
#include "daemon.h"

// NP_Initialize で初期化される
NPNetscapeFuncs *gNPNFuncs = 0;

// アプリケーションハンドルとかいうやつ
HINSTANCE gHInstance = 0;

namespace {
	bool _initialized = false;
}

// DLL でエクスポートされる関数たち
extern "C" {

	BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpvReserved)
	{
		gHInstance = hInst;
		switch(fdwReason) {
		case DLL_PROCESS_ATTACH: break;
		case DLL_PROCESS_DETACH: break;
		case  DLL_THREAD_ATTACH: break;
		case  DLL_THREAD_DETACH: break;
		}
		return  TRUE;
	}

	NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pFuncs) {
		//DEBUG_LOG("NP_GetEntryPoints");
		pFuncs->version   = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
		pFuncs->newp      = NPP_New;
		pFuncs->destroy   = NPP_Destroy;
		pFuncs->getvalue  = NPP_GetValue;
		pFuncs->event     = NPP_HandleEvent;
		pFuncs->setwindow = NPP_SetWindow;
		return NPERR_NO_ERROR;
	}

	NPError OSCALL NP_Initialize(NPNetscapeFuncs* aNPNFuncs)
	{
		//DEBUG_LOG("NP_Initialize");
		gNPNFuncs = aNPNFuncs;

		if(_initialized) {
			return NPERR_NO_ERROR;
		}
		_initialized = true;
		ntg::execDaemon();
		
		return NPERR_NO_ERROR;
	}

	NPError OSCALL NP_Shutdown()
	{
		//DEBUG_LOG("NP_Shutdown");
		if(_initialized) {
			ntg::stopDaemon();
		}
		return NPERR_NO_ERROR;
	}

	char* NP_GetMIMEDescription()
	{
		//DEBUG_LOG("NP_GetMIMEDescription");
		return "application/x-ntg-plugin";
	}
}
