~setvalue^Tank~
<!DOCTYPE html>
<html>
<head>
   <title>Motorhome - ~getvalue~</title>
   <link rel='stylesheet' href='/base.css' type='text/css'/>
   <link rel='stylesheet' href='/settings.css' type='text/css'/>
   <script src='/ajax.js' type='text/javascript'></script>
   <script src='/tank.js' type='text/javascript'></script>
   <meta name='viewport' content='width=device-width, initial-scale=1'>
   <link rel='icon'       href='/favicon.ico' type='image/x-icon'/>
</head>
<body>
<a class='tab-shortcut' href='#main-content'>Skip to content</a>
~expand^menu~
<h1 id='main-content'>Motorhome - ~getvalue~</h1>
<h2>Fresh water</h2>
~expand^value^Sensor mv^txt-fresh-mv~
~expand^value^Sensor supply mv^txt-fresh-supply-mv~
~expand^input^Rom^val-fresh-rom^10.0~
~expand^value^Temperature^txt-fresh-temperature~
~expand^input^Sensor base degrees^val-fresh-base-degrees^3.0~
~expand^input^Sensor base mv^val-fresh-base-mv^3.0~
~expand^input^Sensor mv per degree^val-fresh-mv-per-degree^3.0~
~expand^value^Sensor mv to subtract^txt-fresh-mv-compensation~
<br/>
~expand^value^Depth mm^txt-fresh-level~
~expand^input^Sensor position front mm^val-fresh-sensor-front^3.0~
~expand^input^Sensor position right mm^val-fresh-sensor-right^3.0~
~expand^value^Sensor mm to subtract^txt-fresh-mm-compensation~
<br/>
~expand^input^Tank width back-front mm^val-fresh-width^3.0~
~expand^input^Tank length left-right mm^val-fresh-length^3.0~
~expand^value^Volume litres^txt-fresh-volume~
<h2>Accelerometer</h2>
~expand^input^X raw flat^val-accelerometer-x-flat^3.0~
~expand^input^Y raw flat^val-accelerometer-y-flat^3.0~
~expand^input^Z raw flat^val-accelerometer-z-flat^3.0~
~expand^value^X raw^txt-accelerometer-x~
~expand^value^Y raw^txt-accelerometer-y~
~expand^value^Z raw^txt-accelerometer-z~
~expand^value^Pitch front to back (Z) deg^txt-tilt-front-deg~
~expand^value^Roll left to right (X) deg^txt-tilt-right-deg~
<h2>Roms</h2>
~expand^value^Rom 0^txt-rom-0~
~expand^value^Rom 1^txt-rom-1~
~expand^value^Rom 2^txt-rom-2~
~expand^value^Rom 3^txt-rom-3~
~expand^value^Rom 0 temperature^txt-rom-data-0~
~expand^value^Rom 1 temperature^txt-rom-data-1~
~expand^value^Rom 2 temperature^txt-rom-data-2~
~expand^value^Rom 3 temperature^txt-rom-data-3~
<h2>LPG</h2>
~expand^value^Sensor mv^txt-lpg-mv~
~expand^value^Sensor resistance^txt-lpg-resistance~
~expand^input^Sensor resistance min^val-lpg-resistance-min^3.0~
~expand^input^Sensor resistance max^val-lpg-resistance-max^3.0~
~expand^input^Sensor volume min^val-lpg-volume-min^3.0~
~expand^input^Sensor volume max^val-lpg-volume-max^3.0~
~expand^value^Volume litres^txt-lpg-volume~
<h2>Ambient</h2>
~expand^value^Outside temperature^txt-outside-temperature~
~expand^value^Internal temperature^txt-heating-temperature~
~expand^input^Outside rom^val-outside-rom^10.0~
~expand^input^Internal rom^val-heating-rom^10.0~
</body>
</html>