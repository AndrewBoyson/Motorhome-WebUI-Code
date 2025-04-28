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

<h2>Sms alerts</h2>
<form action='/config' method='get'>
  <input type='text' style='text-align:left;' value='test alert' name='sms-send-test-alert'>
  <input type='submit' value='Send sms'>
</form>
<form action='/config' method='get'>
  <input type='text' style='text-align:left;' value='~SmsAlertNumber~' name='sms-alert-number'>
  <input type='submit' value='Set sms'>
</form>
<form action='/config' method='get'>
  <input type='text' style='text-align:left;' value='~SmsUserName~' name='sms-username'>
  <input type='submit' value='Username'>
</form>
<form action='/config' method='get'>
  <input type='text' style='text-align:left;' value='~SmsPassword~' name='sms-password'>
  <input type='submit' value='Password'>
</form>
<form action='/config' method='get'>
  <input type='text' style='text-align:left;' value='~SmsHostName~' name='sms-hostname'>
  <input type='submit' value='Hostname'>
</form>
<p>Username <code>~SmsUserName~</code> and password <code>~SmsPassword~</code> must be an account in a group with write access to
Services > Mobile Utilities > Messages > Send Messages to allow SMSs to be sent by the RUT.</p>
<p>Hostname <code>~SmsHostName~</code> must be same as that in RUT System > General > Hostname.</p>

<h2>Credentials</h2>
<form action='/config' method='get'>
  <input type='text' style='text-align:left;' value='~CredentialsPassword~' name='credentials-password'>
  <input type='submit' value='Password'>
</form>
<p>The session id <code>~CredentialsId~</code> changes whenever the password is set</p>
<p>password=<code>~CredentialsPassword~</code> must be set
in RUT Services > Mobile Utilities > SMS Gateway > SMS Forwarding > Extra data pair 1 to allow received SMSs to be acted on by the Pi.</p>
~expand^edit^../data/settings/curve^Curve~
</body>
</html>