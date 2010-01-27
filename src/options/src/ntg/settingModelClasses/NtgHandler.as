package ntg.settingModelClasses
{

	import ntg.NtgISettingModel;
	
	import yanp.util.JSONEncoder;

	public class NtgHandler implements NtgISettingModel
	{

		private var _name:String;

		private var _handlerNames:Array = [];
		
		private var _type:String;
		
		private var _vars:Object;


		public function NtgHandler()
		{
		}
		

		public function get name():String
		{
			return _name;
		}
		public function set name(name:String):void
		{
			_name = name;
		}

		public function get handlerNames():Array
		{
			return _handlerNames;
		}
		public function set handlerNames(handlerNames:Array):void
		{
			_handlerNames = handlerNames;
		}

		public function get type():String
		{
			return _type;
		}
		public function set type(type:String):void
		{
			_type = type;
		}

		public function get vars():Object
		{
			return _vars;
		}
		public function set vars(vars:Object):void
		{
			_vars = vars;
		}
		
		public function get summary():String
		{
			var summary:Object = {type: _type};
			if(_vars) {
				summary.vars = _vars;
			}
			if(_handlerNames && _handlerNames.length != 0) {
				summary["extends"] = _handlerNames;
			}
			return JSONEncoder.encode(summary, false, true);
		}
		
		public function read(input:Object):void
		{
			_handlerNames = input.handlerNames;
			_type = input.type;
			_vars = input.vars;
		}
		
		public function show():Object
		{
			return {
				handlerNames: _handlerNames,
				        type: _type,
				        vars: _vars
			};
		}
		
	}
}
