package ntg.settingModelClasses
{

	public class NtgWheelGestureKey extends NtgGestureKey
	{
		
		private var _isUp:Boolean;

		public function NtgWheelGestureKey()
		{
			super();
		}
		
		public override function get gestureSummary():String
		{
			return "wheel:"+ (_isUp ? "上" : "下");
		}
		
		public function get isUp():Boolean
		{
			return _isUp;
		}
		public function set isUp(isUp:Boolean):void
		{
			_isUp = isUp;
		}
		
		public override function read(input:Object):void
		{
			_isUp = String(input) == "U";
		}
		
		public override function show():Object
		{
			return _isUp ? "U" : "D" ;
		}
	}
}
