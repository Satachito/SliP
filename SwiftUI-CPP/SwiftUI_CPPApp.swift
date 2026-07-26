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

	var body: some Scene {
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
	}
}

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
	@Environment( \.openWindow ) private var openWindow

	var body: some Commands {
		CommandGroup( after: .newItem ) {
			Button( "Run" ) { run?() }
				.keyboardShortcut( .return, modifiers: .command )
				.disabled( run == nil )
		}

		CommandGroup( replacing: .help ) {
			Button( "SliP Help" ) { openWindow( id: "help" ) }
				.keyboardShortcut( "?", modifiers: .command )
			Button( "Privacy Policy" ) { openWindow( id: "privacy" ) }
		}
	}
}
