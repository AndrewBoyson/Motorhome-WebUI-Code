//Battery script
'use strict';

let batteryCountedCapacityAs      = '';
let batteryCurrentMa              = '';
let batteryTargetSocPercent       = '';
let batteryOutputState            = '';
let batteryChargeEnabled          = false;
let batteryDischargeEnabled       = false;
let batteryTemperature8bfdp       = '';
let batteryTemperatureSetPoint    = '';
let batteryHeater8bfdp            = '';
let batteryVoltageMv              = '';
let batteryCurrentOffsetMa        = '';
let batteryHeaterProportional     = '';
let batteryHeaterIntegral         = '';
let batteryTargetMode             = false;
let batteryTargetMv               = '';
let batteryMsAtRest               = '';
let batteryVoltageSettleTimeMins  = '';

function parse()
{
    let lines = Ajax.response.split('\n');
    batteryCountedCapacityAs      = lines[ 0];
    batteryCurrentMa              = lines[ 1];
    batteryTargetSocPercent       = lines[ 2];
    batteryOutputState            = lines[ 3];
    batteryChargeEnabled          = lines[ 4] === '1';
    batteryDischargeEnabled       = lines[ 5] === '1';
    batteryTemperature8bfdp       = lines[ 6];
    batteryTemperatureSetPoint    = lines[ 7];
    batteryHeater8bfdp            = lines[ 8];
    batteryVoltageMv              = lines[ 9];
    batteryCurrentOffsetMa        = lines[10];
    batteryHeaterProportional     = lines[11];
    batteryHeaterIntegral         = lines[12];
    batteryTargetMode             = lines[13] === '1';
    batteryTargetMv               = lines[14];
    batteryMsAtRest               = lines[15];
    batteryVoltageSettleTimeMins  = lines[16];
}

const CAPACITY_AH        = 280;
const AS_PER_PULSE       = 61.444;
const AS_PER_PERCENT     = CAPACITY_AH*36;
const PULSES_PER_PERCENT = AS_PER_PERCENT/AS_PER_PULSE;

function formatSeconds(value)
{
    if      (!isFinite(value)) return ('never');
    else if (value <       0)  return ('-');
    else if (value <       1)  return ('0');
    else if (value <      60)  return               value.toFixed(0) + ' seconds';
    else if (value <    3600)  return (value / (     60)).toFixed(1) + ' minutes';
    else if (value < 24*3600)  return (value / (   3600)).toFixed(1) + ' hours';
    else                       return (value / (24*3600)).toFixed(1) + ' days';
}

