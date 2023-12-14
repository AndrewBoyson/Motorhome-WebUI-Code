~setvalue^Battery~
<!DOCTYPE html>
<html>
<head>
   <title>Motorhome - ~getvalue~</title>
   <link rel='stylesheet' href='/base.css' type='text/css'/>
   <link rel='stylesheet' href='/settings.css' type='text/css'/>
   <script src='/ajax.js' type='text/javascript'></script>
   <script src='/battery.js' type='text/javascript'></script>
   <meta name='viewport' content='width=device-width, initial-scale=1'>
   <link rel='icon'       href='/favicon.ico' type='image/x-icon'/>
</head>
<body>
<a class='tab-shortcut' href='#main-content'>Skip to content</a>
~expand^menu~
<h1 id='main-content'>Motorhome - ~getvalue~</h1>
<h2>Charge</h2>
~expand^input^Coulombs As^val-battery-counted-capacity-amp-seconds^4.0~
~expand^value^Amp hours Ah^txt-battery-counted-capacity-amp-hours~
~expand^input^Percent %^val-battery-counted-capacity-percent^3.0~
<h2>Voltage</h2>
~expand^value^Battery voltage mV^txt-battery-voltage-mv~
~expand^value^Cell voltage mV^txt-battery-cell-voltage-mv~
<h2>Current</h2>
~expand^value^Battery current mA^txt-battery-current-ma~
<h2>Charge correction</h2>
~expand^input^As per hour^val-battery-aging-as-per-hour^3.0~
~expand^value^mA^txt-battery-aging-ma~
~expand^input^Percent per month^val-battery-aging-percent-per-month^3.0~
<h2>Heater</h2>
~expand^value^Temperature °C^txt-battery-temperature~
~expand^input^Set point °C^val-battery-temperature-setpoint^3.0~
~expand^input^Kp power % per °C^val-battery-heater-proportional^3.0~
~expand^input^Ki power % per °C per hour^val-battery-heater-integral^3.0~
~expand^value^Ti hours^txt-battery-heater-ti-hours^3.0~
~expand^value^Output power %^txt-battery-heater-output-percent~
~expand^value^Output mA^txt-battery-heater-output-ma~
~expand^value^Output W^txt-battery-heater-output-watts~
<h2>State</h2>
~expand^input^Battery capacity target %^val-battery-capacity-setpoint-percent^2.0~
~expand^value^Battery output state^txt-battery-output-state~
~expand^value^Seconds to target^txt-battery-seconds-to-target~
~expand^value^Hours to target^txt-battery-hours-to-target~
~expand^switch^Battery charge enable^att-battery-charge-enabled~
~expand^switch^Battery discharge enable^att-battery-discharge-enabled~
</body>
</html>
