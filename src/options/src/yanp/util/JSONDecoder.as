package yanp.util
{

	public final class JSONDecoder
	{
		
		private var _JSON:String;
		private var _cursor:int = 0;
		
		public function JSONDecoder(JSON:String)
		{
			_JSON = JSON;
		}
		
		public static function decode(JSON:String):*
		{
			return new JSONDecoder(JSON)._decodeNextValue();
		}

		private function _decodeNextValue():*
		{
			if(_matchToProgress(/\G\{/g)) {
				var objValue:Object = {};
				_decodeNextListBody(function ():void
				{
					var key:String = _decodeNextString();
					if(!_matchToProgress(/\G:/g)) {
						_raise("`:' がありません。");
					}
					objValue[key] = _decodeNextValue();
				}, /\G\}/g);
				return objValue;
			}
			if(_matchToProgress(/\G\[/g)) {
				var arrayValue:Array = [];
				_decodeNextListBody(function ():void
				{
					arrayValue.push(_decodeNextValue());
				}, /\G]/g);
				return arrayValue;
			}
			var result:Object = _matchToProgress(/\G([^\W\d]\w*)/g);
			if(result) {
				switch(result[1]) {
				case      "true": return true;
				case     "false": return false;
				case "undefined": return undefined;
				case      "null": return null;
				}
				_raise("不明なID `"+ result[1] +"' があります。");
			}
			result = _matchToProgress(/\G-?(?:0|[1-9]\d*)(\.\d*)?([Ee][\-+]?\d+)?/g);
			if(result) {
				var hasFractionalPart:Boolean = result.hasOwnProperty("1") || result.hasOwnProperty("2");
				return hasFractionalPart ? parseFloat(result[0]) : parseInt(result[0]) ;
			}
			return _decodeNextString();
		}
		
		private function _decodeNextListBody(elementHandler:Function, terminator:RegExp):void
		{
			
			for(var isFirstElement:Boolean = true
				; !_matchToProgress(terminator); isFirstElement = false)
			{
				if(!(isFirstElement || _matchToProgress(/\G,/g))) {
					_raise("`,' がありません。");
				}	
				elementHandler();
			}
		}
		
		private function _decodeNextString():String
		{
			if(!_matchToProgress(/\G"/g)) {
				_raise("文字列がありません。");
			}
			var stringValue:String = "";
			for(;;) {
				var result:Object
					= _matchToProgress(/\G(.*?)("|\\(u[A-Fa-f\d]{4}|[bfnrt\\\/"]))/g, false);
				if(!result) {
					_raise("文字列が `\"' で終了していません。");
				}
				if(result[1].length != 0) {
					stringValue += result[1];
				}
				if(result[2] == '"') {
					break;
				}
				stringValue += _unescape(result[3]);
			}
			return stringValue;
		}
		
		private function _unescape(escapeSequence:String):String
		{
			switch(escapeSequence.charAt(0)) {
			case  "b": return "\b";
			case  "f": return "\f";
			case  "n": return "\n";
			case  "r": return "\r";
			case  "t": return "\t";
			case "\\": return "\\";
			case  "/": return  "/";
			case  '"': return  '"';
			case  "u":
				return String.fromCharCode(parseInt(escapeSequence.substr(1), 16));
			}
			return null;
		}

		private function _matchToProgress(pattern:RegExp, skipWhitespace:Boolean=true):Object
		{
			if(skipWhitespace) {
				_matchToProgress(/\s*/g, false);
			}
			pattern.lastIndex = _cursor;
			var result:Object = pattern.exec(_JSON)
			if(result) {
				_cursor = pattern.lastIndex;
			}
			return result;
		}
		
		private function _raise(message:String):void
		{
			throw new Error("JSONDecoder._raise: "+ message +" [index="+ _cursor +", rest="+ _JSON.substr(_cursor) +"]");
		}
	}
}

