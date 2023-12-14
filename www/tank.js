//Tank script
'use strict';

let freshTemp16ths        = '';
let freshRom              = '';
let freshSupplyMv         = '';
let freshBaseTemp16ths    = '';
let freshBaseMv           = '';
let freshUvPer16th        = '';
let freshMv               = '';
let freshLevel            = '';
let freshVolume           = '';
let freshSensorFront      = '';
let freshSensorRight      = '';
let freshWidth            = '';
let freshLength           = '';
let accelerometerXFlat    = '';
let accelerometerYFlat    = '';
let accelerometerZFlat    = '';
let accelerometerX        = '';
let accelerometerY        = '';
let accelerometerZ        = '';
let rom0                  = '';
let rom1                  = '';
let rom2                  = '';
let rom3                  = '';
let romData0              = '';
let romData1              = '';
let romData2              = '';
let romData3              = '';
let lpgMv                 = '';
let lpgResistance16ths    = '';
let lpgResistanceMin16ths = '';
let lpgResistanceMax16ths = '';
let lpgVolumeMinMl        = '';
let lpgVolumeMaxMl        = '';
let lpgVolumeMl           = '';
let outsideRom            = '';
let heatingRom            = '';
let outsideTemp16ths      = '';
let heatingTemp16ths      = '';

function parse()
{
    let lines = Ajax.response.split('\n');
    freshTemp16ths        = lines[ 0];
    freshRom              = lines[ 1];
    freshSupplyMv         = lines[ 2];
    freshBaseTemp16ths    = lines[ 3];
    freshBaseMv           = lines[ 4];
    freshUvPer16th        = lines[ 5];
    freshMv               = lines[ 6];
    freshLevel            = lines[ 7];
    freshVolume           = lines[ 8];
    freshSensorFront      = lines[ 9];
    freshSensorRight      = lines[10];
    freshWidth            = lines[11];
    freshLength           = lines[12];
    accelerometerXFlat    = lines[13];
    accelerometerYFlat    = lines[14];
    accelerometerZFlat    = lines[15];
    accelerometerX        = lines[16];
    accelerometerY        = lines[17];
    accelerometerZ        = lines[18];
    rom0                  = lines[19];
    rom1                  = lines[20];
    rom2                  = lines[21];
    rom3                  = lines[22];
    romData0              = lines[23];
    romData1              = lines[24];
    romData2              = lines[25];
    romData3              = lines[26];
    lpgMv                 = lines[27];
    lpgResistance16ths    = lines[28];
    lpgResistanceMin16ths = lines[29];
    lpgResistanceMax16ths = lines[30];
    lpgVolumeMinMl        = lines[31];
    lpgVolumeMaxMl        = lines[32];
    lpgVolumeMl           = lines[33];
    outsideRom            = lines[34];
    heatingRom            = lines[35];
    outsideTemp16ths      = lines[36];
    heatingTemp16ths      = lines[37];
}


