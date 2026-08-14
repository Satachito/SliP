#	The JNI entry points are found by name from C++, so they must survive.
-keepclasseswithmembernames class tokyo.sat.slip.SliP$Companion {
	native <methods>;
}
