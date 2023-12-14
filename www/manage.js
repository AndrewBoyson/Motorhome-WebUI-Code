//Manage script
'use strict';

let batteryCurrentMa              = '';
let batteryOutputState            = '';
let batteryMode                   = '';
let batteryAwayPercent            = '';
let batteryHomePercent            = '';
let batteryRestMa                 = '';
let batterySecondsAtRest          = '';
let plotRestSeconds               = '';
let plotDirection                 = '';
let plotIncPercent                = '';
let plotMaxPercent                = '';
let plotMinPercent                = '';
let calAs                         = '';
let calMv                         = '';
let calAsPerMv                    = '';
let calTime                       = '';
let calMinAs                      = '';
let okToCalibrate                 = '';

function parse()
{
    let lines = Ajax.response.split('\n');
    batteryCurrentMa              = lines[ 0];
    batteryOutputState            = lines[ 1];
    batteryMode                   = lines[ 2];
    batteryAwayPercent            = lines[ 3];
    batteryHomePercent            = lines[ 4];
    batteryRestMa                 = lines[ 5];
    batterySecondsAtRest          = lines[ 6];
    plotRestSeconds               = lines[ 7];
    plotDirection                 = lines[ 8];
    plotIncPercent                = lines[ 9];
    plotMaxPercent                = lines[10];
    plotMinPercent                = lines[11];
    calAs                         = lines[12];
    calMv                         = lines[13];
    calAsPerMv                    = lines[14];
    calTime                       = lines[15];
    calMinAs                      = lines[16];
    okToCalibrate                 = lines[17] === '1';
}

const CAPACITY_AH        = 280;
const AS_PER_PULSE       = 61.444;
const AS_PER_PERCENT     = CAPACITY_AH*36;
const PULSES_PER_PERCENT = AS_PER_PERCENT/AS_PER_PULSE;