function display()
{
    let elem;

    let tiltXrad = accelerometerX/16384;
    let tiltXdeg = tiltXrad * 57;          //180/pi = 57.3
    let tiltYrad = accelerometerY/16384;
    let tiltYdeg = tiltYrad * 57;          //180/pi = 57.3
    let tiltZrad = accelerometerZ/16384;
    let tiltZdeg = tiltZrad * 57;          //180/pi = 57.3

    let mvComp = (freshTemp16ths - freshBaseTemp16ths) * freshUvPer16th / 1024;
    let mmComp = (accelerometerX * freshSensorRight - accelerometerZ * freshSensorFront) / 16384;

    elem = Ajax.getElementOrNull('txt-fresh-temperature'    ); if (elem) elem.textContent =  (freshTemp16ths / 16).toFixed(1);
    elem = Ajax.getElementOrNull('val-fresh-rom'            ); if (elem) elem.value       =  freshRom;
    elem = Ajax.getElementOrNull('txt-fresh-supply-mv'      ); if (elem) elem.textContent =  freshSupplyMv;
    elem = Ajax.getElementOrNull('val-fresh-base-degrees'   ); if (elem) elem.value       =  (freshBaseTemp16ths / 16).toFixed(1);
    elem = Ajax.getElementOrNull('val-fresh-base-mv'        ); if (elem) elem.value       =  freshBaseMv;
    elem = Ajax.getElementOrNull('val-fresh-mv-per-degree'  ); if (elem) elem.value       =  (freshUvPer16th * 16 / 1024).toFixed(2);
    elem = Ajax.getElementOrNull('txt-fresh-mv'             ); if (elem) elem.textContent =  freshMv;
    elem = Ajax.getElementOrNull('txt-fresh-mv-compensation'); if (elem) elem.textContent =  mvComp.toFixed(0);
    elem = Ajax.getElementOrNull('txt-fresh-mm-compensation'); if (elem) elem.textContent =  mmComp.toFixed(0);
    elem = Ajax.getElementOrNull('txt-fresh-level'          ); if (elem) elem.textContent =  freshLevel;
    elem = Ajax.getElementOrNull('txt-fresh-volume'         ); if (elem) elem.textContent =  freshVolume;
    elem = Ajax.getElementOrNull('val-fresh-sensor-front'   ); if (elem) elem.value       =  freshSensorFront;
    elem = Ajax.getElementOrNull('val-fresh-sensor-right'   ); if (elem) elem.value       =  freshSensorRight;
    elem = Ajax.getElementOrNull('val-fresh-width'          ); if (elem) elem.value       =  freshWidth;
    elem = Ajax.getElementOrNull('val-fresh-length'         ); if (elem) elem.value       =  freshLength;
    elem = Ajax.getElementOrNull('val-accelerometer-x-flat' ); if (elem) elem.value       =  accelerometerXFlat;
    elem = Ajax.getElementOrNull('val-accelerometer-y-flat' ); if (elem) elem.value       =  accelerometerYFlat;
    elem = Ajax.getElementOrNull('val-accelerometer-z-flat' ); if (elem) elem.value       =  accelerometerZFlat;
    elem = Ajax.getElementOrNull('txt-accelerometer-x'      ); if (elem) elem.textContent =  accelerometerX;
    elem = Ajax.getElementOrNull('txt-accelerometer-y'      ); if (elem) elem.textContent =  accelerometerY;
    elem = Ajax.getElementOrNull('txt-accelerometer-z'      ); if (elem) elem.textContent =  accelerometerZ;
    elem = Ajax.getElementOrNull('txt-tilt-right-deg'       ); if (elem) elem.textContent =  tiltXdeg.toFixed(1);
    elem = Ajax.getElementOrNull('txt-tilt-front-deg'       ); if (elem) elem.textContent =  tiltZdeg.toFixed(1);
    elem = Ajax.getElementOrNull('txt-rom-0'                ); if (elem) elem.textContent =  rom0;
    elem = Ajax.getElementOrNull('txt-rom-1'                ); if (elem) elem.textContent =  rom1;
    elem = Ajax.getElementOrNull('txt-rom-2'                ); if (elem) elem.textContent =  rom2;
    elem = Ajax.getElementOrNull('txt-rom-3'                ); if (elem) elem.textContent =  rom3;
    elem = Ajax.getElementOrNull('txt-rom-data-0'           ); if (elem) elem.textContent =  (romData0 / 16).toFixed(1);
    elem = Ajax.getElementOrNull('txt-rom-data-1'           ); if (elem) elem.textContent =  (romData1 / 16).toFixed(1);
    elem = Ajax.getElementOrNull('txt-rom-data-2'           ); if (elem) elem.textContent =  (romData2 / 16).toFixed(1);
    elem = Ajax.getElementOrNull('txt-rom-data-3'           ); if (elem) elem.textContent =  (romData3 / 16).toFixed(1);
    elem = Ajax.getElementOrNull('txt-lpg-mv'               ); if (elem) elem.textContent =  lpgMv;
    elem = Ajax.getElementOrNull('txt-lpg-resistance'       ); if (elem) elem.textContent =  (lpgResistance16ths    / 16).toFixed(1);
    elem = Ajax.getElementOrNull('val-lpg-resistance-min'   ); if (elem) elem.value       =  (lpgResistanceMin16ths / 16).toFixed(1);
    elem = Ajax.getElementOrNull('val-lpg-resistance-max'   ); if (elem) elem.value       =  (lpgResistanceMax16ths / 16).toFixed(1);
    elem = Ajax.getElementOrNull('val-lpg-volume-min'       ); if (elem) elem.value       =  (lpgVolumeMinMl / 1024).toFixed(1);
    elem = Ajax.getElementOrNull('val-lpg-volume-max'       ); if (elem) elem.value       =  (lpgVolumeMaxMl / 1024).toFixed(1);
    elem = Ajax.getElementOrNull('txt-lpg-volume'           ); if (elem) elem.textContent =  (lpgVolumeMl    / 1024).toFixed(1);
    elem = Ajax.getElementOrNull('val-outside-rom'          ); if (elem) elem.value       =  outsideRom;
    elem = Ajax.getElementOrNull('val-heating-rom'          ); if (elem) elem.value       =  heatingRom;
    elem = Ajax.getElementOrNull('txt-outside-temperature'  ); if (elem) elem.textContent =  (outsideTemp16ths / 16).toFixed(1);
    elem = Ajax.getElementOrNull('txt-heating-temperature'  ); if (elem) elem.textContent =  (heatingTemp16ths / 16).toFixed(1);
}

