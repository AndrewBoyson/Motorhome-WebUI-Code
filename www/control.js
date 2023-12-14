//Control script
'use strict';

let waterPump                = false;
let waterFill                = false;
let waterDrain               = false;
let inverter                 = false;
let dplus                    = false;
let litres                   = '';
let pumpMinLitres            = '';
let pumpDplusLitres          = '';
let drainMaxLitres           = '';

function parse()
{
    let lines = Ajax.response.split('\n');
    waterPump                = lines[0] === '1';
    waterFill                = lines[1] === '1';
    waterDrain               = lines[2] === '1';
    inverter                 = lines[3] === '1';
    dplus                    = lines[4] === '1';
    litres                   = lines[5];
    pumpMinLitres            = lines[6];
    pumpDplusLitres          = lines[7];
    drainMaxLitres           = lines[8];
}

const CAPACITY_AH        = 280;
const AS_PER_PERCENT     = CAPACITY_AH*36;

function display()
{
    let elem;
    elem = Ajax.getElementOrNull('att-control-water-pump'               ); if (elem) elem.setAttribute('dir', waterPump  ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-control-water-fill'               ); if (elem) elem.setAttribute('dir', waterFill  ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-control-water-drain'              ); if (elem) elem.setAttribute('dir', waterDrain ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-control-inverter'                 ); if (elem) elem.setAttribute('dir', inverter   ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-control-dplus'                    ); if (elem) elem.setAttribute('dir', dplus      ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('val-control-pump-min-litres'          ); if (elem) elem.value       =  pumpMinLitres;
    elem = Ajax.getElementOrNull('val-control-pump-dplus-litres'        ); if (elem) elem.value       =  pumpDplusLitres;
    elem = Ajax.getElementOrNull('val-control-drain-max-litres'         ); if (elem) elem.value       =  drainMaxLitres;
    elem = Ajax.getElementOrNull('txt-control-litres'                   ); if (elem) elem.textContent =  litres;
}

function change(elem)
{
    if (elem.id === 'att-control-water-pump'       ) AjaxSendNameValue('control-water-pump'       ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'att-control-water-fill'       ) AjaxSendNameValue('control-water-fill'       ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'att-control-water-drain'      ) AjaxSendNameValue('control-water-drain'      ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'att-control-inverter'         ) AjaxSendNameValue('control-inverter'         ,  elem.dir == 'rtl' ? '0' :  '1');
    if (elem.id === 'val-control-pump-min-litres'  ) AjaxSendNameValue('control-pump-min-litres'  ,  elem.value);
    if (elem.id === 'val-control-pump-dplus-litres') AjaxSendNameValue('control-pump-dplus-litres',  elem.value);
    if (elem.id === 'val-control-drain-max-litres' ) AjaxSendNameValue('control-drain-max-litres' ,  elem.value);
}

Ajax.server     = '/control-ajax';
Ajax.onResponse = function() { parse(); display(); };
Ajax.init();