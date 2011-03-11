
#include "npp.h"
#include "npapi_utils.h"
#include "ntg32.h"
#include "ntghk.h"

#include <string.h>
#include <vector>
#include <map>

namespace
{
	HWND _hWnd = 0;
}

class MyScriptableObject : public nppScriptableObject<MyScriptableObject>
{

	typedef bool (MyScriptableObject::*so_method_t)(const NPVariant*, uint32_t, NPVariant*);
	typedef std::map<NPIdentifier,so_method_t> so_methods_t;
	
	NPP _npp;
	NPObject *_gestureListener;
	
	HWND _hWnd;

public:
	MyScriptableObject(NPP npp)
	:_npp(npp)
	,_gestureListener(0)
	,_hWnd(0)
	{
		DEBUG_LOG("MyScriptableObject: CONSTRUCT");
	}
	
	~MyScriptableObject()
	{
		if(_gestureListener) {
			gNPNFuncs->releaseobject(_gestureListener);
			_gestureListener = 0;
		}
		DEBUG_LOG("MyScriptableObject: DESTRUCT");
	}

	bool hasMethod(NPIdentifier name)
	{
		//DEBUG_LOG("MyScriptableObject.hasMethod");
		return true;
	}
	
	bool invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		static so_methods_t _methods;
		DEBUG_LOG("MyScriptableObject.invoke");
		
