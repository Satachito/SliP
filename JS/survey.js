
class _List {
}
console.log( typeof _List )			//	function

class List extends _List {
}
console.log( typeof List )			//	function

const _l = new _List()
console.log( _l instanceof _List )	//	true
console.log( _l instanceof List )	//	true
console.log( typeof _l )			//	object
console.log( _l.constructor.name )	//	object

const l = new List()
console.log( l instanceof _List )	//	true
console.log( l instanceof List )	//	true
console.log( typeof l )				//	object
console.log( l.constructor.name )	//	object


