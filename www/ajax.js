//Ajax class
'use strict';

//Exposed properties
let ajaxResponse_   = '';
let ajaxHeaders_    = '';
let ajaxDate_       = null;
let ajaxMs_         =  0;
let ajaxOnResponse_ = null;
let ajaxOnTick_     = null;
let ajaxServer_     = '';

//Private variables
let   ajaxRequestIsFromUpdate_        = false;
let   ajaxXhr_                        =  null;
let   ajaxMsCountAtAjaxSend_          =     0;
const ajaxTickMs_                     =   100;
const ajaxQuickUpdateMs_              =   500;
const ajaxUpdateMs_                   =  2000;
let   ajaxDoQuickUpdate_              = false;

//Private utilities
function ajaxGetElementOrNull_(elementName) //Returns the element if it: exists; block is overidden; does not have focus
{
    let elem = document.getElementById(elementName);
    if (!elem) return null;
    //Block the element if it is an input and has focus and we haven't just updated the value
    if (elementName.slice(0, 3) === "val" && elem === document.activeElement && !ajaxRequestIsFromUpdate_) return null;
    return elem;
}
function ajaxHexToBit_(text, iBit)
{
   let value = parseInt(text, 16);
   value >>= iBit;
   return value & 1;
}
function ajaxHexToSignedInt8_(text)
{
    let value = parseInt(text, 16);
    if (value < 0x80) return value;
    return value - 0x100;
}
function ajaxHexToSignedInt16_(text)
{
    let value = parseInt(text, 16);
    if (value < 0x8000) return value;
    return value - 0x10000;
}
function ajaxHexToSignedInt32_(text)
{
    let value = parseInt(text, 16);
    if (value < 0x80000000) return value;
    return value - 0x100000000;
}


//Private ajax functions
function ajaxHandleAjaxResponse_()
{
   if (ajaxXhr_.readyState == 4 && ajaxXhr_.status == 200)
   {
        ajaxResponse_ = ajaxXhr_.responseText;
        ajaxHeaders_  = ajaxXhr_.getAllResponseHeaders();
        let iDateStart = Ajax.headers.toLowerCase().indexOf('date:');
        let iDateEnd   = Ajax.headers.indexOf('\r', iDateStart);
        ajaxDate_      = new Date(Ajax.headers.slice(iDateStart + 5, iDateEnd));

        let elem;
        elem = ajaxGetElementOrNull_('ajax-response'   ); if (elem) elem.textContent = ajaxResponse_;
        elem = ajaxGetElementOrNull_('ajax-headers'    ); if (elem) elem.textContent = ajaxHeaders_;
        elem = ajaxGetElementOrNull_('ajax-date-local' );
        if (elem)
        {
            elem.textContent = ajaxDate_.toLocaleString(    undefined, {  weekday     : 'short'  ,
                                                                          day         : '2-digit',
                                                                          month       : 'short'  ,
                                                                          year        : 'numeric',
                                                                          hour        : '2-digit',
                                                                          minute      : '2-digit',
                                                                          timeZoneName: 'short'
                                                                       }
                                                       );
        }
        if (ajaxOnResponse_) ajaxOnResponse_();
        ajaxRequestIsFromUpdate_ = false; //Received response so reset request is from update
   }
}
function ajaxSendNameValue_(name, value) //Used by this script and from HTML page
{
    ajaxXhr_ = new XMLHttpRequest();
    ajaxXhr_.onreadystatechange = ajaxHandleAjaxResponse_;
    if (name)
    {
        if (value)
        {
            ajaxXhr_.open('GET', ajaxServer_ + '?' + name + '=' + encodeURIComponent(value), true);
        }
        else
        {
            ajaxXhr_.open('GET', ajaxServer_ + '?' + name, true);
        }
    }
    else
    {
        ajaxXhr_.open('GET', ajaxServer_, true);
    }
    ajaxXhr_.send();
    ajaxDoQuickUpdate_     = false;
    ajaxMsCountAtAjaxSend_ = ajaxMs_;
}
function AjaxSendNameValue(name, value) //From html
{
    ajaxRequestIsFromUpdate_        = true; //Request has come from an update
    ajaxSendNameValue_(name, value);
    ajaxDoQuickUpdate_              = true;
}

//Private functions
function ajaxTick_() //Called about every 100ms
{
    ajaxMs_ += ajaxTickMs_; //Don't use Date.now() as we don't know when the PC's clock will be updated around a leap second
    if (ajaxMs_ >= ajaxMsCountAtAjaxSend_ + ajaxUpdateMs_) ajaxSendNameValue_('', '');
    if (ajaxDoQuickUpdate_)
    {
        if (ajaxMs_ >= ajaxMsCountAtAjaxSend_ + ajaxQuickUpdateMs_) ajaxSendNameValue_('', '');
    }
    if (ajaxOnTick_) ajaxOnTick_();
}
function ajaxInit_()
{
    setInterval(ajaxTick_, ajaxTickMs_);
    ajaxSendNameValue_('', '');
}

//Exposed public
class Ajax
{
    static get ms        ()  { return ajaxMs_         ; }
    static get response  ()  { return ajaxResponse_   ; }
    static get headers   ()  { return ajaxHeaders_    ; }
    static get date      ()  { return ajaxDate_       ; }
    
    static set tickMs    (v) { ajaxTickMs_     = v; }
    static set updateMs  (v) { ajaxUpdateMs_   = v; }
    static set server    (v) { ajaxServer_     = v; }
    static set onResponse(v) { ajaxOnResponse_ = v; }
    static set onTick    (v) { ajaxOnTick_     = v; }

    static getElementOrNull(elementName) { return ajaxGetElementOrNull_(elementName) ; }
    static hexToBit        (text, iBit ) { return ajaxHexToBit_        (text, iBit ) ; }
    static hexToSignedInt8 (text       ) { return ajaxHexToSignedInt8_ (text       ) ; }
    static hexToSignedInt16(text       ) { return ajaxHexToSignedInt16_(text       ) ; }
    static hexToSignedInt32(text       ) { return ajaxHexToSignedInt32_(text       ) ; }
    
    static init()
    {
        if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', ajaxInit_ ); // Loading hasn't finished yet
        else                                                                                 ajaxInit_(); //`DOMContentLoaded` has already fired
    }
}