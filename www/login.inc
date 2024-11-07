<!DOCTYPE html>
<html>
<head>
   <title>Motorhome - Login</title>
   <link rel='stylesheet' href='/base.css' type='text/css'/>
   <meta name='viewport' content='width=device-width, initial-scale=1'>
   <link rel='icon'       href='/favicon.ico' type='image/x-icon'/>
</head>
<body>
<style>                            * { font-size:4vw; box-sizing: border-box; }
@media (min-width: 600px) { * { font-size:24px; } }

*                        { font-family:'Segoe UI', Calibri, Arial, sans-serif; }
*:focus                  { outline: 0; }
body                     { margin-left:0.1em; line-height:1.1em; padding:0; border:0; }
h1                       { color:darkblue; font-size:120%; font-weight:normal; margin-bottom:0.3em; }
h2                       { color:darkblue; font-size:110%; font-weight:normal; margin-bottom:0.3em; }
div                      { line-height:1.2em; }
form                     { line-height:1.2em; margin:0; }
input[type=text]         { border:thin solid gray; border-radius:0.2em; margin:0; padding:0.1em; width:9em; text-align:left}
input[type=submit]       { display:none; }
</style><h1 id='main-content'>Heating - Login</h1>
<h2>Welcome - please enter the password</h2>
<form action='/' method='get' autocomplete='off'>
  <div style='width:8em; display:inline-block;'>Password</div>
  <input type='text'   name='password' value='' autofocus>
  <input type='submit'                 value='' >
</form>
</body>
</html>
