package ntg.events
{

	import flash.events.Event;

	public class NtgDialogBoxEvent extends Event
	{
		
		public static const OK:String = "OK";
		
		private var _result:*;
		
		public function NtgDialogBoxEvent(type:String, result:*, bubbles:Boolean=false, cancelable:Boolean=false)
		{
			super(type, bubbles, cancelable);
			_result = result;
		}
		
		public function get result():*
		{
			return _result;
		}
		
	}
}
