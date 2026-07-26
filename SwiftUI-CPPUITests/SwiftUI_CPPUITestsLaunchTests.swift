//
//  SwiftUI_CPPUITestsLaunchTests.swift
//  SwiftUI-CPPUITests
//
//  Created by Satoru Ogura on 2025/06/15.
//  Copyright © 2025 Satoru Ogura. All rights reserved.
//

import XCTest

final class SwiftUI_CPPUITestsLaunchTests: XCTestCase {

    override class var runsForEachTargetApplicationUIConfiguration: Bool {
        true
    }

    override func setUpWithError() throws {
        continueAfterFailure = false
    }

    @MainActor
    func testLaunch() throws {
        let app = XCUIApplication()
        #if os(iOS)
        app.launchArguments.append( "--ui-testing" )
        #endif
        app.launch()

        #if os(macOS)
        app.typeKey("n", modifierFlags: .command)
        let window = app.windows.firstMatch
        XCTAssertTrue(window.waitForExistence(timeout: 5))

        app.typeKey(.return, modifierFlags: .command)

        let attachment = XCTAttachment(screenshot: window.screenshot())
        #else
        let run = app.buttons.matching(
            NSPredicate(format: "label IN %@", [ "Run", "実行" ])
        ).firstMatch
        XCTAssertTrue(run.waitForExistence(timeout: 5))
        run.tap()

        let attachment = XCTAttachment(screenshot: app.screenshot())
        #endif
        attachment.name = "SliP Calculator"
        attachment.lifetime = .keepAlways
        add(attachment)
    }
}
