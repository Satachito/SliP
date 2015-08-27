$(function() {

  // eval
  $(".run-button").on("click", function() {
    $.post("/eval", {code: $(".code").val()}, function(res) {
      $(".code-result").empty()
//      if (res.err) {
//        $(".code-result").append(res.err.replace(/\n/g, "<br/>"))
//      } else {
        $(".code-result").append(res.out.replace(/\n/g, "<br/>"))
//      }
    }, "json")
  })

  // input soft tab
  $(".code").on("focus", function() {
    window.document.onkeydown = function(e) {
      if(e.keyCode === 9) {   // 9 = Tab
        $(".code").val($(".code").val() + "  ")
        e.preventDefault()
      }
    }
  }).on("blur", function() {
    window.document.onkeydown = function(e) {
      return true
    }
  })

})
