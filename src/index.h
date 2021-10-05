const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>IoT Catbox Server</h2>
  <h3>By: Jameson Beebe</h3>
  <p>
    <i class="fas fa-poo" style="color:#7b5c00;"></i> 
    <span class="dht-labels">Weight</span>
    <span id="weight">%WEIGHT%</span>
  </p>
  <p>
    <i class="fas fa-0" style="color:#7b5c00;"></i> 
    <button type="button" onclick="zeroScale();">Zero Scale</button>
  </p>
  <p>
    <i class="fas fa-scale-balanced" style="color:#7b5c00;"></i> 
    <button type="button" onclick="zeroScale();">Reset Scale</button>
  </p>

  <p>
    <i class="fas fa-power-off" style="color:#7b5c00;"></i> 
    <button type="button" onclick="restartEsp();">Restart</button>
  </p>
<iframe width="450" height="250" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/1357181/charts/1?&days=10&dynamic=true&results=10000&api_key=EAL7YI5W5NF0Q8SQ"></iframe>
<iframe width="450" height="250" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/1357181/charts/2?&days=10&dynamic=true&results=10000&api_key=EAL7YI5W5NF0Q8SQ"></iframe>
<iframe width="450" height="250" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/1357181/charts/7?&days=10&dynamic=true&results=10000&api_key=EAL7YI5W5NF0Q8SQ"></iframe>


</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("weight").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/weight", true);
  xhttp.send();
}, 2000);

function zeroScale() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/zero", true);
  xhttp.send();
}

function restartScale() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/resetscale", true);
  xhttp.send();
}

function restartEsp() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/reset", true);
  xhttp.send();
}

</script>
</html>)rawliteral";