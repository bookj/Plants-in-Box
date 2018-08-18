const char MAIN_page[] PROGMEM = R"=====(
    <html>
        <head>
            <title>Device Configuration</title>
        </head>
        <body>
            <center><b>Device Configuration</b></center>
            <center><b>Activation Method : </b></center>
            <form action="/activation" method="post">
                <input type="radio" name="method" value="abp" checked>
                ABP
                <input type="radio" name="method" value="otaa">
                OTAA    
                <br><input type="submit" value="Next">
            </form>
        </body>
    </html>
)=====";

const char ABP_page[] PROGMEM = R"=====(
    <html>
        <head>
            <title>Device Configuration</title>
            <script>
                function isEmpty(_devAddr, _nwkSKey, _appSKey) {
                    var text = "";
                    if(_devAddr.length == 0) {
                        text += "Device Address is empty\n";
                    }
                    if(_nwkSKey.length == 0) {
                        text += "Network Session Key is empty\n";
                    }
                    if(_appSKey.length == 0) {
                        text += "App Session Key is empty\n";
                    }
                    return text;
                }

                function isHex(value) {
                    var i = 0;
                    for(i = 0; i < value.length; i++) {
                        // A => 65
                        // F => 70
                        // a => 97
                        // f => 102
                        // 0 => 48
                        // 9 => 57
                        if(((value.charCodeAt(i) >= 65) && (value.charCodeAt(i) <= 70)) || 
                        ((value.charCodeAt(i) >= 97) && (value.charCodeAt(i) <= 102)) || 
                        ((value.charCodeAt(i) >= 48) && (value.charCodeAt(i) <= 57))) {
                            return true;
                        }
                        else {
                            return false;
                        }
                    }
                }

                function validateLengthABP(_devAddr, _nwkSKey, _appSKey) {
                    // BYTE_devAddr_HEX 4*2
                    // BYTE_nwkSKey_HEX 16*2
                    // BYTE_appSKey_HEX 16*2

                    var text = "";

                    if(_devAddr.length != 8) {
                        text += "Device Address must be 8 characters\n";
                    }
                    if(_nwkSKey.length != 32) {
                        text += "Network Session Key must be 32 characters\n";
                    }
                    if(_appSKey.length != 32) {
                        text += "App Session Key must be 32 characters\n";
                    }
                    return text;
                }

                function validateHexABP(_devAddr, _nwkSKey, _appSKey) {
                    var text = "";
                    if(!isHex(_devAddr)) {
                        text += "Device Address must be hexadecimal\n";
                    }
                    if(!isHex(_nwkSKey)) {
                        text += "Network Session Key must be hexadecimal\n";
                    }
                    if(!isHex(_appSKey)) {
                        text += "App Session Key must be hexadecimal\n";
                    }
                    return text;
                }

                function validateForm() {
                    var _devAddr, _nwkSKey, _appSKey;
                    
                    _devAddr = document.forms["formABP"]["devAddr"].value;
                    _nwkSKey = document.forms["formABP"]["nwkSKey"].value;
                    _appSKey = document.forms["formABP"]["appSKey"].value;

                    if(isEmpty(_devAddr, _nwkSKey, _appSKey).length != 0 ) {
                        alert(isEmpty(_devAddr, _nwkSKey, _appSKey));
                        return false;
                    }
                    else if(validateLengthABP(_devAddr, _nwkSKey, _appSKey).length != 0) {
                        alert(validateLengthABP(_devAddr, _nwkSKey, _appSKey));
                        return false;
                    }
                    else if(validateHexABP(_devAddr, _nwkSKey, _appSKey).length != 0) {
                        alert(validateHexABP(_devAddr, _nwkSKey, _appSKey));
                        return false;
                    }
                    else {
                        alert("Successfully ...");
                        return true;
                    }
                }

                function goBack() {
                    window.history.back();
                }

            </script>
        </head>
        <body>
            <center><b>Device Configuration</b></center>
            <center><b>Activation Method : ABP</b></center>
            <form name="formABP" action="/post" onsubmit="return validateForm()" method="post">
                Device Address
                <input type="text" name="devAddr"><br>
                Network Session Key
                <input type="text" name="nwkSKey"><br>
                App Session Key    
                <input type="text" name="appSKey"><br>

                <input type="submit" value="Submit">
            </form>
            <button onclick="goBack()">Go Back</button>
        </body>
    </html>
)=====";

