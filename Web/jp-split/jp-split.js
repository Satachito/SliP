customElements.define(
	'jp-split-h'
,	class extends HTMLElement {
		constructor() {
			super()
			if ( this.children.length != 3 ) throw 'this.children.length != 3'
			const _1 = this.children[ 0 ]
			const _G = this.children[ 1 ]
			const _2 = this.children[ 2 ]
			this.style.display = 'grid'
			this.style.gridTemplateColumns = this.getAttribute( 'grid-template' )
			_G.onmousedown = md => {
				const size1 = _1.offsetWidth
				const size2 = _2.offsetWidth
				this.onmousemove = mm => {
					const bias = mm.clientX - md.clientX
					if ( !bias ) return
					if ( bias < 0 ) { 
						const new1 = size1 < -bias ? 0 : size1 + bias
						this.style.gridTemplateColumns = `${new1}fr ${_G.offsetWidth}px ${size1 + size2 - new1}fr`
					} else {
						const new2 = size2 < +bias ? 0 : size2 - bias
						this.style.gridTemplateColumns = `${size1 + size2 - new2}fr ${_G.offsetWidth}px ${new2}fr`
					}
				}
				this.onmouseup = () => this.onmousemove = null
				this.onmouseleave = () => this.onmousemove = null
			}
		}
	}
)

customElements.define(
	'jp-split-v'
,	class extends HTMLElement {
		constructor() {
			super()
			if ( this.children.length != 3 ) throw 'this.children.length != 3'
			const _1 = this.children[ 0 ]
			const _G = this.children[ 1 ]
			const _2 = this.children[ 2 ]
			this.style.display = 'grid'
			this.style.gridTemplateRows = this.getAttribute( 'grid-template' )
			_G.onmousedown = md => {
				const size1 = _1.offsetHeight
				const size2 = _2.offsetHeight
				this.onmousemove = mm => {
					const bias = mm.clientY - md.clientY
					if ( !bias ) return
					if ( bias < 0 ) { 
						const new1 = size1 < -bias ? 0 : size1 + bias
						this.style.gridTemplateRows = `${new1}fr ${_G.offsetHeight}px ${size1 + size2 - new1}fr`
					} else {
						const new2 = size2 < +bias ? 0 : size2 - bias
						this.style.gridTemplateRows = `${size1 + size2 - new2}fr ${_G.offsetHeight}px ${new2}fr`
					}
				}
				this.onmouseup = () => this.onmousemove = null
				this.onmouseleave = () => this.onmousemove = null
			}
		}
	}
)

