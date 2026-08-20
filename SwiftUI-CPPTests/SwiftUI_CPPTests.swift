import Testing
@testable import SliP

struct SwiftUI_CPPTests {

	@Test @MainActor func sessionsAreIndependent() {
		let first = SliPSession()
		let second = SliPSession()

		let definition = first.run( "( 'storeTestValue = 41 )", mode: .programming )
		#expect( definition.results.first?.failed == false )

		let firstResult = first.run( "( storeTestValue + 1 )", mode: .programming )
		#expect( firstResult.results.first?.value == "42" )

		let secondResult = second.run( "( storeTestValue + 1 )", mode: .programming )
		#expect( secondResult.results.first?.failed == true )
	}

	@Test @MainActor func resetClearsOnlyItsSession() {
		let session = SliPSession()
		_ = session.run( "( 'storeResetValue = 7 )", mode: .programming )
		session.reset()

		let result = session.run( "( storeResetValue )", mode: .programming )
		#expect( result.results.first?.failed == true )
	}

	@Test @MainActor func canvasCommandsAreReturnedWithTheRun() {
		let session = SliPSession()
		let run = session.run(
			"( 'c = { 80 60 }:canvas:fillStyle `red`:fillRect{ 10 20 30 40 } )",
			mode: .programming
		)

		#expect( run.results.first?.failed == false )
		#expect( run.canvases.count == 1 )
		#expect( run.canvases.first?.width == 80 )
		#expect( run.canvases.first?.height == 60 )
		#expect( run.canvases.first?.commands.first?.kind == "fill" )
		#expect( run.canvases.first?.commands.first?.color == "red" )
	}

}
