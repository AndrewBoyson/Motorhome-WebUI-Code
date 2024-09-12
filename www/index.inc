<!DOCTYPE html>
<html>
<head>
   <title>Motorhome - Home</title>
   <link rel='stylesheet' href='/base.css' type='text/css'/>
   <script src='/ajax.js' type='text/javascript'></script>
   <script src='/index.js' type='text/javascript'></script>
   <meta name='viewport' content='width=device-width, initial-scale=1'>
   <link rel='icon'       href='/favicon.ico' type='image/x-icon'/>
</head>
<body>
<a href='/control' class='hamburger'>
<div class='bar'  ></div>
<div class='space'></div>
<div class='bar'  ></div>
<div class='space'></div>
<div class='bar'  ></div>
</a>
<br/>
<br/>
<div id='ajax-date-local'></div>
<br/>
~expand^meter^Battery (%)^battery-counted-capacity-percent^0^100^25^90^60~
~expand^meter^Fresh water (L)^fresh-volume^0^100^10^30^80~
~expand^meter^Lpg (L)^lpg-volume-litres^0^20^6.1^10^15~
<br/>
~expand^value^Battery current mA^txt-battery-current-ma~
~expand^value^Days to empty^txt-battery-days-to-empty~
~expand^value^Battery state^txt-battery-output-state~
~expand^value^Battery mode^txt-battery-mode~
<br/>
~expand^value^Pitch degrees^txt-tilt-front-deg~
~expand^value^Roll degrees^txt-tilt-right-deg~
<br/>
~expand^value^Outside temperature^txt-outside-temperature~
~expand^value^Internal temperature^txt-heating-temperature~
<br/>
~expand^led^Water pump^att-control-water-pump~
~expand^led^Water fill^att-control-water-fill~
~expand^led^Water drain^att-control-water-drain~
<br/>
~expand^led^Inverter^att-control-inverter~
~expand^led^EHU^att-control-ehu~
</body>
</html>
