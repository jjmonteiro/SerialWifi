/*****************************************************************************/
/*	Serial Debugger over Wifi - UP840126 - University of Portsmouth
/*****************************************************************************/

const char* Index_1 PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
  <title>Serial Wifi Debugger</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<style>

body {
    font-family: "Arial";
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
    width: 500px;
    height: 500px;
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
    height: 83%;
    margin: 5px 0;
    resize: none;
    background: white;
    box-sizing: border-box;
}

.textbox {
    width: 100%;
    min-height: 5%;
    margin: 5px 0;
    padding: 8px;
    font-family: "Arial";
    font-size: 14px;
    color: black;
    line-height: 1.3;
    text-decoration: none;
    box-sizing: border-box;
}

.center {text-align: center}
input:hover,input:focus {border: 2px solid grey}
</style>
</head>

<body>

<div class="flex-container">
<header>
  <h1>Serial Wifi Debugger</h1>
</header>

<!---LEFT PANEL---><nav class="nav"><p>Informations</p>
<p class="textbox border">
)=====";
