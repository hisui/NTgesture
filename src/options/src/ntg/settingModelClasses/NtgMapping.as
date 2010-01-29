package ntg.settingModelClasses
{

	import ntg.NtgISettingModel;

	public final class NtgMapping implements NtgISettingModel
	{
		
		public static const  kTypeMouseGesture:String =  "mouse";
		public static const kTypeRockerGesture:String = "rocker";
		public static const  kTypeWheelGesture:String =  "wheel";
		
		private var _handlerName:String;
		
		private var _gestureType:String;
		
		private var _gestureKey:NtgGestureKey;
		
		private var _actions:Array = [];
		
		private var _priority:int;
		

		public function NtgMapping()
		{
		}

		public function get handlerName():String
		{
			return _handlerName;
		}
		public function set handlerName(handlerName:String):void
		{
			_handlerName = handlerName;
		}
		
		public function get gestureType():String
		{
			return _gestureType;
		}
		public function set gestureType(gestureType:String):void
		{
			_gestureType = gestureType;
		}

		public function get gestureKey():NtgGestureKey
		{
			return _gestureKey;
		}
		public function set gestureKey(gestureKey:NtgGestureKey):void
		{
			_gestureKey = gestureKey;
		}
		
		public function get actions():Array
		{
			return _actions;
		}
		public function set actions(actions:Array):void
		{
			_actions = actions;
		}
		
		public function get priority():int
		{
			return _priority;
		}
		public function set priority(priority:int):void
		{
			_priority = priority;
		}
		
		public function get gestureSummary():String
		{
			return _gestureKey.gestureSummary;
		}
		
		public function get actionSummary():String
		{
			return _actions.map(function (action:NtgAction, ..._:*):String { return action.id }).join(", ");
		}

		public function read(input:Object):void
		{
			_gestureType = input.type;
			_actions = input.actions.map(function (input:Object, ..._:*):NtgAction
			{
					var action:NtgAction = new NtgAction();
					action.read(input);
					return action;
			});
			_priority = input.prio;
		}
		
		public function show():Object
		{
			return {
				   type: _gestureType,
				actions: _actions.map(function (action:NtgAction, ..._:*):Object { return action.show(); }),
				   prio: _priority
			};
		}
	}
}
