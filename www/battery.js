//Battery script
'use strict';

let batteryCountedCapacityAs      = '';
let batteryCurrentMa              = '';
let batteryCapacitySetpoint       = '';
let batteryOutputState            = '';
let batteryChargeEnabled          = false;
let batteryDischargeEnabled       = false;
let batteryTemperature8bfdp       = '';
let batteryTemperatureSetPoint    = '';
let batteryHeater8bfdp            = '';
let batteryVoltageMv              = '';
let batteryAgingAsPerHour         = '';
let batteryHeaterProportional     = '';
let batteryHeaterIntegral         = '';

function parse()
{
    let lines = Ajax.response.split('\n');
    batteryCountedCapacityAs      = lines[ 0];
    batteryCurrentMa              = lines[ 1];
    batteryCapacitySetpoint       = lines[ 2];
    batteryOutputState            = lines[ 3];
    batteryChargeEnabled          = lines[ 4] === '1';
    batteryDischargeEnabled       = lines[ 5] === '1';
    batteryTemperature8bfdp       = lines[ 6];
    batteryTemperatureSetPoint    = lines[ 7];
    batteryHeater8bfdp            = lines[ 8];
    batteryVoltageMv              = lines[ 9];
    batteryAgingAsPerHour         = lines[10];
    batteryHeaterProportional     = lines[11];
    batteryHeaterIntegral         = lines[12];
}

const CAPACITY_AH        = 280;
const AS_PER_PULSE       = 61.444;
const AS_PER_PERCENT     = CAPACITY_AH*36;
const PULSES_PER_PERCENT = AS_PER_PERCENT/AS_PER_PULSE;

function display()
{
    let targetAs = batteryCapacitySetpoint * AS_PER_PERCENT;
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
    elem = Ajax.getElementOrNull('val-battery-capacity-setpoint-percent'    ); if (elem) elem.value       =  batteryCapacitySetpoint;
    elem = Ajax.getElementOrNull('txt-battery-output-state'                 ); if (elem) elem.textContent =  batteryOutputState;
    elem = Ajax.getElementOrNull('att-battery-charge-enabled'               ); if (elem) elem.setAttribute('dir', batteryChargeEnabled    ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-battery-discharge-enabled'            ); if (elem) elem.setAttribute('dir', batteryDischargeEnabled ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('txt-battery-temperature'                  ); if (elem) elem.textContent = (batteryTemperature8bfdp/256).toFixed(1);
    elem = Ajax.getElementOrNull('val-battery-temperature-setpoint'         ); if (elem) elem.value       = (batteryTemperatureSetPoint/10).toFixed(1);
    elem = Ajax.getElementOrNull('txt-battery-heater-output-percent'        ); if (elem) elem.textContent =  (batteryHeater8bfdp * 100 / 256).toFixed(1);
    elem = Ajax.getElementOrNull('txt-battery-heater-output-ma'             ); if (elem) elem.textContent =  heaterMa.toFixed(0);
    elem = Ajax.getElementOrNull('txt-battery-heater-output-watts'          ); if (elem) elem.textContent =  heaterWatts.toFixed(1);
    elem = Ajax.getElementOrNull('txt-battery-voltage-mv'                   ); if (elem) elem.textContent =  batteryVoltageMv;
    elem = Ajax.getElementOrNull('txt-battery-cell-voltage-mv'              ); if (elem) elem.textContent = (batteryVoltageMv/4).toFixed(0);
    elem = Ajax.getElementOrNull('txt-battery-hours-to-target'              ); if (elem) elem.textContent = sTimeToTargetHours;
    elem = Ajax.getElementOrNull('txt-battery-seconds-to-target'            ); if (elem) elem.textContent = sTimeToTargetSeconds;
    elem = Ajax.getElementOrNull('txt-battery-days-to-empty'                ); if (elem) elem.textContent = sTimeToEmptyDays;
    elem = Ajax.getElementOrNull('val-battery-uv-per-ma'                    ); if (elem) elem.value       =  batteryUvPerMa;
    elem = Ajax.getElementOrNull('val-battery-aging-as-per-hour'            ); if (elem) elem.value       =  batteryAgingAsPerHour;
    elem = Ajax.getElementOrNull('txt-battery-aging-ma'                     ); if (elem) elem.textContent = (batteryAgingAsPerHour * 1000 / 3600).toFixed(0);
    elem = Ajax.getElementOrNull('val-battery-aging-percent-per-month'      ); if (elem) elem.value       = (batteryAgingAsPerHour * 24 * 30 / AS_PER_PERCENT).toFixed(1);
    elem = Ajax.getElementOrNull('val-battery-heater-proportional'          ); if (elem) elem.value       = (batteryHeaterProportional / 256 / 256 * 100     ).toFixed(0);
    elem = Ajax.getElementOrNull('val-battery-heater-integral'              ); if (elem) elem.value       = (batteryHeaterIntegral     / 256 / 256 * 100 * 60).toFixed(0);
    elem = Ajax.getElementOrNull('txt-battery-heater-ti-hours'              ); if (elem) elem.textContent = (TiMinutes / 60).toFixed(2);
}

function change(elem)
{   
    if (elem.id === 'val-battery-counted-capacity-amp-seconds') AjaxSendNameValue('battery-counted-capacity-amp-seconds',  elem.value);
    if (elem.id === 'val-battery-counted-capacity-percent'    ) AjaxSendNameValue('battery-counted-capacity-amp-seconds', (elem.value * AS_PER_PERCENT).toFixed(0));
    if (elem.id === 'val-battery-aging-as-per-hour'           ) AjaxSendNameValue('battery-aging-as-per-hour'           ,  elem.value);
    if (elem.id === 'val-battery-aging-percent-per-month'     ) AjaxSendNameValue('battery-aging-as-per-hour'           , (elem.value / 24 /30 * AS_PER_PERCENT).toFixed(0));
    if (elem.id === 'val-battery-temperature-setpoint'        ) AjaxSendNameValue('battery-temperature-target-tenths'   , (elem.value * 10).toFixed(0));
    if (elem.id === 'val-battery-capacity-setpoint-percent'   ) AjaxSendNameValue('battery-capacity-setpoint-percent'   ,  elem.value);
    if (elem.id === 'att-battery-charge-enabled'              ) AjaxSendNameValue('battery-charge-enabled'              ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'att-battery-discharge-enabled'           ) AjaxSendNameValue('battery-discharge-enabled'           ,  elem.dir == 'rtl' ? '0' :  '1');
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