package tokyo.sat.slip

import org.json.JSONArray

//	Kotlin's view of the engine, mirroring Bridge/SliPEngine.swift.  Everything
//	the bridge returns is the JSON described in C++/Embed.hpp; this file decodes
//	it, and nothing above it deals in byte arrays or handles.
//
//	Strings cross the boundary as UTF-8 bytes rather than as jstring — see the
//	note at the top of cpp/jni.cpp for why.

data class SliPResult(
	val source: String?,	//	the form as the reader understood it
	val value:  String?,	//	its printed value, when it evaluated
	val error:  String?,	//	why it did not, when it did not
) {
	val failed get() = error != null
}

enum class SliPMode( val title: String, val help: String ) {
	Calculator(
		"Calculator",
		"One expression per line: 2πr, cosπ, 'r = 2",
	),
	Programming(
		"Programming",
		"Toplevel SliP forms: ( 'fact = '… ), { … }, « … »",
	),
}

class SliP private constructor( private var handle: Long ) : AutoCloseable {

	companion object {
		init { System.loadLibrary( "slip" ) }

		private external fun nativeVersion(): ByteArray
		private external fun nativeSetRoundPrecision( precision: Int )
		private external fun nativeNewSession(): Long
		private external fun nativeDeleteSession( handle: Long )
		private external fun nativeResetSession( handle: Long )
		private external fun nativeSugared( handle: Long, source: ByteArray ): ByteArray
		private external fun nativeRepl( handle: Long, source: ByteArray ): ByteArray

		val version: String get() = String( nativeVersion(), Charsets.UTF_8 )

		fun setRoundPrecision( precision: Int ) = nativeSetRoundPrecision( precision )

		fun session() = SliP( nativeNewSession() )

		//	The engine promises parseable JSON.  If that promise is ever broken,
		//	say so plainly rather than showing an empty pane.
		internal fun decode( json: String ): List< SliPResult > = try {
			val array = JSONArray( json )
			( 0 until array.length() ).mapNotNull { i ->
				array.optJSONObject( i )?.let {
					SliPResult(
						source = it.optString( "source",   "" ).ifEmpty { null },
						value  = if ( it.has( "response" ) ) it.getString( "response" ) else null,
						error  = if ( it.has( "error"    ) ) it.getString( "error"    ) else null,
					)
				}
			}
		} catch ( e: Exception ) {
			listOf( SliPResult( null, null, "Engine returned unreadable output: $json" ) )
		}
	}

	fun run( source: String, mode: SliPMode ): List< SliPResult > {
		val bytes = source.toByteArray( Charsets.UTF_8 )
		val json  = when ( mode ) {
			SliPMode.Calculator  -> nativeSugared( handle, bytes )
			SliPMode.Programming -> nativeRepl( handle, bytes )
		}
		return decode( String( json, Charsets.UTF_8 ) )
	}

	fun reset() = nativeResetSession( handle )

	override fun close() {
		if ( handle != 0L ) {
			nativeDeleteSession( handle )
			handle = 0L
		}
	}
}
