package ntg.settingModelClasses
{

	public class NtgMouseGestureKey extends NtgGestureKey
	{
		
		public static const kArrowEast:String  = "E";
		public static const kArrowWest:String  = "W";
		public static const kArrowNorth:String = "N";
		public static const kArrowSouth:String = "S";
		
		public static const kMaxArrows:int = 15;

		private var _arrows:String;

		public function NtgMouseGestureKey()
		{
			super();
		}
		
		public function get arrows():String
		{
			return _arrows;
		}
		public function set arrows(arrows:String):void
		{
			_arrows = arrows;
		}
		
		public override function get gestureSummary():String
		{
			return "mouse:"+ asArrowNotation();
		}
		
		public function asArrowNotation():String
		{
			return _arrows
				.replace(/E/g, "→")
				.replace(/W/g, "←")
				.replace(/N/g, "↑")
				.replace(/S/g, "↓")
				;
		}
		
		public override function read(input:Object):void
		{
			_arrows = String(input);
		}
		
		public override function show():Object
		{
			return _arrows;
		}
		
	}
}