		if(_methods.empty()) {
			#define _ADD_SO_METHOD(NAME)   \
				_methods.insert(           \
					so_methods_t::value_type(gNPNFuncs->getstringidentifier(#NAME), &MyScriptableObject::NAME));

			_ADD_SO_METHOD(setGestureListener);
			_ADD_SO_METHOD(postMessage);
			_ADD_SO_METHOD(sendKey);
			_ADD_SO_METHOD(setGestureMask);
			_ADD_SO_METHOD(createProcess);
			
			#undef _ADD_SO_METHOD
		}
		so_methods_t::const_iterator i = _methods.find(name);
		if(i != _methods.end()) {
			return (this->*i->second)(args, argCount, result);
		}
		else {
			std::ostringstream oss;
			oss << (uintptr_t)name;
			DEBUG_LOG("\tNo such method:"+ oss.str());
		}
		return false;
	}
	
	bool invokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		//DEBUG_LOG("MyScriptableObject.invokeDefault");
		return true;
	}
	
	bool hasProperty(NPIdentifier name)
	{
		//DEBUG_LOG("MyScriptableObject.hasProperty");
		return false;
	}
	
	bool getProperty(NPIdentifier name, NPVariant *result)
	{
		//DEBUG_LOG("MyScriptableObject.getProperty");
		return false;
	}
	
	
	//
	// GestureListener にイベントを通知するメソッド
	//

	void fireMouseGestureBegin()
	{
		static const NPIdentifier kIdMouseGestureBegin = gNPNFuncs->getstringidentifier("mouseGestureBegin");
		if(_gestureListener) {
			NPVariant result;
			gNPNFuncs->invoke(_npp, _gestureListener, kIdMouseGestureBegin, 0, 0, &result);
		}
	}

	void fireMouseGestureEnd(uint32_t arrows, HWND hWnd)
	{
		static const NPIdentifier kIdMouseGestureEnd = gNPNFuncs->getstringidentifier("mouseGestureEnd");
		_hWnd = hWnd;
		if(_gestureListener) {
			NPVariant result;
			NPVariant args[1];
			args[0].type = NPVariantType_Int32;
			args[0].value.intValue = static_cast<int32_t>(arrows);
			
			gNPNFuncs->invoke(_npp, _gestureListener,
				kIdMouseGestureEnd, args, sizeof(args) / sizeof(*args), &result);
		}
	}

	void fireMouseGestureProgress()
	{
		static const NPIdentifier kIdMouseGestureProgress = gNPNFuncs->getstringidentifier("mouseGestureProgress");
		if(_gestureListener) {
			NPVariant result;
			gNPNFuncs->invoke(_npp, _gestureListener, kIdMouseGestureProgress, 0, 0, &result);
		}
	}

	void fireRockerGestureEnd(bool isLeft, HWND hWnd)
	{
		static const NPIdentifier kIdRockerGestureEnd = gNPNFuncs->getstringidentifier("rockerGestureEnd");
		_hWnd = hWnd;
		if(_gestureListener) {
			NPVariant result;
			NPVariant args[1];
			args[0].type = NPVariantType_Bool;
			args[0].value.boolValue = isLeft;
			
			gNPNFuncs->invoke(_npp, _gestureListener,
				kIdRockerGestureEnd, args, sizeof(args) / sizeof(*args), &result);
		}
	}

	void fireWheelGestureEnd(bool isUp, HWND hWnd)
	{
		static const NPIdentifier kIdWheelGestureEnd = gNPNFuncs->getstringidentifier("wheelGestureEnd");
		_hWnd = hWnd;
		if(_gestureListener) {
			NPVariant result;
			NPVariant args[1];
			args[0].type = NPVariantType_Bool;
			args[0].value.boolValue = isUp;
			
			gNPNFuncs->invoke(_npp, _gestureListener,
				kIdWheelGestureEnd, args, sizeof(args) / sizeof(*args), &result);
		}
	}

private:

	bool setGestureListener(const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		DEBUG_LOG("MyScriptableObject.setGestureListener: called!");
		if(argCount == 1 && args[0].type == NPVariantType_Object) {
			DEBUG_LOG("MyScriptableObject.setGestureListener: OK.");
			if(_gestureListener) {
				gNPNFuncs->releaseobject(_gestureListener);
			}
			_gestureListener = args[0].value.objectValue;
			gNPNFuncs->retainobject(_gestureListener);
		}
		return true;
	}


	bool postMessage(const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		DEBUG_LOG("MyScriptableObject.postMessage: called!");
		if(argCount == 3
			&& IS_NPNUMBER(args[0])
			&& IS_NPNUMBER(args[1])
			&& IS_NPNUMBER(args[2]))
		{
			if(_hWnd) {
				::PostMessage(_hWnd,
					npnumber_to_int(args[0]),
					npnumber_to_int(args[0]),
					npnumber_to_int(args[0]));
			}
		}
		return true;
	}


	enum modifier_t { MODIF_SHIFT = 1, MODIF_CTRL = 2, MODIF_ALT = 4 };
	static void _sendKey_0(unsigned code, unsigned modifiers)
	{

		INPUT input[8];
		int i = 0;

		#define _KEYBORD_INPUT(KEYCODE, FLAGS)                     \
			input[i].type  = INPUT_KEYBOARD;                       \
			input[i].ki.wVk         = KEYCODE;                     \
			input[i].ki.wScan       = ::MapVirtualKey(KEYCODE, 0); \
			input[i].ki.dwFlags     = FLAGS;                       \
			input[i].ki.time        = 0;                           \
			input[i].ki.dwExtraInfo = 0;                           \
			++i;
		
		#define _SET_MODIFIERS(FLAGS)                                           \
			for(int j = 1; j <= MODIF_ALT; j <<= 1) {                           \
				if(modifiers & j) {                                             \
					switch(modifier_t(j)) {                                     \
					case MODIF_SHIFT: _KEYBORD_INPUT(VK_SHIFT  , FLAGS); break; \
					case  MODIF_CTRL: _KEYBORD_INPUT(VK_CONTROL, FLAGS); break; \
					case   MODIF_ALT: _KEYBORD_INPUT(VK_MENU   , FLAGS); break; \
					}                                                           \
				}                                                               \
			}
		
		_SET_MODIFIERS(0);
		_KEYBORD_INPUT(code, 0);
		_KEYBORD_INPUT(code, KEYEVENTF_KEYUP);
		_SET_MODIFIERS(KEYEVENTF_KEYUP);
		
		::SendInput(i, input, sizeof(INPUT));
		
		#undef _SET_MODIFIERS
		#undef _KEYBORD_INPUT
	}
	
	bool sendKey(const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		DEBUG_LOG("MyScriptableObject.sendKey: called!");
		// Chrome 10 から、数値リテラルの値が NPVariantType_Int32 ではなく NPVariantType_Double になったっぽい
		if(argCount == 2
			&& IS_NPNUMBER(args[0])
			&& IS_NPNUMBER(args[1]))
		{
			_sendKey_0(npnumber_to_int32(args[0]), npnumber_to_int32(args[1]));
		}
		return true;
	}

	
	bool setGestureMask(const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		if(argCount == 1
			&& args[0].type == NPVariantType_Int32)
		{
			ntghk_setGestureMask(args[0].value.intValue);
		}
		return true;
	}


	bool createProcess(const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		if(argCount == 2
			&& args[0].type == NPVariantType_String
			&& args[1].type == NPVariantType_String)
		{
		
			const NPString &commandLineRaw = args[1].value.stringValue;
			std::vector<char> commandLine(commandLineRaw.UTF8Characters,
				commandLineRaw.UTF8Characters + strlen(commandLineRaw.UTF8Characters) + 1);
			// MEMO: UTF8Length は多分、UTF8をデコードした時の文字数？

			STARTUPINFO startUpInfo = {0};
			PROCESS_INFORMATION processInfo;

			::CreateProcess(
				args[0].value.stringValue.UTF8Characters,
				&commandLine[0],
				NULL,
				NULL,
				FALSE,
				CREATE_DEFAULT_ERROR_MODE,
				NULL,
				NULL,
				&startUpInfo,
				&processInfo
			);
			
			// ISSUE: つか、NPStringって開放しなきていいの？
		}
		return true;
	}
};

static NPObject *_scriptableObject = 0;


NPError NP_LOADDS NPP_New(NPMIMEType pluginType, NPP instance,
						  uint16_t mode, int16_t argc, char* argn[],
						  char* argv[], NPSavedData* saved)
{
	//DEBUG_LOG("NP_New");
	return NPERR_NO_ERROR;
}

NPError NP_LOADDS NPP_Destroy(NPP instance, NPSavedData** save) {
	//DEBUG_LOG("NPP_Destroy");
	return NPERR_NO_ERROR;
}

NPError NP_LOADDS NPP_GetValue(NPP instance, NPPVariable variable, void* value)
{
	//DEBUG_LOG("NPP_GetValue");

	switch(variable) {
	case NPPVpluginNameString:
		*reinterpret_cast<char**>(value) = "NTgesture";
		break;
		
	case NPPVpluginDescriptionString:
		*reinterpret_cast<char**>(value) = "NTgesture for Google Chrome.";
		break;
		
	case NPPVpluginScriptableNPObject:	
		if(!_scriptableObject) {
			_scriptableObject = gNPNFuncs->createobject(instance, &MyScriptableObject::getNPClass());
		}
		gNPNFuncs->retainobject(_scriptableObject);
		*reinterpret_cast<NPObject**>(value) = _scriptableObject;
		break;

	default:
		return NPERR_GENERIC_ERROR;
	}
	return NPERR_NO_ERROR;
}

NPError NP_LOADDS NPP_HandleEvent(NPP instance, void* ev) {
	//DEBUG_LOG("NPP_HandleEvent");
	return NPERR_NO_ERROR;
}

NPError NP_LOADDS NPP_SetWindow(NPP instance, NPWindow* pNPWindow) {
	//DEBUG_LOG("NPP_SetWindow");
	_hWnd = reinterpret_cast<HWND>(pNPWindow->window);
	return NPERR_NO_ERROR;
}



namespace ntg
{

	void fireMouseGestureBegin()
	{
		if(_scriptableObject) {
			static_cast<MyScriptableObject*>(_scriptableObject)->fireMouseGestureBegin();
		}
	}

	void fireMouseGestureEnd(uint32_t arrows, HWND hWnd)
	{
		if(_scriptableObject) {
			static_cast<MyScriptableObject*>(_scriptableObject)->fireMouseGestureEnd(arrows, hWnd);
		}
	}

	void fireMouseGestureProgress()
	{
		if(_scriptableObject) {
			static_cast<MyScriptableObject*>(_scriptableObject)->fireMouseGestureProgress();
		}
	}
	
	void fireRockerGestureEnd(bool isLeft, HWND hWnd)
	{
		if(_scriptableObject) {
			static_cast<MyScriptableObject*>(_scriptableObject)->fireRockerGestureEnd(isLeft, hWnd);
		}
	}
	
	void fireWheelGestureEnd(bool isUp, HWND hWnd)
	{
		if(_scriptableObject) {
			static_cast<MyScriptableObject*>(_scriptableObject)->fireWheelGestureEnd(isUp, hWnd);
		}
	}

}


