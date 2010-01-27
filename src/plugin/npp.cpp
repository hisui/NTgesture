
#include "npp.h"
#include "nppScriptableObject.h"
#include "ntg32.h"
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
		//debugPrint("MyScriptableObject: CONSTRUCT");
	}
	
	~MyScriptableObject()
	{
		if(_gestureListener) {
			gNPNFuncs->releaseobject(_gestureListener);
		}
		//debugPrint("MyScriptableObject: DESTRUCT");
	}

	bool hasMethod(NPIdentifier name)
	{
		//debugPrint("MyScriptableObject.hasMethod");
		return true;
	}
	
	bool invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		static so_methods_t _methods;
		if(_methods.empty()) {
			#define _ADD_SO_METHOD(NAME)   \
				_methods.insert(           \
					so_methods_t::value_type(gNPNFuncs->getstringidentifier(#NAME), &MyScriptableObject::NAME));

			_ADD_SO_METHOD(setGestureListener);
			_ADD_SO_METHOD(postMessage);
			_ADD_SO_METHOD(sendKey);
			
			#undef _ADD_SO_METHOD
		}
		so_methods_t::const_iterator i = _methods.find(name);
		debugPrint("MyScriptableObject.invoke");
		if(i != _methods.end()) {
			return (this->*i->second)(args, argCount, result);
		}
		else {
			std::ostringstream oss;
			oss << (uintptr_t)name;
			debugPrint("\tNo such method:"+ oss.str());
		}
		return false;
	}
	
	bool invokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		//debugPrint("MyScriptableObject.invokeDefault");
		return true;
	}
	
	bool hasProperty(NPIdentifier name)
	{
		//debugPrint("MyScriptableObject.hasProperty");
		return false;
	}
	
	bool getProperty(NPIdentifier name, NPVariant *result)
	{
		//debugPrint("MyScriptableObject.getProperty");
		return false;
	}
	
	
	//
	//
	//

	void fireMouseGestureBegin()
	{
		static NPIdentifier kIdMouseGestureBegin = gNPNFuncs->getstringidentifier("mouseGestureBegin");
		if(_gestureListener) {
			NPVariant result;
			gNPNFuncs->invoke(_npp, _gestureListener, kIdMouseGestureBegin, 0, 0, &result);
		}
	}

	void fireMouseGestureEnd(uint32_t arrows, HWND hWnd)
	{
		static NPIdentifier kIdMouseGestureEnd = gNPNFuncs->getstringidentifier("mouseGestureEnd");
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
		static NPIdentifier kIdMouseGestureProgress = gNPNFuncs->getstringidentifier("mouseGestureProgress");
		if(_gestureListener) {
			NPVariant result;
			gNPNFuncs->invoke(_npp, _gestureListener, kIdMouseGestureProgress, 0, 0, &result);
		}
	}

	void fireRockerGestureEnd(bool isLeft, HWND hWnd)
	{
		static NPIdentifier kIdRockerGestureEnd = gNPNFuncs->getstringidentifier("rockerGestureEnd");
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

private:


	bool setGestureListener(const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		//debugPrint("MyScriptableObject.setGestureListener: called!!!");
		if(argCount == 1 && args[0].type == NPVariantType_Object) {
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
		if(argCount == 3
			&& args[0].type == NPVariantType_Int32
			&& args[1].type == NPVariantType_Int32
			&& args[2].type == NPVariantType_Int32)
		{
			if(_hWnd) {
				::PostMessage(_hWnd, args[0].value.intValue, args[1].value.intValue, args[2].value.intValue);
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
		if(argCount == 2
			&& args[0].type == NPVariantType_Int32
			&& args[1].type == NPVariantType_Int32)
		{
			_sendKey_0(args[0].value.intValue, args[1].value.intValue);
		}
		return true;
	}

};

static NPObject *_scriptableObject = 0;


NPError NP_LOADDS NPP_New(NPMIMEType pluginType, NPP instance,
						  uint16_t mode, int16_t argc, char* argn[],
						  char* argv[], NPSavedData* saved)
{
	//debugPrint("NP_New");
	return NPERR_NO_ERROR;
}

NPError NP_LOADDS NPP_Destroy(NPP instance, NPSavedData** save) {
	//debugPrint("NPP_Destroy");
	return NPERR_NO_ERROR;
}

NPError NP_LOADDS NPP_GetValue(NPP instance, NPPVariable variable, void* value)
{
	//debugPrint("NPP_GetValue");

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
	//debugPrint("NPP_HandleEvent");
	return NPERR_NO_ERROR;
}

NPError NP_LOADDS NPP_SetWindow(NPP instance, NPWindow* pNPWindow) {
	//debugPrint("NPP_SetWindow");
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

}


