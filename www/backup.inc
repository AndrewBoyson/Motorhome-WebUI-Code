~setvalue^Backup~
<!DOCTYPE html>
<html>
<head>
   <title>Motorhome - ~getvalue~</title>
   <link rel='stylesheet' href='/base.css' type='text/css'/>
   <link rel='stylesheet' href='/settings.css' type='text/css'/>
   <script src='/ajax.js' type='text/javascript'></script>
   <script src='/backup.js' type='text/javascript'></script>
   <meta name='viewport' content='width=device-width, initial-scale=1'>
   <link rel='icon'       href='/favicon.ico' type='image/x-icon'/>
</head>
<body>
<a class='tab-shortcut' href='#main-content'>Skip to content</a>
~expand^menu~
<h1 id='main-content'>Motorhome - ~getvalue~</h1>
<h2>USB</h2>
~expand^led^Plugged in^att-usb-drive-plugged-in~
~expand^led^Mounted^att-usb-drive-mounted~
~expand^value^Size MB^txt-usb-size~
~expand^value^Free MB^txt-usb-free~
~expand^value^Used MB^txt-usb-used~
~expand^value^Label^txt-usb-label~
~expand^button^Backup source folder^backup-to-usb^Backup^5.0~
<h2>Source Folder</h2>
<code id='txt-app-list'></code>
<h2>USB Folder</h2>
<code id='txt-usb-list'></code>
</body>
</html>