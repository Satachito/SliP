package tokyo.sat.slip

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.input.TextFieldValue
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

//	The same screen the other apps have — SwiftUI-CPP/ContentView.swift is the
//	original, and this follows it: a mode picker, a keep-session switch and Run
//	across the top, the editor and the symbol keypad above, the results below.

//	The operators are most of the language and none of them are on a keyboard.
private val SYMBOLS = listOf(
	"'", "@", "£", "¶", "¤", "∅",
	"×", "÷", "±", "·", "∈", "∋",
	"¿", "¬", "¡", "¦", "§", "∥",
	"⟨", "⟩", "«", "»", "𝑒", "π",
)

private const val SAMPLE = """// Select Calculator, then press Run.
1 + 2 × 3
cosπ
[1 2 3] + 10"""

class MainActivity : ComponentActivity() {
	override fun onCreate( savedInstanceState: Bundle? ) {
		super.onCreate( savedInstanceState )
		enableEdgeToEdge()
		setContent { MaterialTheme { Screen() } }
	}
}

@Composable
private fun Screen() {
	//	One interpreter session for the life of the screen, disposed with it.
	val session = remember { SliP.session() }
	DisposableEffect( Unit ) { onDispose { session.close() } }

	//	TextFieldValue is not saveable on its own — it carries a selection and a
	//	composition — so it needs its own Saver to survive a rotation.
	var text        by rememberSaveable( stateSaver = TextFieldValue.Saver ) {
		mutableStateOf( TextFieldValue( SAMPLE ) )
	}
	var mode        by rememberSaveable { mutableStateOf( SliPMode.Calculator ) }
	var keepSession by rememberSaveable { mutableStateOf( false ) }
	var results     by remember { mutableStateOf( emptyList< SliPResult >() ) }

	fun run() {
		if ( !keepSession ) session.reset()
		results = session.run( text.text, mode )
	}

	Scaffold( modifier = Modifier.fillMaxSize() ) { insets ->
		Column( Modifier.padding( insets ).fillMaxSize() ) {

			Row(
				Modifier.fillMaxWidth().padding( horizontal = 8.dp, vertical = 4.dp ),
				verticalAlignment = Alignment.CenterVertically,
			) {
				SingleChoiceSegmentedButtonRow( Modifier.weight( 1f ) ) {
					SliPMode.entries.forEachIndexed { i, _mode ->
						SegmentedButton(
							selected = mode == _mode,
							onClick  = { mode = _mode },
							shape    = SegmentedButtonDefaults.itemShape( i, SliPMode.entries.size ),
						) { Text( _mode.title, maxLines = 1 ) }
					}
				}
				Spacer( Modifier.width( 8.dp ) )
				Switch(
					checked         = keepSession,
					onCheckedChange = { keepSession = it },
				)
				Spacer( Modifier.width( 4.dp ) )
				TextButton( onClick = { run() } ) { Text( "Run" ) }
			}
			HorizontalDivider()

			//	The split is carried by plain Boxes.  weight() on the TextField
			//	and on the results pane did not divide the space the way it
			//	reads — one of them ended up with almost all of it — because
			//	both of those measure their own content.  A Box takes its share
			//	and the child fills it.
			Box( Modifier.fillMaxWidth().weight( 1f ) ) {
				BasicMonospaceField(
					value    = text,
					onChange = { text = it },
					modifier = Modifier.fillMaxSize(),
				)
			}
			HorizontalDivider()

			Keypad { symbol ->
				val at = text.selection.end.coerceIn( 0, text.text.length )
				val next = text.text.substring( 0, at ) + symbol + text.text.substring( at )
				text = TextFieldValue(
					text      = next,
					selection = androidx.compose.ui.text.TextRange( at + symbol.length ),
				)
			}
			HorizontalDivider()

			Box( Modifier.fillMaxWidth().weight( 1.2f ) ) {
				Results( results, Modifier.fillMaxSize() )
			}
		}
	}
}

@Composable
private fun BasicMonospaceField(
	value: TextFieldValue,
	onChange: ( TextFieldValue ) -> Unit,
	modifier: Modifier = Modifier,
) {
	TextField(
		value         = value,
		onValueChange = onChange,
		modifier      = modifier,
		textStyle     = TextStyle( fontFamily = FontFamily.Monospace, fontSize = 15.sp ),
		colors        = TextFieldDefaults.colors(
			focusedIndicatorColor   = Color.Transparent,
			unfocusedIndicatorColor = Color.Transparent,
		),
	)
}

//	Rows rather than a grid with a fixed height: a height guessed in dp cut the
//	last row off, and the last row is where 𝑒 and π are.  This way the keypad is
//	as tall as its four rows and no taller.
@Composable
private fun Keypad( onSymbol: ( String ) -> Unit ) {
	Column( Modifier.fillMaxWidth().padding( vertical = 2.dp ) ) {
		SYMBOLS.chunked( 6 ).forEach { row ->
			Row( Modifier.fillMaxWidth() ) {
				row.forEach { symbol ->
					TextButton(
						onClick         = { onSymbol( symbol ) },
						modifier        = Modifier.weight( 1f ),
						contentPadding  = PaddingValues( 0.dp ),
					) {
						Text( symbol, fontFamily = FontFamily.Monospace, fontSize = 19.sp, maxLines = 1 )
					}
				}
			}
		}
	}
}

//	The modifier goes on the SelectionContainer, not on the LazyColumn inside it.
//	Column's weight() only applies to a direct child; handing it further down
//	compiles and then does nothing, which left this pane at zero height whenever
//	there was nothing in it yet.
@Composable
private fun Results( results: List< SliPResult >, modifier: Modifier = Modifier ) {
	SelectionContainer( modifier ) {
		LazyColumn( Modifier.fillMaxSize().padding( horizontal = 12.dp ) ) {
			items( results ) { result ->
				Column( Modifier.padding( vertical = 3.dp ) ) {
					result.source?.let {
						Text(
							it,
							fontFamily = FontFamily.Monospace,
							fontSize   = 12.sp,
							color      = MaterialTheme.colorScheme.onSurfaceVariant,
						)
					}
					Text(
						result.error ?: result.value ?: "",
						fontFamily = FontFamily.Monospace,
						fontSize   = 16.sp,
						color = if ( result.failed ) MaterialTheme.colorScheme.error
						        else MaterialTheme.colorScheme.onSurface,
					)
				}
			}
		}
	}
}
