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
        app.launch()

        // Insert steps here to perform after app launch but before taking a screenshot,
        // such as logging into a test account or navigating somewhere in the app

        let attachment = XCTAttachment(screenshot: app.screenshot())
        attachment.name = "Launch Screen"
        attachment.lifetime = .keepAlways
        add(attachment)
    }
}
