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
          } else {
            content.style.display = "block";
          }
        });
  }
}
