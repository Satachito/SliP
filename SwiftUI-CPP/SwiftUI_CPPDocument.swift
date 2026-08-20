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

    //  Empty.  A new document is a blank sheet: the calculator writes its own
    //  history a line at a time, and a program nobody has written yet is not
    //  three examples of somebody else's.
    init(text: String = "") {
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
