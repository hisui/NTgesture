package ntg.settingModelClasses
{

	import ntg.NtgISettingModel;
	
	public class NtgAction implements NtgISettingModel
	{
		
		private var _id:String;
		
		private var _vars:Object;
		
		public function NtgAction()
		{
		}

		public function get id():String
		{
			return _id;
		}
		public function set id(id:String):void
		{
			_id = id;
		}

		public function get vars():Object
		{
			return _vars;
		}
		public function set vars(vars:Object):void
		{
			_vars = vars;
		}
		
		public function read(input:Object):void
		{
			_id   = input.id;
			_vars = input.vars;
		}
		
		public function show():Object
		{
			return {
				  id: _id,
				vars: _vars
			};
		}
	}
}