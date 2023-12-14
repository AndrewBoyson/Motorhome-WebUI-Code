//Samples script
'use strict';

let usbDriveIsPluggedIn         = false;
let usbDriveIsMounted           = false;
let usbDriveSizeBytes           = '';
let usbDriveFreeBytes           = '';
let usbDriveLabel               = '';
let appList                     = '';
let usbList                     = '';

function parseLines(text)
{
    let lines = text.split('\n');
    usbDriveIsPluggedIn         = lines[0] === '1';
    usbDriveIsMounted           = lines[1] === '1';
    usbDriveSizeBytes           = lines[2];
    usbDriveFreeBytes           = lines[3];
    usbDriveLabel               = lines[4];
}

function parse()
{
    let topics = Ajax.response.split('\f');
    parseLines   (topics[0]);
    appList     = topics[1];
    usbList     = topics[2];
}
function display()
{
    let elem;
    elem = Ajax.getElementOrNull('att-usb-drive-plugged-in'); if (elem) elem.setAttribute('dir', usbDriveIsPluggedIn   ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('att-usb-drive-mounted'   ); if (elem) elem.setAttribute('dir', usbDriveIsMounted     ? 'rtl' : 'ltr');
    elem = Ajax.getElementOrNull('txt-usb-size'            ); if (elem) elem.textContent = (usbDriveSizeBytes / 1000000).toFixed(0);
    elem = Ajax.getElementOrNull('txt-usb-free'            ); if (elem) elem.textContent = (usbDriveFreeBytes / 1000000).toFixed(0);
    elem = Ajax.getElementOrNull('txt-usb-used'            ); if (elem) elem.textContent = ((usbDriveSizeBytes - usbDriveFreeBytes) / 1000000).toFixed(0);
    elem = Ajax.getElementOrNull('txt-usb-label'           ); if (elem) elem.textContent = usbDriveLabel;
    elem = Ajax.getElementOrNull('txt-app-list'            ); if (elem) elem.textContent = appList;
    elem = Ajax.getElementOrNull('txt-usb-list'            ); if (elem) elem.textContent = usbList;
}

Ajax.server     = '/backup-ajax';
Ajax.onResponse = function() { parse(); display(); };
Ajax.init();