function display()
{
    let targetAs = batteryTargetSocPercent * AS_PER_PERCENT;
    if (batteryTargetMode) targetAs = 57 * AS_PER_PERCENT; //Target mode is mv so assume corresponds to about 57%
    let capacityToTargetAs = targetAs - batteryCountedCapacityAs;

    let timeToEmptySeconds  = -batteryCountedCapacityAs / batteryCurrentMa * 1000;
    let sTimeToEmptySeconds = timeToEmptySeconds >= 0 ? timeToEmptySeconds.toFixed(0) : '-';
    let sTimeToEmptyDays    = timeToEmptySeconds >= 0 ? (timeToEmptySeconds  / 3600 / 24).toFixed(1) : '-';

    let timeToTargetSeconds  = capacityToTargetAs / batteryCurrentMa * 1000;
    let sTimeToTargetSeconds = timeToTargetSeconds >= 0 ? timeToTargetSeconds.toFixed(0) : '-';
    let sTimeToTargetHours   = timeToTargetSeconds >= 0 ? (timeToTargetSeconds / 3600).toFixed(1) : '-';

    let heaterVoltage = Math.sqrt(batteryHeater8bfdp * 256) / 256 * 14;
    let heaterMa      = heaterVoltage / 8.8 * 1000;
    let heaterWatts   = heaterVoltage * heaterVoltage / 8.8;

    let TiMinutes = batteryHeaterProportional / batteryHeaterIntegral;

    let elem;
    elem = Ajax.getElementOrNull('txt-battery-counted-capacity-amp-seconds' ); if (elem) elem.textContent =  batteryCountedCapacityAs;
    elem = Ajax.getElementOrNull('val-battery-counted-capacity-amp-seconds' ); if (elem) elem.value       =  batteryCountedCapacityAs;
    elem = Ajax.getElementOrNull('txt-battery-counted-capacity-amp-hours'   ); if (elem) elem.textContent = (batteryCountedCapacityAs/3600).toFixed(1);
    elem = Ajax.getElementOrNull('txt-battery-counted-capacity-percent'     ); if (elem) elem.textContent = (batteryCountedCapacityAs / AS_PER_PERCENT).toFixed(1);
    elem = Ajax.getElementOrNull('val-battery-counted-capacity-percent'     ); if (elem) elem.value       = (batteryCountedCapacityAs / AS_PER_PERCENT).toFixed(1);
    elem = Ajax.getElementOrNull('txt-battery-current-ma'                   ); if (elem) elem.textContent =  batteryCurrentMa;
    elem = Ajax.getElementOrNull('val-battery-target-soc-percent'           ); if (elem) elem.value       =  batteryTargetSocPercent;
    elem = Ajax.getElementOrNull('txt-battery-output-state'                 ); if (elem) elem.textContent =  batteryOutputState;
    elem = Ajax.getElementOrNull('att-battery-charge-enabled'               ); if (elem) elem.setAttribute('dir', batteryChargeEnabled    ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-battery-discharge-enabled'            ); if (elem) elem.setAttribute('dir', batteryDischargeEnabled ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-battery-target-mode'                  ); if (elem) elem.setAttribute('dir', batteryTargetMode       ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('txt-battery-temperature'                  ); if (elem) elem.textContent = (batteryTemperature8bfdp/256).toFixed(1);
    elem = Ajax.getElementOrNull('val-battery-temperature-setpoint'         ); if (elem) elem.value       = (batteryTemperatureSetPoint/10).toFixed(1);
    elem = Ajax.getElementOrNull('txt-battery-heater-output-percent'        ); if (elem) elem.textContent =  (batteryHeater8bfdp * 100 / 256).toFixed(1);
    elem = Ajax.getElementOrNull('txt-battery-heater-output-ma'             ); if (elem) elem.textContent =  heaterMa.toFixed(0);
    elem = Ajax.getElementOrNull('txt-battery-heater-output-watts'          ); if (elem) elem.textContent =  heaterWatts.toFixed(1);
    elem = Ajax.getElementOrNull('txt-battery-voltage-mv'                   ); if (elem) elem.textContent =  batteryVoltageMv;
    elem = Ajax.getElementOrNull('txt-battery-cell-voltage-mv'              ); if (elem) elem.textContent = (batteryVoltageMv/4).toFixed(0);
    elem = Ajax.getElementOrNull('txt-battery-time-to-target'               ); if (elem) elem.textContent = formatSeconds(timeToTargetSeconds);
    elem = Ajax.getElementOrNull('txt-battery-days-to-empty'                ); if (elem) elem.textContent = sTimeToEmptyDays;
    elem = Ajax.getElementOrNull('val-battery-uv-per-ma'                    ); if (elem) elem.value       =  batteryUvPerMa;
    elem = Ajax.getElementOrNull('val-battery-current-offset-ma'            ); if (elem) elem.value       =  batteryCurrentOffsetMa;
    elem = Ajax.getElementOrNull('val-battery-heater-proportional'          ); if (elem) elem.value       = (batteryHeaterProportional / 256 / 256 * 100     ).toFixed(0);
    elem = Ajax.getElementOrNull('val-battery-heater-integral'              ); if (elem) elem.value       = (batteryHeaterIntegral     / 256 / 256 * 100 * 60).toFixed(0);
    elem = Ajax.getElementOrNull('txt-battery-heater-ti-hours'              ); if (elem) elem.textContent = (TiMinutes / 60).toFixed(2);
    elem = Ajax.getElementOrNull('val-battery-target-mv'                    ); if (elem) elem.value       =  batteryTargetMv;
    elem = Ajax.getElementOrNull('txt-battery-time-at-rest'                 ); if (elem) elem.textContent =  formatSeconds(batteryMsAtRest / 1000);
    elem = Ajax.getElementOrNull('val-battery-voltage-settle-time-mins'     ); if (elem) elem.value       =  batteryVoltageSettleTimeMins;
}

function change(elem)
{   
    if (elem.id === 'val-battery-counted-capacity-amp-seconds') AjaxSendNameValue('battery-counted-capacity-amp-seconds',  elem.value);
    if (elem.id === 'val-battery-counted-capacity-percent'    ) AjaxSendNameValue('battery-counted-capacity-amp-seconds', (elem.value * AS_PER_PERCENT).toFixed(0));
    if (elem.id === 'val-battery-current-offset-ma'           ) AjaxSendNameValue('battery-current-offset-ma'           ,  elem.value);
    if (elem.id === 'val-battery-temperature-setpoint'        ) AjaxSendNameValue('battery-temperature-target-tenths'   , (elem.value * 10).toFixed(0));
    if (elem.id === 'att-battery-target-mode'                 ) AjaxSendNameValue('battery-target-mode'                 ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'val-battery-target-soc-percent'          ) AjaxSendNameValue('battery-target-soc-percent'          ,  elem.value);
    if (elem.id === 'val-battery-target-mv'                   ) AjaxSendNameValue('battery-target-mv'                   ,  elem.value);
    if (elem.id === 'att-battery-charge-enabled'              ) AjaxSendNameValue('battery-charge-enabled'              ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'att-battery-discharge-enabled'           ) AjaxSendNameValue('battery-discharge-enabled'           ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'val-battery-voltage-settle-time-mins'    ) AjaxSendNameValue('battery-voltage-settle-time-mins'    ,  elem.value);
    if (elem.id === 'val-battery-heater-proportional'         )
    {
        let value = elem.value * 256 * 256 / 100;
        if (value >  65535) value = 65535;
        if (value <      0) value =     0;
        AjaxSendNameValue('battery-heater-proportional', value.toFixed(0));
    }
    if (elem.id === 'val-battery-heater-integral'             )
    {
        let value = elem.value * 256 * 256 / 100 / 60;
        if (value >  65535) value = 65535;
        if (value <      0) value =     0;
        AjaxSendNameValue('battery-heater-integral'    , value.toFixed(0));
    }
}

Ajax.server     = '/battery-ajax';
Ajax.onResponse = function() { parse(); display(); };
Ajax.init();