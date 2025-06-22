//
//  SwiftUI_CPPDocument.swift
//  SwiftUI-CPP
//
//  Created by Satoru Ogura on 2025/06/15.
//  Copyright © 2025 Satoru Ogura. All rights reserved.
//

import SwiftUI
import UniformTypeIdentifiers

extension UTType {
    static var slip: UTType {
        UTType( importedAs: "tokyo.828.slip" )
    }
}

struct SwiftUI_CPPDocument: FileDocument {
    var text: String

    init(text: String = "[1 2 3]\n\"ABC\"") {
        self.text = text
    }

    static var readableContentTypes: [UTType] { [ .slip ] }

    init(configuration: ReadConfiguration) throws {
        guard let data = configuration.file.regularFileContents,
              let string = String(data: data, encoding: .utf8)
        else {
            throw CocoaError(.fileReadCorruptFile)
        }
        text = string
    }
    
    func fileWrapper(configuration: WriteConfiguration) throws -> FileWrapper {
        let data = text.data(using: .utf8)!
        return .init(regularFileWithContents: data)
    }
}
