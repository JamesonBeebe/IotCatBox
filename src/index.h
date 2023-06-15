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
    table {font-size: 3.0rem;}
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
	<table id="myTable" border="1">
		<tr>
			<th>Litter Weight</th>
			<th id="litterWeight">%LITTERWEIGHT%</th>
		</tr>
		<tr>
			<th>Whiskey Uses</th>
			<th id="cat1uses">%CAT1USES%</th>
		</tr>
		<tr>
			<th>Riggins Uses</th>
			<th id="cat2uses">%CAT2USES%</th>
		</tr>
    <tr>
			<th>State</th>
			<th id="state">%STATE%</th>
		</tr>
	</table>
  </p>
  <p>
    <i class="fas fa-0" style="color:#7b5c00;"></i> 
    <button type="button" onclick="updateWeight();">Update Weight</button>
  </p>
  <p>
    <i class="fas fa-0" style="color:#7b5c00;"></i> 
    <button type="button" onclick="cat1uses();">Whiskey Uses Update</button>
  </p>
  <p>
    <i class="fas fa-scale-balanced" style="color:#7b5c00;"></i> 
    <button type="button" onclick="zeroScale();">Reset Scale</button>
  </p>

  <p>
    <i class="fas fa-power-off" style="color:#7b5c00;"></i> 
    <button type="button" onclick="restartEsp();">Restart</button>
  </p>
<iframe width="450" height="250" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/1357181/charts/1?&days=10&dynamic=true&results=100000&api_key=EAL7YI5W5NF0Q8SQ"></iframe>
<iframe width="450" height="250" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/1357181/charts/2?&days=10&dynamic=true&results=100000&api_key=EAL7YI5W5NF0Q8SQ"></iframe>
<iframe width="450" height="250" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/1357181/charts/7?&days=10&dynamic=true&results=100000&api_key=EAL7YI5W5NF0Q8SQ"></iframe>
<iframe width="450" height="250" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/1357181/charts/4?&days=10&dynamic=true&results=100000&api_key=EAL7YI5W5NF0Q8SQ"></iframe>
</body>

<script>
// Define the variables
var variable1 = "Hello";
var variable2 = 42;
var variable3 = true;
var variable4 = null;

function updateWeight() {
  var xhttp = new XMLHttpRequest();
  if (this.readyState == 4 && this.status == 200) {
    document.getElementById("weight").innerHTML = this.responseText;
  }
  xhttp.open("GET", "/updateWeight", true);
  xhttp.send();
}

function zeroScale() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/zero", true);
  xhttp.send();
}

function litterWeight() {
  var xhttp = new XMLHttpRequest();
  if (this.readyState == 4 && this.status == 200) {
    document.getElementById("litterWeight").innerHTML = this.responseText;
  }
  xhttp.open("GET", "/litterWeight", true);
  xhttp.send();
}

function cat1uses() {
  var xhttp = new XMLHttpRequest();
  if (this.readyState == 4 && this.status == 200) {
    document.getElementById("cat1uses").innerHTML = this.responseText;
  }
  xhttp.open("GET", "/whiskeyUses", true);
  xhttp.send();
}

function cat2uses() {
  var xhttp = new XMLHttpRequest();
  if (this.readyState == 4 && this.status == 200) {
    document.getElementById("cat2uses").innerHTML = this.responseText;
  }
  xhttp.open("GET", "/rigginsUses", true);
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

function updateValues() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var response = JSON.parse(this.responseText);
      document.getElementById("weight").innerHTML = response.weight;
      document.getElementById("state").innerHTML = response.state;
      document.getElementById("cat1uses").innerHTML = response.cat1uses;  
      document.getElementById("cat2uses").innerHTML = response.cat2uses;
      document.getElementById("litterWeight").innerHTML = response.litterWeight;  
    }
  };
  xhttp.open("GET", "/getValues", true);
  xhttp.send();
}

setInterval(function ( ) {
  updateValues();
}, 2000);

//document.getElementById("myTable").rows[0].cells[1].innerHTML = variable1;
//document.getElementById("myTable").rows[1].cells[1].innerHTML = variable2;
//document.getElementById("myTable").rows[2].cells[1].innerHTML = variable3;
//document.getElementById("myTable").rows[2].cells[1].innerHTML = variable4;

</script>
</html>)rawliteral";