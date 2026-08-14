import java.util.Properties

//	Release signing is read from Android/keystore.properties, which is not
//	committed and does not exist here: it names a keystore and the passwords for
//	it, and those are yours to create.  See README.md for the keytool line.
//	Without that file the release variant still builds — unsigned.
val keystore = Properties().apply {
	val f = rootProject.file( "keystore.properties" )
	if ( f.exists() ) f.inputStream().use { load( it ) }
}
val signingReady = keystore.getProperty( "storeFile" ) != null

plugins {
	alias( libs.plugins.android.application )
	alias( libs.plugins.kotlin.android )
	alias( libs.plugins.kotlin.compose )
}

android {
	namespace  = "tokyo.sat.slip"
	compileSdk = 36
	ndkVersion = "29.0.14206865"

	defaultConfig {
		//	sat.tokyo reversed.  The other apps identify as tokyo.828.SliP, which
		//	an Android package name cannot be: every segment has to start with a
		//	letter, and "828" does not.  Play ties an app to this string for good,
		//	so it is worth being sure before the first upload.
		applicationId = "tokyo.sat.slip"
		minSdk        = 26
		targetSdk     = 36
		versionCode   = 1
		versionName   = "2.1.1"		//	tracks SLIP_VERSION in C++/SliP.hpp

		externalNativeBuild {
			cmake {
				//	Exceptions and RTTI are not optional: every error in the
				//	language is a throw, and every type test is a
				//	dynamic_pointer_cast.  The NDK enables both by default, and
				//	they are named here so that a default change cannot quietly
				//	break the build.
				arguments += listOf( "-DANDROID_STL=c++_shared" )
				cppFlags  += listOf( "-std=gnu++2b", "-fexceptions", "-frtti" )
			}
		}
		ndk {
			//	arm64 is every current phone and the Apple-silicon emulator;
			//	x86_64 is the emulator on Intel machines.
			abiFilters += listOf( "arm64-v8a", "x86_64" )
		}
	}

	externalNativeBuild {
		cmake {
			path    = file( "src/main/cpp/CMakeLists.txt" )
			version = "3.31.6"
		}
	}

	signingConfigs {
		create( "release" ) {
			if ( signingReady ) {
				storeFile     = rootProject.file( keystore.getProperty( "storeFile" ) )
				storePassword = keystore.getProperty( "storePassword" )
				keyAlias      = keystore.getProperty( "keyAlias" )
				keyPassword   = keystore.getProperty( "keyPassword" )
			}
		}
	}

	buildTypes {
		release {
			//	R8 is on, so the JNI entry points have to be kept by name —
			//	proguard-rules.pro does that.  Verified by running a release
			//	build on a device, not only by watching it compile.
			isMinifyEnabled   = true
			isShrinkResources = true
			proguardFiles( getDefaultProguardFile( "proguard-android-optimize.txt" ), "proguard-rules.pro" )
			if ( signingReady ) signingConfig = signingConfigs.getByName( "release" )
		}
	}

	compileOptions {
		sourceCompatibility = JavaVersion.VERSION_17
		targetCompatibility = JavaVersion.VERSION_17
	}
	kotlinOptions { jvmTarget = "17" }
	buildFeatures { compose = true }

	sourceSets[ "main" ].java.srcDirs( "src/main/kotlin" )
}

dependencies {
	implementation( platform( libs.androidx.compose.bom ) )
	implementation( libs.androidx.activity.compose )
	implementation( libs.androidx.compose.material3 )
	implementation( libs.androidx.compose.ui )
	implementation( libs.androidx.compose.ui.tooling.preview )
}
