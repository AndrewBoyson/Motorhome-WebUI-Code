~setvalue^Config~
<!DOCTYPE html>
<html>
<head>
   <title>Motorhome - ~getvalue~</title>
   <link rel='stylesheet' href='/base.css' type='text/css'/>
   <link rel='stylesheet' href='/settings.css' type='text/css'/>
   <meta name='viewport' content='width=device-width, initial-scale=1'>
   <link rel='icon'       href='/favicon.ico' type='image/x-icon'/>
</head>
<body>
<a class='tab-shortcut' href='#main-content'>Skip to content</a>
~expand^menu~
<h1 id='main-content'>Motorhome - ~getvalue~</h1>
<h2>Alert</h2>
<form action='/config' method='get'>
  <input type='text' style='text-align:left;' value='~SmsAlertNumber~' name='sms-alert-number'>
  <input type='submit' value='Set sms'>
</form>
<form action='/config' method='get'>
  <input type='text' style='text-align:left;' value='test alert' name='sms-send-test-alert'>
  <input type='submit' value='Send sms'>
</form>
<h2>Credentials</h2>
<form action='/config' method='get'>
  <input type='text' style='text-align:left;' value='~CredentialsPassword~' name='credentials-password'>
  <input type='submit' value='Set Password'>
</form>
<form action='/config' method='get'>
  <input type='text' style='text-align:left;' value='~CredentialsId~' name='credentials-reset-id'>
  <input type='submit' value='Reset ID'>
</form>
~expand^edit^../data/settings/curve^Curve~
</body>
</html>