//	The bridge to Kotlin.  It is the same shape as Bridge/BH.mm: every call hands
//	back the JSON described in C++/Embed.hpp, so a SliP error arrives as data
//	rather than as a C++ exception — which matters here, because an exception
//	thrown across this boundary has nowhere to go but std::terminate.
//
//	Strings cross as byte arrays, not as jstring.  JNI's NewStringUTF and
//	GetStringUTFChars speak modified UTF-8, which encodes anything outside the
//	BMP as a surrogate pair in six bytes rather than as the four bytes of real
//	UTF-8.  SliP has such a character in its alphabet — 𝑒 is U+1D452 — so a
//	jstring round trip would corrupt the one constant most likely to be typed
//	after π.  Bytes in, bytes out, and Kotlin decodes them as UTF-8.

#include <jni.h>
#include <string>

#include "Embed.hpp"

//	Build() is latched on first use.  The CLI, the tests and the WASM module each
//	call it once from their single entry point; a host with a UI has no such
//	point, since Kotlin may call any of these first and may call them again.  A
//	file-scope initializer would be a static-init-order race against BUILTINS in
//	SliP.cpp.
static void
BuildOnce() {
	extern void Build();
	static auto
	$ = ( Build(), true );
	(void)$;
}

static std::string
Bytes( JNIEnv* env, jbyteArray _ ) {
	auto n = env->GetArrayLength( _ );
	std::string $( (size_t)n, '\0' );
	env->GetByteArrayRegion( _, 0, n, reinterpret_cast< jbyte* >( &$[ 0 ] ) );
	return $;
}

static jbyteArray
Bytes( JNIEnv* env, std::string const& _ ) {
	auto $ = env->NewByteArray( (jsize)_.size() );
	env->SetByteArrayRegion( $, 0, (jsize)_.size(),
	                         reinterpret_cast< const jbyte* >( _.data() ) );
	return $;
}

#define	SLIP_JNI( name )	Java_tokyo_sat_slip_SliP_00024Companion_##name

extern "C" {

JNIEXPORT jbyteArray JNICALL
SLIP_JNI( nativeVersion )( JNIEnv* env, jobject ) {
	return Bytes( env, Version() );
}

JNIEXPORT void JNICALL
SLIP_JNI( nativeSetRoundPrecision )( JNIEnv*, jobject, jint _ ) {
	SetRoundPrecision( _ );
}

JNIEXPORT jlong JNICALL
SLIP_JNI( nativeNewSession )( JNIEnv*, jobject ) {
	BuildOnce();
	return reinterpret_cast< jlong >( NewEmbedSession() );
}

JNIEXPORT void JNICALL
SLIP_JNI( nativeDeleteSession )( JNIEnv*, jobject, jlong handle ) {
	DeleteEmbedSession( reinterpret_cast< EmbedSession* >( handle ) );
}

JNIEXPORT void JNICALL
SLIP_JNI( nativeResetSession )( JNIEnv*, jobject, jlong handle ) {
	ResetEmbedSession( reinterpret_cast< EmbedSession* >( handle ) );
}

//	Calculator mode: one entry per non-empty line, and it does not stop at a
//	failure — a typo on the first line must not hide the answer on the sixth.
JNIEXPORT jbyteArray JNICALL
SLIP_JNI( nativeSugared )( JNIEnv* env, jobject, jlong handle, jbyteArray source ) {
	return Bytes( env, SessionSugared( reinterpret_cast< EmbedSession* >( handle ),
	                                   Bytes( env, source ) ) );
}

//	Programming mode: one entry per toplevel form, stopping at the first failure,
//	because a later form usually depends on an earlier one.
JNIEXPORT jbyteArray JNICALL
SLIP_JNI( nativeRepl )( JNIEnv* env, jobject, jlong handle, jbyteArray source ) {
	return Bytes( env, SessionREPL( reinterpret_cast< EmbedSession* >( handle ),
	                                Bytes( env, source ) ) );
}

}
