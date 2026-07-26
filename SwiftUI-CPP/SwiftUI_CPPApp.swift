//
//  SwiftUI_CPPApp.swift
//  SliP
//
//  Created by Satoru Ogura on 2025/06/15.
//  Copyright © 2025 Satoru Ogura. All rights reserved.
//

import SwiftUI

@main
struct SwiftUI_CPPApp: App {

	init() { Preference.apply() }

		@SceneBuilder
	var body: some Scene {
		#if os(iOS)
		#if UI_TESTING
		WindowGroup {
			ScreenshotContentView()
		}
		.commands {
			SliPCommands()
		}
		#else
		DocumentGroup( newDocument: SwiftUI_CPPDocument() ) { file in
			ContentView( document: file.$document )
		}
		.commands {
			SliPCommands()
		}
		#endif
		#else
		DocumentGroup( newDocument: SwiftUI_CPPDocument() ) { file in
			ContentView( document: file.$document )
		}
		.commands {
			SliPCommands()
		}

		Settings { SettingsView() }

		Window( "SliP Help", id: "help" ) { HelpView() }
			.defaultSize( width: 620, height: 620 )

		Window( "Privacy Policy", id: "privacy" ) { PrivacyPolicyView() }
			.defaultSize( width: 560, height: 520 )
		#endif
	}
}

#if os(iOS)
private struct ScreenshotContentView: View {
	@State private var document = SwiftUI_CPPDocument()

	var body: some View {
		ContentView( document: $document )
	}
}
#endif

private struct RunActionKey: FocusedValueKey {
	typealias Value = () -> Void
}

extension FocusedValues {
	var slipRunAction: (() -> Void)? {
		get { self[ RunActionKey.self ] }
		set { self[ RunActionKey.self ] = newValue }
	}
}

private struct SliPCommands: Commands {
	@FocusedValue( \.slipRunAction ) private var run
	#if os(macOS)
	@Environment( \.openWindow ) private var openWindow
	#endif

	var body: some Commands {
		CommandGroup( after: .newItem ) {
			Button( "Run" ) { run?() }
				.keyboardShortcut( .return, modifiers: .command )
				.disabled( run == nil )
		}

		#if os(macOS)
		CommandGroup( replacing: .help ) {
			Button( "SliP Help" ) { openWindow( id: "help" ) }
				.keyboardShortcut( "?", modifiers: .command )
			Button( "Privacy Policy" ) { openWindow( id: "privacy" ) }
		}
		#endif
	}
}
