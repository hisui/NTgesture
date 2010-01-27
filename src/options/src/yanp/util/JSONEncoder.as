package yanp.util
{
	import flash.utils.Dictionary;
	
	public class JSONEncoder
	{

		/**
		 * オブジェクトを JSON に変換する
		 */
		public static function encode(
			rootNode:*, doPrettyPrint:Boolean=true, ignoreError:Boolean=false):String
		{

			const writer:_PrettyWriter = new _PrettyWriter;
			const cyclicRefMarker:Dictionary = new Dictionary;

			encodeJSONRecursively(rootNode);
			return doPrettyPrint ? writer.generateText() : writer.generateText(0, "") ;

			function encodeJSONRecursively(node:*):void
			{

				switch(typeof(node)) {

				case "undefined":
				case "function":
					if(!ignoreError) {
						throw new Error(typeof(node) +" は JSON に変換出来ません！");
					}
					writer.write( "null" );
					return;
	
				case "boolean":
				case "number":
					writer.write( node.toString() );
					return;
	
				case "string":
				case "xml":
					writer.write( toJSONString(node.toString()) );
					return;

				case "object":
					if(node == null) {
						writer.write( "null" );
						return;
					}
					if(cyclicRefMarker[node]) {
						if(!ignoreError) {
							throw new Error("循環参照が存在します");
						}
						writer.write( "null" );
						return;
					}
					cyclicRefMarker[node] = true;

					var separator:String = "";
					if(node is Array) {
						writer.write("[");
						writer.indent(function () :void
						{
							for each(var entry:* in node) {
								writer.write(separator);
								encodeJSONRecursively(entry);
								separator = "\n,";
							}
						});
						writer.write("]");
					}
					else {
						writer.write("{");
						writer.indent(function () :void
						{
							for(var name:String in node) {
								writer.write(separator + toJSONString(name) +":");
								encodeJSONRecursively(node[name]);
								separator = "\n,";
							}
						});
						writer.write("}");
					}
					delete cyclicRefMarker[node];
				}
			}
		}
			
		public static function toJSONString(value:String):String
		{
			var buf:String = '"';
			for(var i:int = 0, j:int = 0; ; ++i) {
				if(i >= value.length) {
					if(j < i) {
						buf += value.substring(j, i);
					}
					break;
				}
				var append:String = null;
				switch(value.charAt(i)) {
				case  '"': append = "\\\""; break;
				case '\\': append = "\\\\"; break;
				case  '/': append =  "\\/"; break;
				case '\b': append =  "\\b"; break;
				case '\f': append =  "\\f"; break;
				case '\n': append =  "\\n"; break;
				case '\r': append =  "\\r"; break;
				case '\t': append =  "\\t"; break;
				default:
					continue;
				}
				if(j < i) {
					buf += value.substring(j, i);
				}
				j = i + 1;
				buf += append;
			}
			return buf +'"';
		}

	}
}

import yanp.util.Strings;

class _PrettyWriter
{

	private var _lines:Array = [new _Line];
	private var _currentIndentLevel:int = 0;

	public function generateText(indentScale:int=2, lineSeparator:String="\n"):String
	{
		return _lines.map(function (line:_Line, ..._:*) :String
			{ return Strings.multiply(" ", line.indent * indentScale ) + line.value; }).join(lineSeparator);
	}

	public function write(text:String):void
	{
		var lines:Array = text.split("\n");
		if(lines.length != 0) {
			_lines[_lines.length - 1].value += lines.shift();
			for each(var value:String in lines) {
				var line:_Line = new _Line;
				line.value  = value;
				line.indent = _currentIndentLevel;
				_lines.push(line);
			}
		}
	}

	public function indent(callback:Function, additional:int=+1):void
	{
		_currentIndentLevel += additional;
		//try {
			callback();
		//} finally {
			_currentIndentLevel -= additional;
		//}
	}
}

class _Line
{
	public var value:String = "";
	public var indent:int;
}

