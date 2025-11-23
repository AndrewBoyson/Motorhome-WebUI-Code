//Control script
'use strict';

let trace                  = false;
let allowBusWrites         = false;
let isActive               = false;
let secondsSinceLastStatus = '';
let secondsSinceLastSend   = '';
let sendOngoing            = false;
let wantedRoomOn           = false;
let wantedWaterOn          = false;
let wantedRoomTemp         = '';
let wantedWaterTemp        = '';
let wantedFanMode          = '';
let wantedEnergySel        = '';
let targetRoomTemp         = '';
let targetWaterTemp        = '';
let targetFanMode          = '';
let targetEnergySel        = '';
let targetElecPower        = '';
let actualRoomTemp         = '';
let actualWaterTemp        = '';
let actualRecvStatus       = '';
let actualOpStatus         = '';
let actualErrorCode        = '';

function parse()
{
    let lines = Ajax.response.split('\n');
    trace                  = lines[ 0] === '1';
    allowBusWrites         = lines[ 1] === '1';
    isActive               = lines[ 2] === '1';
    secondsSinceLastStatus = lines[ 3];
    secondsSinceLastSend   = lines[ 4];
    sendOngoing            = lines[ 5] === '1';
    wantedRoomOn           = lines[ 6] === '1';
    wantedWaterOn          = lines[ 7] === '1';
    wantedRoomTemp         = lines[ 8];
    wantedWaterTemp        = lines[ 9];
    wantedFanMode          = lines[10];
    wantedEnergySel        = lines[11];
    targetRoomTemp         = lines[12];
    targetWaterTemp        = lines[13];
    targetFanMode          = lines[14];
    targetEnergySel        = lines[15];
    targetElecPower        = lines[16];
    actualRoomTemp         = lines[17];
    actualWaterTemp        = lines[18];
    actualRecvStatus       = lines[19];
    actualOpStatus         = lines[20];
    actualErrorCode        = lines[21];
}

function targetWaterTempAsString()
{
    switch (targetWaterTemp)
    {
         case 'O': return 'Off';
         case 'E': return 'Eco 40°';
         case 'H': return 'High 60°';
         case 'B': return 'Boost 65°';
    }
    return 'Unknown';
}
function targetFanModeAsString()
{
    switch (targetFanMode)
    {
         case 'O': return 'Off';
         case 'E': return 'Eco';
         case 'H': return 'High';
    }
    return 'Unknown';
}
function targetEnergySelAsString()
{
    switch (targetEnergySel)
    {
         case 'G': return 'Gas';
         case 'E': return 'Elec ' + targetElecPower;
         case 'M': return 'Mix '  + targetElecPower;
    }
    return 'Unknown';
}
function display()
{
    let elem;
    elem = Ajax.getElementOrNull('att-trace'                     ); if (elem) elem.setAttribute('dir', trace         ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-allow-bus-writes'          ); if (elem) elem.setAttribute('dir', allowBusWrites? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-active'                    ); if (elem) elem.setAttribute('dir', isActive      ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('txt-seconds-since-last-status' ); if (elem) elem.textContent =  secondsSinceLastStatus == 65535 ? '-' : secondsSinceLastStatus;
    elem = Ajax.getElementOrNull('txt-seconds-since-last-send'   ); if (elem) elem.textContent =  secondsSinceLastSend   == 65535 ? '-' : secondsSinceLastSend;
    elem = Ajax.getElementOrNull('att-send-ongoing'              ); if (elem) elem.setAttribute('dir', sendOngoing   ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-wanted-room-on'            ); if (elem) elem.setAttribute('dir', wantedRoomOn  ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-wanted-water-on'           ); if (elem) elem.setAttribute('dir', wantedWaterOn ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('val-wanted-room-temp'          ); if (elem) elem.value       =  wantedRoomTemp;
    elem = Ajax.getElementOrNull('val-wanted-water-temp'         ); if (elem) elem.value       =  wantedWaterTemp;
    elem = Ajax.getElementOrNull('val-wanted-fan-mode'           ); if (elem) elem.value       =  wantedFanMode;
    elem = Ajax.getElementOrNull('val-wanted-energy-sel'         ); if (elem) elem.value       =  wantedEnergySel;
    elem = Ajax.getElementOrNull('att-target-room-on'            ); if (elem) elem.setAttribute('dir', targetRoomTemp  !== '0' ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('txt-target-room-temp'          ); if (elem) elem.textContent =  targetRoomTemp === '0' ? 'Off' : targetRoomTemp;
    elem = Ajax.getElementOrNull('att-target-water-on'           ); if (elem) elem.setAttribute('dir', targetWaterTemp !== 'O' ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('txt-target-water-temp'         ); if (elem) elem.textContent =  targetWaterTempAsString();
    elem = Ajax.getElementOrNull('txt-target-fan-mode'           ); if (elem) elem.textContent =  targetFanModeAsString();
    elem = Ajax.getElementOrNull('txt-target-energy-sel'         ); if (elem) elem.textContent =  targetEnergySelAsString();
    elem = Ajax.getElementOrNull('txt-actual-room-temp'          ); if (elem) elem.textContent = (actualRoomTemp  / 10 - 273).toFixed(1);
    elem = Ajax.getElementOrNull('txt-actual-water-temp'         ); if (elem) elem.textContent = (actualWaterTemp / 10 - 273).toFixed(0);
    elem = Ajax.getElementOrNull('txt-actual-recv-status'        ); if (elem) elem.textContent =  actualRecvStatus;
    elem = Ajax.getElementOrNull('txt-actual-op-status'          ); if (elem) elem.textContent =  actualOpStatus;
    elem = Ajax.getElementOrNull('txt-actual-error-code'         ); if (elem) elem.textContent =  actualErrorCode;
}

function change(elem)
{
    if (elem.id === 'att-trace'            ) AjaxSendNameValue('lin-trace'              ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'att-allow-bus-writes' ) AjaxSendNameValue('lin-allow-bus-writes'   ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'att-wanted-room-on'   ) AjaxSendNameValue('truma-wanted-room-on'   ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'att-wanted-water-on'  ) AjaxSendNameValue('truma-wanted-water-on'  ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'val-wanted-room-temp' ) AjaxSendNameValue('truma-wanted-room-temp' ,  elem.value);
    if (elem.id === 'val-wanted-water-temp') AjaxSendNameValue('truma-wanted-water-temp',  elem.value);
    if (elem.id === 'val-wanted-fan-mode'  ) AjaxSendNameValue('truma-wanted-fan-mode'  ,  elem.value);
    if (elem.id === 'val-wanted-energy-sel') AjaxSendNameValue('truma-wanted-energy-sel',  elem.value);
}

Ajax.server     = '/heating-ajax';
Ajax.onResponse = function() { parse(); display(); };
Ajax.init();