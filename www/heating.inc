~setvalue^Heating~
<!DOCTYPE html>
<html>
<head>
   <title>Motorhome - ~getvalue~</title>
   <link rel='stylesheet' href='/base.css' type='text/css'/>
   <link rel='stylesheet' href='/settings.css' type='text/css'/>
   <script src='/ajax.js' type='text/javascript'></script>
   <script src='/heating.js' type='text/javascript'></script>
   <meta name='viewport' content='width=device-width, initial-scale=1'>
   <link rel='icon'       href='/favicon.ico' type='image/x-icon'/>
</head>
<body>
<a class='tab-shortcut' href='#main-content'>Skip to content</a>
~expand^menu~
<h1 id='main-content'>Motorhome - ~getvalue~</h1>
<br/>
~expand^switch^Lin trace^att-trace~
<!-- ~expand^switch^Lin allow bus writes^att-allow-bus-writes~ -->
~expand^led^Lin active^att-active~
~expand^value^Seconds since last status^txt-seconds-since-last-status~

<h2>Settings to send</h2>
~expand^switch^Room heater^att-wanted-room-on~
<div class='line'>
<label for='val-wanted-room-temp'>Room temperature</label>
<select id='val-wanted-room-temp' onchange='change(this)'>
  <option value='5'>5</option>
  <option value='6'>6</option>
  <option value='7'>7</option>
  <option value='8'>8</option>
  <option value='9'>9</option>
  <option value='10'>10</option>
  <option value='11'>11</option>
  <option value='12'>12</option>
  <option value='13'>13</option>
  <option value='14'>14</option>
  <option value='15'>15</option>
  <option value='16'>16</option>
  <option value='17'>17</option>
  <option value='18'>18</option>
  <option value='19'>19</option>
  <option value='20'>20</option>
  <option value='21'>21</option>
  <option value='22'>22</option>
  <option value='23'>23</option>
  <option value='24'>24</option>
  <option value='25'>25</option>
</select>
</div>
<div class='line'>
<label for='val-wanted-fan-mode'>Fan mode</label>
<select id='val-wanted-fan-mode' onchange='change(this)'>
  <option value='E'>Eco</option>
  <option value='H'>High</option>
</select>
</div>
<br/>
~expand^switch^Water heater^att-wanted-water-on~
<div class='line'>
<label for='val-wanted-water-temp'>Water temperature</label>
<select id='val-wanted-water-temp' onchange='change(this)'>
  <option value='E'>Eco 40°</option>
  <option value='H'>High 60°</option>
  <option value='B'>Boost 65°</option>
</select>
</div>
<br/>
<div class='line'>
<label for='val-wanted-energy-sel'>Energy selection</label>
<select id='val-wanted-energy-sel' onchange='change(this)'>
  <option value='G'>Gas</option>
  <option value='e'>Elec 1</option>
  <option value='E'>Elec 2</option>
  <option value='m'>Mix 1</option>
  <option value='M'>Mix 2</option>
</select>
</div>
<br/>
~expand^button^^truma-send-wanted^Send^3.0~
~expand^led^Send ongoing^att-send-ongoing~
~expand^value^Seconds since last send^txt-seconds-since-last-send~

<h2>Controller status</h2>
~expand^value^Room temp sensor^txt-actual-room-temp~
~expand^led^Room heater on^att-target-room-on~
~expand^value^Room temp target^txt-target-room-temp~
~expand^value^Fan mode^txt-target-fan-mode~
<br/>
~expand^value^Water temp sensor^txt-actual-water-temp~
~expand^led^Water heater on^att-target-water-on~
~expand^value^Water temp target^txt-target-water-temp~
<br/>
~expand^value^Energy selection^txt-target-energy-sel~
<br/>
~expand^value^Receive status^txt-actual-recv-status~
~expand^value^Op status^txt-actual-op-status~
~expand^value^Error code^txt-actual-error-code~
</body>
</html>