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
~expand^input^Coulombs As^val-battery-counted-capacity-amp-seconds^5.0~
~expand^value^Amp hours Ah^txt-battery-counted-capacity-amp-hours~
~expand^input^Percent %^val-battery-counted-capacity-percent^3.0~
<h2>Voltage</h2>
~expand^value^Battery voltage mV^txt-battery-voltage-mv~
~expand^value^Cell voltage mV^txt-battery-cell-voltage-mv~
<h2>Current</h2>
~expand^value^Measured mA^txt-battery-current-ma~
~expand^input^Offset correction mA^val-battery-current-offset-ma^3.0~
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
~expand^switch^Target mode mV or soc%^att-battery-target-mode~
~expand^input^Target soc%^val-battery-target-soc-percent^2.0~
~expand^input^Target mV^val-battery-target-mv^3.0~
~expand^input^Voltage settle time minutes^val-battery-voltage-settle-time-mins^3.0~
~expand^input^Voltage rebound mv^val-battery-voltage-rebound-mv^3.0~
~expand^value^Output state^txt-battery-output-state~
~expand^value^Time to target^txt-battery-time-to-target~
~expand^switch^Charge enable^att-battery-charge-enabled~
~expand^switch^Discharge enable^att-battery-discharge-enabled~
~expand^value^Time at rest^txt-battery-time-at-rest~
</body>
</html>