function change(elem)
{
    if (elem.id === 'val-fresh-base-degrees'   ) AjaxSendNameValue('tank-fresh-base-temp-16ths'   ,  String(parseFloat(elem.value) * 16));
    if (elem.id === 'val-fresh-base-mv'        ) AjaxSendNameValue('tank-fresh-base-mv'           ,  elem.value);
    if (elem.id === 'val-fresh-mv-per-degree'  ) AjaxSendNameValue('tank-fresh-uv-per-16th'       ,  String(parseFloat(elem.value) * 1024 / 16));
    if (elem.id === 'val-fresh-sensor-front'   ) AjaxSendNameValue('tank-fresh-sensor-front'      ,  elem.value);
    if (elem.id === 'val-fresh-sensor-right'   ) AjaxSendNameValue('tank-fresh-sensor-right'      ,  elem.value);
    if (elem.id === 'val-fresh-width'          ) AjaxSendNameValue('tank-fresh-width'             ,  elem.value);
    if (elem.id === 'val-fresh-length'         ) AjaxSendNameValue('tank-fresh-length'            ,  elem.value);
    if (elem.id === 'val-accelerometer-x-flat' ) AjaxSendNameValue('tank-accelerometer-x-flat'    ,  elem.value);
    if (elem.id === 'val-accelerometer-y-flat' ) AjaxSendNameValue('tank-accelerometer-y-flat'    ,  elem.value);
    if (elem.id === 'val-accelerometer-z-flat' ) AjaxSendNameValue('tank-accelerometer-z-flat'    ,  elem.value);
    if (elem.id === 'val-fresh-rom'            ) AjaxSendNameValue('tank-fresh-rom'               ,  elem.value);
    if (elem.id === 'val-lpg-resistance-min'   ) AjaxSendNameValue('tank-lpg-resistance-min-16ths',  String(parseFloat(elem.value) * 16  ));
    if (elem.id === 'val-lpg-resistance-max'   ) AjaxSendNameValue('tank-lpg-resistance-max-16ths',  String(parseFloat(elem.value) * 16  ));
    if (elem.id === 'val-lpg-volume-min'       ) AjaxSendNameValue('tank-lpg-volume-min-ml'       ,  String(parseFloat(elem.value) * 1024));
    if (elem.id === 'val-lpg-volume-max'       ) AjaxSendNameValue('tank-lpg-volume-max-ml'       ,  String(parseFloat(elem.value) * 1024));
    if (elem.id === 'val-outside-rom'          ) AjaxSendNameValue('ambient-outside-rom'          ,  elem.value);
    if (elem.id === 'val-heating-rom'          ) AjaxSendNameValue('ambient-heating-rom'          ,  elem.value);
}

Ajax.server     = '/tank-ajax';
Ajax.onResponse = function() { parse(); display(); };
Ajax.init();