const char OTAA_page[] PROGMEM = R"=====(
    <html>
        <head>
            <title>Device Configuration</title>
            <script>
                function isEmpty(_devEui, _appEui, _appKey) {
                    var text = "";
                    if(_devEui.length == 0) {
                        text += "Device EUI is empty\n";
                    }
                    if(_appEui.length == 0) {
                        text += "Application EUI is empty\n";
                    }
                    if(_appKey.length == 0) {
                        text += "App Key Key is empty\n";
                    }
                    return text;
                }

                function isHex(value) {
                    var i = 0;
                    for(i = 0; i < value.length; i++) {
                        // A => 65
                        // F => 70
                        // a => 97
                        // f => 102
                        // 0 => 48
                        // 9 => 57
                        if(((value.charCodeAt(i) >= 65) && (value.charCodeAt(i) <= 70)) || 
                        ((value.charCodeAt(i) >= 97) && (value.charCodeAt(i) <= 102)) || 
                        ((value.charCodeAt(i) >= 48) && (value.charCodeAt(i) <= 57))) {
                            return true;
                        }
                        else {
                            return false;
                        }
                    }
                }

                function validateLengthOTAA(_devEui, _appEui, _appKey) {
                    // BYTE_devEui_HEX 8*2
                    // BYTE_appEui_HEX 8*2
                    // BYTE_appKey_HEX 16*2

                    var text = "";
                    if(_devEui.length != 16) {
                        text += "Device EUI must be 16 characters\n";
                    }
                    if(_appEui.length != 16) {
                        text += "Application EUI must be 16 characters\n";
                    }
                    if(_appKey.length != 32) {
                        text += "App Key must be 32 characters\n";
                    }
                    return text;
                }

                function validateHexOTAA(_devEui, _appEui, _appKey) {
                    var text = "";
                    if(!isHex(_devEui)) {
                        text += "Device EUI must be hexadecimal\n";
                    }
                    if(!isHex(_appEui)) {
                        text += "Application EUI must be hexadecimal\n";
                    }
                    if(!isHex(_appKey)) {
                        text += "App Key must be hexadecimal\n";
                    }
                    return text;
                }

                function validateFormOTAA() {
                    var _devEui, _appEui, _appKey;
                    
                    _devEui = document.forms["formOTAA"]["devEui"].value;
                    _appEui = document.forms["formOTAA"]["appEui"].value;
                    _appKey = document.forms["formOTAA"]["appKey"].value;

                    if(isEmpty(_devEui, _appEui, _appKey).length != 0 ) {
                        alert(isEmpty(_devEui, _appEui, _appKey));
                        return false;
                    }
                    else if(validateLengthOTAA(_devEui, _appEui, _appKey).length != 0) {
                        alert(validateLengthOTAA(_devEui, _appEui, _appKey));
                        return false;
                    }
                    else if(validateHexOTAA(_devEui, _appEui, _appKey).length != 0) {
                        alert(validateHexOTAA(_devEui, _appEui, _appKey));
                        return false;
                    }
                    else {
                        alert("Successfully ...");
                        return true;
                    }
                }

                function goBack() {
                    window.history.back();
                }

            </script>
        </head>
        <body>
            <center><b>Device Configuration</b></center>
            <center><b>Activation Method : OTAA</b></center>
            <form name="formOTAA" action="/post" onsubmit="return validateFormOTAA()" method="post">
                Device EUI
                <input type="text" name="devEui"><br>
                Application EUI
                <input type="text" name="appEui"><br>
                App Key    
                <input type="text" name="appKey"><br>

                
                <input type="submit" value="Submit">
            </form>
            <button onclick="goBack()">Go Back</button>
        </body>
    </html>
)=====";