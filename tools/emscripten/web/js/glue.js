function initPage() {
  // adding the listener to the source input
  document.getElementById("source").addEventListener(
      "keypress", (event) => {
        if (event.key === "Enter") {
          processCode();
        }
      });

  //adding listeners to the collapsible to fold
  var coll = document.getElementsByClassName("collapsible");
  for (var i = 0; i < coll.length; i++) {
    coll[i].addEventListener(
        "click", function() {
          this.classList.toggle("active");
          var content = this.nextElementSibling;
          if (content.style.display === "block") {
            content.style.display = "none";
            this.innerHTML="&#x25B6; AST";
          } else {
            content.style.display = "block";
            this.innerHTML="&#x25BC; AST";
          }
        });
  }
}
function openActionTab(evt, actionName) {
      var i, tabcontent, tablinks;
      tabcontent = document.getElementsByClassName("tabcontent");
      for (i = 0; i < tabcontent.length; i++) {
              tabcontent[i].style.display = "none";
            }
      tablinks = document.getElementsByClassName("tablinks");
      for (i = 0; i < tablinks.length; i++) {
              tablinks[i].className = tablinks[i].className.replace(" active", "");
            }
      document.getElementById(actionName).style.display = "block";
      evt.currentTarget.className += " active";
}
