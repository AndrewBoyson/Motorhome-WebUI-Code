//Control script
'use strict';

let linTrace                    = false;

function parse()
{
    let lines = Ajax.response.split('\n');
    linTrace  = lines[0] === '1';
}


function display()
{
    let elem;
    elem = Ajax.getElementOrNull('att-lin-trace'                        ); if (elem) elem.setAttribute('dir', linTrace  ? 'rtl' : 'ltr');
}

function change(elem)
{
    if (elem.id === 'att-lin-trace'       ) AjaxSendNameValue('lin-trace'       ,  elem.dir == 'rtl' ? '0' :  '1');
}

Ajax.server     = '/heating-ajax';
Ajax.onResponse = function() { parse(); display(); };
Ajax.init();