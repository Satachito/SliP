//
//  ContentView.swift
//  SwiftUI-CPP
//
//  Created by Satoru Ogura on 2025/06/15.
//  Copyright © 2025 Satoru Ogura. All rights reserved.
//

import SwiftUI

struct ContentView: View {
    @Binding var document: SwiftUI_CPPDocument

    var body: some View {
        TextEditor(text: $document.text)
    }
}

#Preview {
    ContentView(document: .constant(SwiftUI_CPPDocument()))
}
