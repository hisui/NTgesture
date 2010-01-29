package ntg.settingModelClasses
{

	public class NtgRockerGestureKey extends NtgGestureKey
	{
		
		private var _isLeftToRight:Boolean;

		public function NtgRockerGestureKey()
		{
			super();
		}
		
		public override function get gestureSummary():String
		{
			return "rocker:"+ (_isLeftToRight ? "左→右" : "右→左");
		}
		
		public function get isLeftToRight():Boolean
		{
			return _isLeftToRight;
		}
		public function set isLeftToRight(isLeftToRight:Boolean):void
		{
			_isLeftToRight = isLeftToRight;
		}
		
		public override function read(input:Object):void
		{
			_isLeftToRight = String(input) == "R";
		}
		
		public override function show():Object
		{
			return _isLeftToRight ? "R" : "L" ;
		}
	}
}
