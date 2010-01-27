package ntg.settingModelClasses
{

	import ntg.NtgISettingModel;

	public final class NtgSettingRoot implements NtgISettingModel
	{
	
		private var _isMouseGestureEnabled:Boolean;
		
		private var _isLockerGestureEnabled:Boolean;
		
		private var _isWheelGestureEnabled:Boolean;
	
		private var _useLLMouseHook:Boolean;
		
		private var _replaceDLLByJavaScript:Boolean;
		
		private var _processModuleFileName:String;
		
		private var _useShortcutKey:Boolean;

		private var _handlers:Array;
		
		private var _mappings:Array;

		public function NtgSettingRoot()
		{
		}
		
		
		public function get isMouseGestureEnabled():Boolean
		{
			return _isMouseGestureEnabled;
		}
		public function set isMouseGestureEnabled(isMouseGestureEnabled:Boolean):void
		{
			_isMouseGestureEnabled = isMouseGestureEnabled;
		}
		
		public function get isLockerGestureEnabled():Boolean
		{
			return _isLockerGestureEnabled;
		}
		public function set isLockerGestureEnabled(isLockerGestureEnabled:Boolean):void
		{
			_isLockerGestureEnabled = isLockerGestureEnabled;
		}
		
		public function get isWheelGestureEnabled():Boolean
		{
			return _isWheelGestureEnabled;
		}
		public function set isWheelGestureEnabled(isWheelGestureEnabled:Boolean):void
		{
			_isWheelGestureEnabled = isWheelGestureEnabled;
		}
		
		public function get useLLMouseHook():Boolean
		{
			return _useLLMouseHook;
		}
		public function set useLLMouseHook(useLLMouseHook:Boolean):void
		{
			_useLLMouseHook = useLLMouseHook;
		}
		
		public function get replaceDLLByJavaScript():Boolean
		{
			return _replaceDLLByJavaScript;
		}
		public function set replaceDLLByJavaScript(replaceDLLByJavaScript:Boolean):void
		{
			_replaceDLLByJavaScript = replaceDLLByJavaScript;
		}
		
		public function get processModuleFileName():String
		{
			return _processModuleFileName;
		}
		public function set processModuleFileName(processModuleFileName:String):void
		{
			_processModuleFileName = processModuleFileName;
		}
		
		public function get useShortcutKey():Boolean
		{
			return _useShortcutKey;
		}
		public function set useShortcutKey(useShortcutKey:Boolean):void
		{
			_useShortcutKey = useShortcutKey;
		}
		
		public function get handlers():Array
		{
			return _handlers;
		}
		public function set handlers(handlers:Array):void
		{
			_handlers = handlers;
		}
		
		public function get mappings():Array
		{
			return _mappings;
		}
		public function set mappings(mappings:Array):void
		{
			_mappings = mappings;
		}


		public function read(input:Object):void
		{

			 _isMouseGestureEnabled = input.isMouseGestureEnabled;
			_isLockerGestureEnabled = input.isLockerGestureEnabled;
			 _isWheelGestureEnabled = input.isWheelGestureEnabled;
			        _useLLMouseHook = input.useLLMouseHook;
			_replaceDLLByJavaScript = input.replaceDLLByJavaScript;
			 _processModuleFileName = input.processModuleFileName;
			        _useShortcutKey = input.useShortcutKey;
     
     		_handlers = [];
     		_mappings = [];
     		
			for(var name:String in input.handlers) {
				
				var handlerInput:Object = input.handlers[name];
				var handler:NtgHandler = new NtgHandler;
				handler.read(handlerInput);
				handler.name = name;
				_handlers.push(handler);
				
				var mappingInputList:Object = handlerInput.mappings;
				for(var key:String in mappingInputList) {
					
					var mapping:NtgMapping = new NtgMapping;
					mapping.read(mappingInputList[key]);
					
					var g:NtgGestureInput;
					switch(mapping.gestureType) {
					case  "mouse": g = new  NtgMouseGestureInput; break;
					case "rocker": g = new NtgRockerGestureInput; break;
					}
					g.read(key);
					mapping.gestureInput = g;
					mapping.handlerName  = name;
					_mappings.push(mapping);
				}
			}


		}
		
		public function show():Object
		{

			var mappingsForEachHandler:Object = {};
			for each(var mapping:NtgMapping in _mappings) {
				var mappings:Object = (mappingsForEachHandler[mapping.handlerName] ||= {});
				mappings[String(mapping.gestureInput.show())] = mapping.show();
			}

			var handlers:Object = [];			
			for each(var handler:NtgHandler in _handlers) {
				handlers[handler.name] = handler.show();
			}
			
			return {
				 isMouseGestureEnabled: _isMouseGestureEnabled,
				isLockerGestureEnabled: _isLockerGestureEnabled,
				 isWheelGestureEnabled: _isWheelGestureEnabled,
				        useLLMouseHook: _useLLMouseHook,
				replaceDLLByJavaScript: _replaceDLLByJavaScript,
				 processModuleFileName: _processModuleFileName,
				        useShortcutKey: _useShortcutKey,
				              handlers: handlers,
				              mappings: mappings
			};
		}
	}
}
