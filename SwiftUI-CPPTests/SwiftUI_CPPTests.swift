import Testing
@testable import SliP

struct SwiftUI_CPPTests {

	@Test @MainActor func sessionsAreIndependent() {
		let first = SliPSession()
		let second = SliPSession()

		let definition = first.run( "( 'storeTestValue = 41 )", mode: .programming )
		#expect( definition.first?.failed == false )

		let firstResult = first.run( "( storeTestValue + 1 )", mode: .programming )
		#expect( firstResult.first?.value == "42" )

		let secondResult = second.run( "( storeTestValue + 1 )", mode: .programming )
		#expect( secondResult.first?.failed == true )
	}

	@Test @MainActor func resetClearsOnlyItsSession() {
		let session = SliPSession()
		_ = session.run( "( 'storeResetValue = 7 )", mode: .programming )
		session.reset()

		let result = session.run( "( storeResetValue )", mode: .programming )
		#expect( result.first?.failed == true )
	}

}
