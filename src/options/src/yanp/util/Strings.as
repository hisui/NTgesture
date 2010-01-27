package yanp.util
{
	
	/**
	 * 文字列操作のユーティリティ
	 */
	public final class Strings
	{

		public static function multiply(part:String, count:uint):String
		{
			var buf:String = "";
			while(count) {
				if(count & 0x01) {
					buf += part;
					--count;
					continue;
				}
				part += part;
				count >>>= 1;
			}
			return buf;
		}

		public static function floatToString(value:Number, precision:int):String
		{
			var powerOf10:Number = Math.pow(10, precision);
			return String(Math.floor(value * powerOf10) / powerOf10);
		}

		public static function formatSecondsValue(sec:Number):String
		{
			var sec_:uint = sec | 0;
			var s:uint = sec_ % 60;
			var m:uint = sec_ / 60;
			return m +":"+ ("00"+ s).substr(-2, 2);
		}
		
		public static function trim(value:String):String
		{
			var result:Object = /\S(?:.*\S)?/.exec(value);
			return result ? result[0] : "" ;
		}
	}
}
