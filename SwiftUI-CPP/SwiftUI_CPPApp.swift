//
//  SwiftUI_CPPApp.swift
//  SwiftUI-CPP
//
//  Created by Satoru Ogura on 2025/06/15.
//  Copyright © 2025 Satoru Ogura. All rights reserved.
//

import SwiftUI

@main
struct SwiftUI_CPPApp: App {
    var body: some Scene {
        DocumentGroup(newDocument: SwiftUI_CPPDocument()) { file in
			ContentView( document: file.$document, text: "" )
        }
    }
}
