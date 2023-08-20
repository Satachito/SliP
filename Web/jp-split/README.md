[![Published on webcomponents.org](https://img.shields.io/badge/webcomponents.org-published-blue.svg)](https://www.webcomponents.org/element/@satachito/jp-split)

# jp-split

A small and understandable split view witch supports both horizontal and vertical splittings.

## Install

```
npm install @satachito/jp-split --save
```

## Components

`jp-split-h` for horizontal splitting and `jp-split-v` for vertical splitting.

## Demo

See
https://satachito.github.io/jp-split/

## HTML & CSS

Include `jp-split.js` at the very bottom of the `<body>` tag.
Supply `grid-template` attribute like `grid-template="1fr 4px 1fr"`.
For meaning, refer https://developer.mozilla.org/ja/docs/Web/CSS/grid-template-columns


```
---------------- BarH
<style>
#BarH {
;	height			: 200px
}
#BarH > div {
;	background		: grey
;	cursor			: col-resize
}
</style>
<jp-split-h id=BarH grid-template="1fr 4px 1fr">
	<textarea></textarea>
	<div></div>
	<textarea></textarea>
</jp-split-h>

<script>
Array.from( document.getElementsByTagName( 'textarea' ) ).forEach(
	$ => $.textContent = 'Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.'
)
</script>
<script src=node_modules/@satachito/jp-split/jp-split.js></script>
```

