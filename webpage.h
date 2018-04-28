/*****************************************************************************/
/*  Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*****************************************************************************/

const char HTTP_WEBSITE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
  <title>Serial Wifi Debugger</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>

<style>

body {
    font-family: 'Arial';
    font-size: 16px;
    color: black;
    line-height: 1;
}

.flex-container {
    display: -webkit-flex;
    display: flex;
    -webkit-flex-flow: row wrap;
    flex-flow: row wrap;
    text-align: center;
}

.flex-container > * {
    padding: 15px;
    -webkit-flex: 1 100%;
    flex: 1 100%;
}

.nav {
    text-align: left;
    background:#eee;
    width: 150px;
    padding: 30px;
}

.article {
    text-align: left;
    background:#eee;
    padding: 30px;
}

header {background: grey;color:white;}
footer {background: grey;color:white;font-size: 10px;line-height: 2;}

@media all and (min-width: 768px) {
    .nav {text-align:left;-webkit-flex: 1 auto;flex:1 auto;-webkit-order:1;order:1;}
    .article {-webkit-flex:5 0px;flex:5 0px;-webkit-order:2;order:2;}
    footer {-webkit-order:3;order:3;}
}

.border{
    display: block;
    border: 2px solid #ccc;
    border-radius: 4px;
    outline:none;
}

textarea {
    width: 100%;
    height: 90%;
    margin: 5px 0;
    resize: none;
    background: white;
    box-sizing: border-box;
    min-height: 500px;
  overflow-y: scroll;
  overflow-x: hidden;
}

.textbox {
    width: 100%;
    min-height: 7%;
    margin: 5px 0;
    padding: 8px;
    font-family: 'Arial';
    font-size: 14px;
    color: black;
    line-height: 1.3;
    text-decoration: none;
    box-sizing: border-box;
}

.center {text-align: center}

input:hover {border: 2px solid grey}
input:active {border: 2px solid #ccc}
select:hover {border: 2px solid grey}
select:active {border: 2px solid #ccc}

.button {
    width: 30%;
    display: inline;
    margin: 1%;
}

.ver {
    font-size: 9px;
    color: #ccc;
}
</style>
</head>

<body onload='websocketget()'>

<div class='flex-container'>
<header>
  <h1>Serial Wifi Debugger</h1>
</header>

<!---LEFT PANEL---><nav class='nav'>

<p>Informations</p>

<ul class='textbox border' style='list-style-type:none'>
  <li>Network SSID: {{wifiSSID}}</li>
  <li>IP Address: {{ipAddress}}</li>
  <li id='bufferSize'>Buffer Size:</li>
  <li id='freeRam'>Free Memory:</li>
  <li id='powerSupply'>Battery:</li>
</ul>
<ul class="textbox border" style="list-style-type:none">
  <li id='status'>Status: Idle</li>
</ul>
<br>
<p>Configurations</p>

<form action='javascript:void(0)'>
<input class='textbox border' type='text' placeholder='Lookup Command' maxlength='20' id='text2' value='{{faultCommand}}'>
<br>
<p>Data Format</p> 
    
  <select class='textbox border' id='option'>
    <option value='option0' {{option0}}>115200</option>
    <option value='option1' {{option1}}>57600</option>
    <option value='option2' {{option2}}>38400</option>
    <option value='option3' {{option3}}>28800</option>
    <option value='option4' {{option4}}>19200</option>
    <option value='option5' {{option5}}>14400</option>
    <option value='option6' {{option6}}>9600</option>
    <option value='option7' {{option7}}>4800</option>
    <option value='option8' {{option8}}>2400</option>
    <option value='option9' {{option9}}>1200</option>
    <option value='option10' {{option10}}>600</option>
    <option value='option11' {{option11}}>300</option>
  </select>
  
  <br>
  <label>Ascii
    <input id='radio' type='radio' name='radio' value='radio0' {{radio0}}>
  </label>
  <label>Hex
    <input type='radio' name='radio' value='radio1' {{radio1}}>
  </label>
  <br>
  <br>
  <input class='textbox border button' type='submit' name='click0' value='Save' onclick="websocketsend(text2.value + ';' + option.value + ';' + radio.value);alert('Configurations Saved.')">
  <input class='textbox border button' type='button' name='click1' value='Restart' onclick='restart()' >
    <input class='textbox border button' type='button' name='click2' value='Clear' onclick="document.getElementById('dataBuffer').innerHTML = ''">
</form>
</nav>

<!---RIGHT PANEL--->
        
<article class='article'>
<p>Buffer Contents</p>
<textarea class='border' id='dataBuffer' name='dataBuffer' readonly>
</textarea>
</article>
<footer class='ver'>Copyright &copy; 2018 Joaquim Monteiro</footer>
</div> 

<script>

var websock;

function websocketget() {

var status = document.getElementById('status');
var textarea = document.getElementById('dataBuffer');

  if (window.WebSocket) {
    status.innerHTML = 'Status: Connecting..';
    
      if(websock) {
      websock.close();
      websock = null;
      }
      
    websock = new WebSocket('ws://{{ipAddress}}:81/');
    websock.onopen = function(evt) { status.innerHTML = 'Status: Connected.'; };
    websock.onclose = function(evt) { status.innerHTML = 'Status: Disconnected.'; };
    websock.onerror = function(evt) { status.innerHTML = 'Status: Error!'; };
    websock.onmessage = function(evt) {
        var data = evt.data;
        var dataparts = data.split(';');
        document.getElementById('bufferSize').innerHTML = "Buffer Size: " + dataparts[0] + " Kb";
        document.getElementById('freeRam').innerHTML = "Free Memory: " + dataparts[1] + " Kb";
        document.getElementById('powerSupply').innerHTML = "Battery: " + dataparts[2] + " V";
        if (dataparts[3].length){
      textarea.innerHTML += decode(dataparts[3]);
      textarea.scrollTop = textarea.scrollHeight;
      }
      };
  } else {
    alert("This browser does not support Websockets!");
  }
}

function decode(str){
var str, strLen, output, temp, i;
strLen = str.length;
output = "";
  for (i = 0; i < strLen; i++) {
  temp = str.charAt(i) + str.charAt(++i);
        if (document.getElementById("radio").checked || temp=="0a" || temp=="0d") {
        output += "&#x" + temp + ";"; 
        }else{
          output += temp + " ";      
      }
  }
return output;
}

function websocketsend(data) {

  if (!window.WebSocket)
        return;

    if (WebSocket.OPEN) {
        websock.send(data);
    } else {
        alert("The socket is not open.");
    }
  
}    

function restart() {
  if (confirm('Unsaved data will be lost! \nSure to restart device?')) {
    websocketsend('');
    setTimeout('location.reload()', 5000);
  }
}
</script>
</body>  
</html>
)=====";

