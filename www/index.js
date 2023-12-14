//Index script
'use strict';

let batteryCountedCapacityAs = '';
let batteryCurrentMa         = '';
let batteryOutputState       = '';
let freshVolume              = '';
let lpgVolumeMl              = '';
let accelerometerX           = '';
let accelerometerY           = '';
let accelerometerZ           = '';
let outsideTemp16ths         = '';
let heatingTemp16ths         = '';
let waterPump                = false;
let waterFill                = false;
let waterDrain               = false;
let inverter                 = false;
let dplus                    = false;
let batteryMode              = '';
let litresLimit              = '';
let litresDplus              = '';
let lpgLitresMin             = '';

function parse()
{
    let lines = Ajax.response.split('\n');
    batteryCountedCapacityAs = lines[ 0];
    batteryCurrentMa         = lines[ 1];
    batteryOutputState       = lines[ 2];
    freshVolume              = lines[ 3];
    lpgVolumeMl              = lines[ 4];
    accelerometerX           = lines[ 5];
    accelerometerY           = lines[ 6];
    accelerometerZ           = lines[ 7];
    outsideTemp16ths         = lines[ 8];
    heatingTemp16ths         = lines[ 9];
    waterPump                = lines[10] === '1';
    waterFill                = lines[11] === '1';
    waterDrain               = lines[12] === '1';
    inverter                 = lines[13] === '1';
    dplus                    = lines[14] === '1';
    batteryMode              = lines[15];
    litresLimit              = lines[16];
    litresDplus              = lines[17];
    lpgLitresMin             = lines[18];
}

const CAPACITY_AH        = 280;
const AS_PER_PERCENT     = CAPACITY_AH*36;

function display()
{
    let timeToEmptySeconds  = -batteryCountedCapacityAs / batteryCurrentMa * 1000;
    let sTimeToEmptyDays    = timeToEmptySeconds >= 0 ? (timeToEmptySeconds  / 3600 / 24).toFixed(1) : '-';

    let tiltXrad = accelerometerX/16384;
    let tiltXdeg = tiltXrad * 57;          //180/pi = 57.3
    let tiltYrad = accelerometerY/16384;
    let tiltYdeg = tiltYrad * 57;          //180/pi = 57.3
    let tiltZrad = accelerometerZ/16384;
    let tiltZdeg = tiltZrad * 57;          //180/pi = 57.3

    let elem;
    elem = Ajax.getElementOrNull('txt-battery-counted-capacity-percent'); if (elem) elem.textContent = (batteryCountedCapacityAs / AS_PER_PERCENT).toFixed(1);
    elem = Ajax.getElementOrNull('met-battery-counted-capacity-percent'); if (elem) {
                                                                                    elem.value       = (batteryCountedCapacityAs / AS_PER_PERCENT).toFixed(1);
                                                                                    elem.min         = 0;
                                                                                    elem.low         = 25;
                                                                                    elem.optimum     = 60;
                                                                                    elem.high        = 90;
                                                                                    elem.max         = 100;
                                                                                    }
    elem = Ajax.getElementOrNull('txt-battery-current-ma'              ); if (elem) elem.textContent =  batteryCurrentMa;
    elem = Ajax.getElementOrNull('txt-battery-output-state'            ); if (elem) elem.textContent =  batteryOutputState;
    elem = Ajax.getElementOrNull('txt-battery-days-to-empty'           ); if (elem) elem.textContent = sTimeToEmptyDays;
    elem = Ajax.getElementOrNull('txt-fresh-volume'                    ); if (elem) elem.textContent =  freshVolume;
    elem = Ajax.getElementOrNull('met-fresh-volume'                    ); if (elem) {
                                                                                    elem.value       = freshVolume;
                                                                                    elem.min         = 0;
                                                                                    elem.low         = litresLimit;
                                                                                    elem.high        = litresDplus;
                                                                                    elem.optimum     = litresDplus + 1;
                                                                                    elem.max         = 100;
                                                                                    }
    elem = Ajax.getElementOrNull('txt-lpg-volume-litres'               ); if (elem) elem.textContent = (lpgVolumeMl / 1024).toFixed(1);
    elem = Ajax.getElementOrNull('met-lpg-volume-litres'               ); if (elem) {
                                                                                    elem.value       = (lpgVolumeMl / 1024).toFixed(1);
                                                                                    elem.min         = 0;
                                                                                    elem.low         = lpgLitresMin + 0.5;
                                                                                    elem.high        = 10;
                                                                                    elem.optimum     = 15;
                                                                                    elem.max         = 20;
                                                                                    }
    elem = Ajax.getElementOrNull('txt-tilt-right-deg'                  ); if (elem) elem.textContent =  tiltXdeg.toFixed(1);
    elem = Ajax.getElementOrNull('txt-tilt-front-deg'                  ); if (elem) elem.textContent =  tiltZdeg.toFixed(1);
    elem = Ajax.getElementOrNull('txt-outside-temperature'             ); if (elem) elem.textContent =  (outsideTemp16ths / 16).toFixed(1);
    elem = Ajax.getElementOrNull('txt-heating-temperature'             ); if (elem) elem.textContent =  (heatingTemp16ths / 16).toFixed(1);
    elem = Ajax.getElementOrNull('att-switch-water-pump'               ); if (elem) elem.setAttribute('dir', waterPump  ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-switch-water-fill'               ); if (elem) elem.setAttribute('dir', waterFill  ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-switch-water-drain'              ); if (elem) elem.setAttribute('dir', waterDrain ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-switch-inverter'                 ); if (elem) elem.setAttribute('dir', inverter   ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-switch-dplus'                    ); if (elem) elem.setAttribute('dir', dplus      ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-mode-away'                       ); if (elem) elem.setAttribute('dir', batteryMode == 1 ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-mode-home'                       ); if (elem) elem.setAttribute('dir', batteryMode == 2 ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('txt-battery-mode'                    ); if (elem) {
                                                                                    switch (batteryMode)
                                                                                        {
                                                                                            case '0': elem.textContent = "Manual"   ; break;
                                                                                            case '1': elem.textContent = "Away"     ; break;
                                                                                            case '2': elem.textContent = "Home"     ; break;
                                                                                            default : elem.textContent = batteryMode; break;
                                                                                        }
                                                                                    }
}

function change(elem)
{
}

Ajax.server     = '/index-ajax';
Ajax.onResponse = function() { parse(); display(); };
Ajax.init();