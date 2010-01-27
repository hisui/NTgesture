
#include <npapi.h>
#include <npruntime.h>

class nppScriptableObjectBase
{
public:
	bool hasMethod(NPIdentifier name)
	{
		return false;
	}
	
	bool invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		return false;
	}
	
	bool invokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		return false;
	}
	
	bool hasProperty(NPIdentifier name)
	{
		return false;
	}
	
	bool getProperty(NPIdentifier name, NPVariant *result)
	{
		return false;
	}
};

// ScriptableObject を作るためのテンプレ。未完成
template<class Self> class nppScriptableObject : public NPObject, public nppScriptableObjectBase
{

public:
	static NPClass &getNPClass()
	{
	
		#define _IF_OVERRIDDEN(NAME, BRIDGE, TYPE, ...) (               \
			(TYPE (Self::*)(__VA_ARGS__))&Self::NAME ==                 \
			(TYPE (Self::*)(__VA_ARGS__))&nppScriptableObjectBase::NAME \
				? NULL : BRIDGE)                                        \

		static NPClass npClass = {
			NP_CLASS_STRUCT_VERSION,
			NULL,
			NULL,
			NULL,
			_IF_OVERRIDDEN(hasMethod    , NPHasMethod    , bool, NPIdentifier),
			_IF_OVERRIDDEN(invoke       , NPInvoke       , bool, NPIdentifier, const NPVariant*, uint32_t, NPVariant*),
			_IF_OVERRIDDEN(invokeDefault, NPInvokeDefault, bool, const NPVariant*, uint32_t, NPVariant*),
			_IF_OVERRIDDEN(hasProperty  , NPHasProperty  , bool, NPIdentifier),
			_IF_OVERRIDDEN(getProperty  , NPGetProperty  , bool, NPIdentifier, NPVariant*),
			NULL,
			NULL,
		};
		
		return npClass;
		
		#undef _IF_OVERRIDDEN
	}

private:

	static NPObject *NPAllocate(NPP npp, NPClass aClass)
	{
		return new Self(npp);
	}
	
	static void NPDeallocate(NPObject *npobj)
	{
		Self *self = static_cast<Self*>(npobj);
		delete self;
	}

	static bool NPHasMethod(NPObject *npobj, NPIdentifier name)
	{
		return static_cast<Self*>(npobj)->hasMethod(name);
	}

	static bool NPInvoke(NPObject *npobj,
		NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		return static_cast<Self*>(npobj)->invoke(name, args, argCount, result);
	}

	static bool NPInvokeDefault(NPObject *npobj,
		const NPVariant *args, uint32_t argCount, NPVariant *result)
	{
		return static_cast<Self*>(npobj)->invokeDefault(args, argCount, result);
	}

	static bool NPHasProperty(NPObject *npobj, NPIdentifier name)
	{
		return static_cast<Self*>(npobj)->hasProperty(name);
	}

	static bool NPGetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result)
	{
		return static_cast<Self*>(npobj)->getProperty(name, result);
	}

};