function display()
{
    //let targetAs = batteryCapacitySetpoint * AS_PER_PERCENT;
    //let capacityToTargetAs = targetAs - batteryCountedCapacityAs;

    //let timeToEmptySeconds  = -batteryCountedCapacityAs / batteryCurrentMa * 1000;
    //let sTimeToEmptySeconds = timeToEmptySeconds >= 0 ? timeToEmptySeconds.toFixed(0) : '-';
    //let sTimeToEmptyDays    = timeToEmptySeconds >= 0 ? (timeToEmptySeconds  / 3600 / 24).toFixed(1) : '-';

    //let timeToTargetSeconds  = capacityToTargetAs / batteryCurrentMa * 1000;
    //let sTimeToTargetSeconds = timeToTargetSeconds >= 0 ? timeToTargetSeconds.toFixed(0) : '-';
    //let sTimeToTargetHours   = timeToTargetSeconds >= 0 ? (timeToTargetSeconds / 3600).toFixed(1) : '-';

    let timeToNow = AS_PER_PULSE / Math.abs(batteryCurrentMa) * 1000;
    let timeTo100mA = AS_PER_PULSE / batteryRestMa * 1000 - timeToNow;
    let sTimeTo100mA = batteryOutputState == 'N' && timeTo100mA >= 0 ? timeTo100mA.toFixed(0) : '-';

    let calDt = new Date(calTime*1000);

    let elem;
    elem = Ajax.getElementOrNull('txt-battery-current-ma'                   ); if (elem) elem.textContent =  batteryCurrentMa;
    elem = Ajax.getElementOrNull('txt-battery-output-state'                 ); if (elem) elem.textContent =  batteryOutputState;
    elem = Ajax.getElementOrNull('att-mode-away'                            ); if (elem) elem.setAttribute('dir', batteryMode == 1 ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-mode-home'                            ); if (elem) elem.setAttribute('dir', batteryMode == 2 ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('val-battery-away-percent'                 ); if (elem) elem.value       =  batteryAwayPercent;
    elem = Ajax.getElementOrNull('val-battery-home-percent'                 ); if (elem) elem.value       =  batteryHomePercent;
    elem = Ajax.getElementOrNull('val-battery-rest-ma'                      ); if (elem) elem.value       =  batteryRestMa;
    elem = Ajax.getElementOrNull('txt-plot-rest-remaining'                  ); if (elem) elem.textContent =  batterySecondsAtRest;
    elem = Ajax.getElementOrNull('txt-plot-time-to-rest'                    ); if (elem) elem.textContent = sTimeTo100mA;
    elem = Ajax.getElementOrNull('att-plot-up'                              ); if (elem) elem.setAttribute('dir', plotDirection == +1 ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-plot-down'                            ); if (elem) elem.setAttribute('dir', plotDirection == -1 ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('val-plot-rest-seconds'                    ); if (elem) elem.value       =  plotRestSeconds;
    elem = Ajax.getElementOrNull('val-plot-inc-percent'                     ); if (elem) elem.value       =  plotIncPercent;
    elem = Ajax.getElementOrNull('val-plot-max-percent'                     ); if (elem) elem.value       =  plotMaxPercent;
    elem = Ajax.getElementOrNull('val-plot-min-percent'                     ); if (elem) elem.value       =  plotMinPercent;
    elem = Ajax.getElementOrNull('val-cal-as'                               ); if (elem) elem.value       =  calAs;
    elem = Ajax.getElementOrNull('val-cal-percent'                          ); if (elem) elem.value       = (calAs / AS_PER_PERCENT).toFixed(1);
    elem = Ajax.getElementOrNull('val-cal-mv'                               ); if (elem) elem.value       =  calMv;
    elem = Ajax.getElementOrNull('val-cal-as-per-mv'                        ); if (elem) elem.value       =  calAsPerMv;
    elem = Ajax.getElementOrNull('val-cal-mv-per-percent'                   ); if (elem) elem.value       = (AS_PER_PERCENT / calAsPerMv).toFixed(1);
    elem = Ajax.getElementOrNull('val-cal-time'                             ); if (elem) elem.value       =  calDt.toISOString().slice(0, -8).replace ('T', ' ');
    elem = Ajax.getElementOrNull('val-cal-min-as'                           ); if (elem) elem.value       =  calMinAs;
    elem = Ajax.getElementOrNull('val-cal-min-percent'                      ); if (elem) elem.value       = (calMinAs / AS_PER_PERCENT).toFixed(1);
    elem = Ajax.getElementOrNull('att-cal-ok-to-start'                      ); if (elem) elem.setAttribute('dir', okToCalibrate ? 'rtl' : 'ltr');

}

function change(elem)
{   
    if (elem.id === 'att-mode-away'                           ) AjaxSendNameValue('battery-mode'                        ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'att-mode-home'                           ) AjaxSendNameValue('battery-mode'                        ,  elem.dir == 'rtl' ? '0' :  '2');
    if (elem.id === 'val-battery-away-percent'                ) AjaxSendNameValue('battery-away-percent'                ,  elem.value);
    if (elem.id === 'val-battery-home-percent'                ) AjaxSendNameValue('battery-home-percent'                ,  elem.value);
    if (elem.id === 'val-battery-rest-ma'                     ) AjaxSendNameValue('battery-rest-ma'                     ,  elem.value);
    if (elem.id === 'val-plot-rest-seconds'                   ) AjaxSendNameValue('plot-rest-seconds'                   ,  elem.value);
    if (elem.id === 'att-plot-up'                             ) AjaxSendNameValue('plot-direction'                      ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'att-plot-down'                           ) AjaxSendNameValue('plot-direction'                      ,  elem.dir == 'rtl' ? '0' : '-1');
    if (elem.id === 'val-plot-inc-percent'                    ) AjaxSendNameValue('plot-inc-percent'                    ,  elem.value);
    if (elem.id === 'val-plot-max-percent'                    ) AjaxSendNameValue('plot-max-percent'                    ,  elem.value);
    if (elem.id === 'val-plot-min-percent'                    ) AjaxSendNameValue('plot-min-percent'                    ,  elem.value);
    if (elem.id === 'val-cal-as'                              ) AjaxSendNameValue('battery-cal-as'                      ,  elem.value);
    if (elem.id === 'val-cal-percent'                         ) AjaxSendNameValue('battery-cal-as'                      , (elem.value * AS_PER_PERCENT).toFixed(0));
    if (elem.id === 'val-cal-mv'                              ) AjaxSendNameValue('battery-cal-mv'                      ,  elem.value);
    if (elem.id === 'val-cal-as-per-mv'                       ) AjaxSendNameValue('battery-cal-as-per-mv'               ,  elem.value);
    if (elem.id === 'val-cal-mv-per-percent'                  ) AjaxSendNameValue('battery-cal-as-per-mv'               , (AS_PER_PERCENT / elem.value).toFixed(0));
    if (elem.id === 'val-cal-time'                            ) AjaxSendNameValue('battery-cal-time'                    ,  Date.parse(elem.value + 'Z') / 1000);
    if (elem.id === 'val-cal-min-as'                          ) AjaxSendNameValue('battery-cal-min-as'                  ,  elem.value);
    if (elem.id === 'val-cal-min-percent'                     ) AjaxSendNameValue('battery-cal-min-as'                  , (elem.value * AS_PER_PERCENT).toFixed(0));
}

Ajax.server     = '/manage-ajax';
Ajax.onResponse = function() { parse(); display(); };
Ajax.init();