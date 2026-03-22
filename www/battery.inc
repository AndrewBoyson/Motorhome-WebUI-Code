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
~expand^value^Pulse interval^txt-battery-pulse-interval-secs~
<h2>Output</h2>
~expand^value^Output state^txt-battery-output-state~
~expand^switch^Charge enable^att-battery-charge-enabled~
~expand^switch^Discharge enable^att-battery-discharge-enabled~
~expand^switch^Target mode mV or soc%^att-battery-target-mode~
~expand^input^Target soc%^val-battery-target-soc-percent^2.0~
~expand^input^Voltage rebound mv^val-battery-voltage-rebound-mv^3.0~
~expand^value^Time to target^txt-battery-time-to-target~
<h2>Curve</h2>
~expand^input^Inflexion mV^val-battery-inflexion-mv^3.0~
~expand^input^Inflexion soc%^val-battery-inflexion-percent^3.0~
<h2>Calibrate</h2>
~expand^led^Is at rest^att-battery-is-at-rest~
~expand^value^Time at rest^txt-battery-time-at-rest~
~expand^input^Current settle time hours^val-battery-current-settle-time-hours^3.0~
~expand^input^Voltage settle time hours^val-battery-voltage-settle-time-hours^3.0~
~expand^led^Current is calibrating^att-battery-cal-current-is-active~
~expand^led^SoC calibrating from voltage^att-battery-cal-charge-is-active~
<h2>Cycle</h2>
~expand^value^Pos pulses^txt-battery-count-pos-pulses~
~expand^value^Neg pulses^txt-battery-count-neg-pulses~
~expand^value^Cycle difference As^txt-battery-manage-difference-as~
~expand^value^Cycle difference %^txt-battery-manage-difference-percent~
~expand^input^Adjustment mAs per pulse^val-battery-manage-pulse-adjust-mas^3.0~
<h2>Heater</h2>
~expand^value^Temperature °C^txt-battery-temperature~
~expand^input^Set point °C^val-battery-temperature-setpoint^3.0~
~expand^input^Kp % per °C (100)^val-battery-heater-proportional^3.0~
~expand^input^Ki % per °C per hour (120)^val-battery-heater-integral^3.0~
~expand^value^Ti hours^txt-battery-heater-ti-hours^3.0~
~expand^value^Output power %^txt-battery-heater-output-percent~
~expand^value^Output mA^txt-battery-heater-output-ma~
~expand^value^Output W^txt-battery-heater-output-watts~
</body>
</html>
