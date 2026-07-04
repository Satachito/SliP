#include "../C++/SliP.hpp"

#include <emscripten/val.h>
using namespace emscripten;

struct
Val	: SliP {
	val	$;
	Val( val $ ) : $( $ ) {}
	string
	REPR() const {
		return "emscripten::val";
	}
};

auto
BuildJS() {
	Register< Unary >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
cerr << "canvas: " << _->REPR() << endl;
			val	options = val::object();
			options.set("width", 800);
			options.set("height", 600);
			options.set("left", 100);
			options.set("top", 50);
			options.set("context", "webgl");
			return MS< Val >( 
				val::global( "window" ).call< val >(
					"createCanvas"
				,	options
				)
			);
		}
	,	"canvas"
	);
}
