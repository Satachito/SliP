window.createCanvas = _ => {
	const $ = document.body.appendChild( document.createElement( 'canvas' ) )
	$.width = _.width
	$.height = _.height

	if (_.left) $.style.left = _.left + 'px'
	if (_.top)  $.style.top  = _.top + 'px'

	$.setAttribute( 'title', 'Double click to close' )

	$.onmousedown = md => { 
		const org = $.getBoundingClientRect()
		$.onmousemove = mm => {
			$.style.left = org.left + mm.clientX - md.clientX + 'px'
			$.style.top  = org.top  + mm.clientY - md.clientY + 'px'
		}
		$.onmouseup = $.onmouseleave = () => $.onmousemove = $.onmouseup = null
	}

	$.onclick = ev => ev.detail > 1 && $.parentNode.removeChild( $ )

	return _.context === 'webgl'
	?	$.getContext('webgl', { preserveDrawingBuffer: true })
	:	$.getContext('2d')
}

