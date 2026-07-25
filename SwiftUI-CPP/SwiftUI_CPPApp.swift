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
			CommandGroup( after: .newItem ) {
				Button( "Run" ) { NotificationCenter.default.post( name: .slipRun, object: nil ) }
					.keyboardShortcut( .return, modifiers: .command )
			}
		}

		Settings { SettingsView() }
	}
}

extension Notification.Name {
	static let slipRun = Notification.Name( "slipRun" )
}
