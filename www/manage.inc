~setvalue^Manage~
<!DOCTYPE html>
<html>
<head>
   <title>Motorhome - ~getvalue~</title>
   <link rel='stylesheet' href='/base.css' type='text/css'/>
   <link rel='stylesheet' href='/settings.css' type='text/css'/>
   <script src='/ajax.js' type='text/javascript'></script>
   <script src='/manage.js' type='text/javascript'></script>
   <meta name='viewport' content='width=device-width, initial-scale=1'>
   <link rel='icon'       href='/favicon.ico' type='image/x-icon'/>
</head>
<body>
<a class='tab-shortcut' href='#main-content'>Skip to content</a>
~expand^menu~
<h1 id='main-content'>Motorhome - ~getvalue~</h1>
<h2>Mode</h2>
~expand^switch^Away (neither -> manual)^att-mode-away~
~expand^switch^Home (neither -> manual)^att-mode-home~
~expand^input^Away %^val-battery-away-percent^2.0~
~expand^input^Home %^val-battery-home-percent^2.0~
<h2>Rest</h2>
~expand^value^Battery current mA^txt-battery-current-ma~
~expand^value^Battery output state^txt-battery-output-state~
~expand^value^Seconds to rest^txt-plot-time-to-rest~
~expand^value^Seconds at rest^txt-plot-rest-remaining~
~expand^input^Rest mA^val-battery-rest-ma^2.0~
<h2>Plot</h2>
~expand^input^Rest seconds^val-plot-rest-seconds^3.0~
~expand^switch^Go up^att-plot-up~
~expand^switch^Go down^att-plot-down~
~expand^input^Increment percent^val-plot-inc-percent^2.0~
~expand^input^Maximum percent^val-plot-max-percent^2.0~
~expand^input^Minimum percent^val-plot-min-percent^2.0~
<h2>Calibrate</h2>
~expand^input^Minimum start charge %^val-cal-min-percent^3.0~
~expand^led^Calibration ready or ongoing^att-cal-ok-to-start~
~expand^input^Last done UTC^val-cal-time^10.0~
~expand^input^Base charge %^val-cal-percent^3.0~
~expand^input^Base voltage mV^val-cal-mv^3.0~
~expand^input^Slope As/mV^val-cal-as-per-mv^3.0~
~expand^input^Slope mV/%^val-cal-mv-per-percent^3.0~
</body>
</html>
