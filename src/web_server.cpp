#include <WiFi.h>
#include <WebServer.h>
#include "controller.h"
#include "secrets.h"
#include "web_server.h"

namespace {
WebServer server(80);
String address = "offline";
bool accessPointMode = false;

bool connectStation(unsigned long timeoutMs)
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long started = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - started < timeoutMs)
    {
        delay(100);
    }
    return WiFi.status() == WL_CONNECTED;
}

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<style>#clearEndpoints{background:#f8dddd;color:#9f3030;border-color:#d87575}#saveStart{background:#dce9f8;color:#245a8d;border-color:#77a9d5}#saveEnd{background:#d9efdf;color:#267244;border-color:#6cb486}#clearEndpoints.saved,#saveStart.saved,#saveEnd.saved{background:#2f8a61;color:#fff;border-color:#2f8a61}</style>
<title>MacroController</title>
<style>
:root{font-family:"Segoe UI",Arial,sans-serif;color:#24313a;background:#edf1f3}*{box-sizing:border-box}body{margin:0;min-height:100vh;background:linear-gradient(135deg,#f7fafb,#e5ecef);font-size:14px}.shell{max-width:1120px;margin:auto;padding:18px 22px}.topbar{display:flex;align-items:center;justify-content:space-between;margin-bottom:14px}.brand{font-size:22px;font-weight:700;letter-spacing:.2px}.connection{color:#5b6b73;font-size:12px}.panel{background:#fff;border:1px solid #d7e0e4;border-radius:10px;box-shadow:0 3px 12px #24313a12;padding:14px}.status{display:grid;grid-template-columns:1.4fr repeat(4,1fr);gap:1px;background:#d7e0e4;border-radius:8px;overflow:hidden}.metric{background:#fff;padding:12px 14px}.metric .value{font-size:25px;font-weight:700;line-height:1.1}.metric:first-child .value{color:#287a58;text-transform:capitalize}.caption{display:block;color:#788891;font-size:11px;text-transform:uppercase;letter-spacing:.6px;margin-top:5px}.layout{display:grid;grid-template-columns:1.35fr .65fr;gap:14px;margin-top:14px}.panel h2{font-size:14px;margin:0 0 12px;color:#43545d}.settings{display:grid;grid-template-columns:repeat(2,1fr);gap:9px 14px}.field{display:flex;align-items:center;justify-content:space-between;gap:10px;color:#52636b}.field input,.field select{width:130px}.field input,.field select,button{font:inherit;padding:8px 9px;border:1px solid #c6d2d7;border-radius:6px;background:#f8fafb;color:#24313a}.field input:disabled,button:disabled{cursor:not-allowed;opacity:.45}select{cursor:help}button{cursor:pointer;font-weight:600;min-height:36px}.actions{display:grid;grid-template-columns:repeat(2,1fr);gap:8px}.actions button{width:100%}.primary{background:#2f8a61;color:#fff;border-color:#2f8a61}.danger{background:#c45151;color:#fff;border-color:#c45151}.jog{background:#f1c56b;border-color:#e3b452}.summary{border-top:1px solid #e4eaed;margin-top:14px;padding-top:10px;color:#5c6c74}.summary b{color:#24313a}.error{min-height:18px;margin:10px 0 0;color:#b13d3d}.hint{color:#7a8a91;font-size:12px;margin:8px 0 0}.endpoint{margin-top:14px}.endpoint .actions{grid-template-columns:repeat(3,1fr)}.footer{color:#7a8a91;font-size:11px;text-align:right;margin-top:10px}@media(max-width:760px){.shell{padding:12px}.status{grid-template-columns:repeat(3,1fr)}.status .metric:first-child{grid-column:span 3}.layout{grid-template-columns:1fr}.settings{grid-template-columns:repeat(2,1fr)}}@media(max-width:480px){.settings{grid-template-columns:1fr}.field input,.field select{width:145px}}
</style><style>.settings{display:flex;flex-direction:column;gap:12px}.settingsGroup{display:grid;grid-template-columns:repeat(2,1fr);gap:9px 14px;border-top:1px solid #e4eaed;padding-top:10px}.settingsGroup:first-child{border-top:0;padding-top:0}.settingsGroup h3{grid-column:1/-1;font-size:12px;text-transform:uppercase;letter-spacing:.5px;color:#788891;margin:0}.field input[readonly]{background:#e9eef0;color:#43545d}</style></head><body><main class="shell">
<header class="topbar"><div class="brand">MacroController</div><div class="connection">Live controller</div></header>
<section class="status"><div class="metric"><div id="state" class="value">idle</div><span class="caption">state</span></div><div class="metric"><div id="remaining" class="value">0</div><span class="caption">shots left</span></div><div class="metric"><div id="distance" class="value">0</div><span class="caption">distance (mm)</span></div><div class="metric"><div id="dof" class="value">0</div><span class="caption">DoF (um)</span></div><div class="metric"><div id="shots" class="value">0</div><span class="caption">total shots</span></div></section>
<div class="layout"><section class="panel"><h2>Setup</h2><div class="settings">
<div class="settingsGroup"><h3>Capture setup</h3><label class="field">Mode <select id="modeInput" title="Distance calculates shots from total travel; Start / stop uses the manually recorded endpoints."><option value="distance">Distance</option><option value="steps">Start / stop</option></select></label><label class="field">Distance <input id="distanceInput" type="number" min="0" max="5000"></label><label class="field">Optical mode <select id="methodInput"><option value="macro">Macro Lens</option><option value="objective">Objective Lens</option><option value="reverse">Reverse Lens</option></select></label><label class="field">Delay (s) <input id="delayInput" type="number" min="0" max="3600"></label><label class="field">Steps / micrometer <input id="stepsInput" type="number" min="0.01" step="0.01"></label><label class="field">Sensor type <select id="sensorInput"><option value="0">Full Frame</option><option value="1">APS-C</option><option value="2">Micro Four Thirds</option></select></label></div>
<div class="settingsGroup"><h3>Optical values</h3><label class="field">Nominal aperture <input id="apertureInput" type="number" min="0.1" step="0.1"></label><label class="field">Magnification <input id="magInput" type="number" min="0.1" step="0.1"></label><label class="field">Effective aperture <input id="effectiveApertureInput" type="number" readonly></label></div>
<div class="settingsGroup"><h3>Objective lens</h3><label class="field">Base tube length (mm) <input id="baseTubeInput" type="number" min="10.01" step="0.1"></label><label class="field">Actual tube length (mm) <input id="actualTubeInput" type="number" min="10.01" step="0.1"></label><label class="field">Objective NA <input id="naInput" type="number" min="0.01" max="1" step="0.01"></label><label class="field">Objective design magnification <input id="objectiveMagInput" type="number" min="0.1" step="0.1"></label></div>
<div class="settingsGroup"><h3>Reverse lens</h3><label class="field">Front lens focal length (mm) <input id="frontFocalInput" type="number" min="0.1" step="0.1"></label><label class="field">Rear lens focal length (mm) <input id="rearFocalInput" type="number" min="0.1" step="0.1"></label></div>
</div><div class="summary">Calculated: <b id="steps">0</b> steps/shot</div></section>
<section class="panel"><h2>Run controls</h2><div class="actions"><button id="runToggle" class="primary" onclick="toggleRun()">Start / Stop</button><button onclick="post('/api/camera/test')">Camera test</button></div><p id="error" class="error"></p><p class="hint">Start uses the values shown above. The same button stops an active run.</p></section></div>
<div class="layout"><section class="panel"><h2>Manual movement</h2><div class="actions"><button class="jog" onclick="jog(-1,'long')">Long reverse</button><button class="jog" onclick="jog(1,'long')">Long forward</button><button class="jog" onclick="jog(-1,'fast')">Fast reverse</button><button class="jog" onclick="jog(1,'fast')">Fast forward</button><button class="jog" onclick="jog(-1,'slow')">Step reverse</button><button class="jog" onclick="jog(1,'slow')">Step forward</button></div></section><section class="panel endpoint"><h2>Endpoints</h2><div class="actions"><button id="clearEndpoints" onclick="post('/api/endpoints/clear')">Clear</button><button id="saveStart" onclick="post('/api/endpoints/start')">Save start</button><button id="saveEnd" onclick="post('/api/endpoints/end')">Save end + return</button></div><p class="hint">Recorded steps: <b id="endpoint">0</b></p></section></div><div class="footer">Updates every 500 ms</div></main>
<script>
const ids=['distanceInput','delayInput','stepsInput','magInput','apertureInput','effectiveApertureInput','baseTubeInput','actualTubeInput','objectiveMagInput','naInput','frontFocalInput','rearFocalInput','methodInput','sensorInput','modeInput'];const clearEndpoints=document.getElementById('clearEndpoints');const saveStart=document.getElementById('saveStart');const saveEnd=document.getElementById('saveEnd');let ready=false;
const busyStates=['manual_move','trigger','camera_wait','moving','settling','returning','stopping'];
async function post(url){await fetch(url,{method:'POST'});await update();if(url==='/api/endpoints/clear'){clearEndpoints.textContent='Cleared';clearEndpoints.classList.add('saved');saveStart.textContent='Save start';saveStart.classList.remove('saved');saveEnd.textContent='Save end + return';saveEnd.classList.remove('saved')}else if(url==='/api/endpoints/start'){saveStart.textContent='Start saved';saveStart.classList.add('saved');saveEnd.textContent='Save end + return';saveEnd.classList.remove('saved')}else if(url==='/api/endpoints/end'){saveEnd.textContent='End saved';saveEnd.classList.add('saved')}}
async function jog(direction,speed){await fetch(`/api/jog?direction=${direction}&speed=${speed}`,{method:'POST'});await update()}
async function toggleRun(){const active=busyStates.includes(state.textContent);await post(active?'/api/run/stop':'/api/run/start')}
function updateModeControls(){const stepsMode=modeInput.value==='steps';const opticalMode=methodInput.value;distanceInput.disabled=stepsMode;clearEndpoints.disabled=!stepsMode;saveStart.disabled=!stepsMode;saveEnd.disabled=!stepsMode;magInput.disabled=opticalMode!=='macro';document.querySelectorAll('.objectiveField').forEach(e=>e.querySelector('input').disabled=opticalMode!=='objective');document.querySelectorAll('.reverseField').forEach(e=>e.querySelector('input').disabled=opticalMode!=='reverse')}
async function update(){const r=await fetch('/api/state');const d=await r.json();
 state.textContent=d.state;remaining.textContent=d.remainingShots;distance.textContent=(d.distanceTravelled/1000).toFixed(2);dof.textContent=d.depthOfField;steps.textContent=d.stepsPerShot;shots.textContent=d.totalShots;endpoint.textContent=d.endpointSteps;error.textContent=d.error||'';magInput.value=d.magnification;effectiveApertureInput.value=d.effectiveAperture.toFixed(2);
 const active=busyStates.includes(d.state);runToggle.textContent=active?'Stop':'Start';runToggle.className=active?'danger':'primary';
 if(!ready){distanceInput.value=d.shootDistance;delayInput.value=d.delaySeconds;stepsInput.value=d.stepsPerMicron;apertureInput.value=d.aperture;baseTubeInput.value=d.objectiveBaseTubeLength;actualTubeInput.value=d.objectiveActualTubeLength;objectiveMagInput.value=d.objectiveDesignMagnification;naInput.value=d.numericalAperture;frontFocalInput.value=d.reverseFrontFocalLength;rearFocalInput.value=d.reverseRearFocalLength;methodInput.value=d.opticalMode;sensorInput.value=d.sensorType;modeInput.value=d.stepsMode?'steps':'distance';ready=true}updateModeControls()}
function changed(){const q=new URLSearchParams({distance:distanceInput.value,delay:delayInput.value,stepsPerMicron:stepsInput.value,magnification:magInput.value,aperture:apertureInput.value,baseTube:baseTubeInput.value,actualTube:actualTubeInput.value,objectiveMag:objectiveMagInput.value,na:naInput.value,frontFocal:frontFocalInput.value,rearFocal:rearFocalInput.value,method:methodInput.value,sensor:sensorInput.value,mode:modeInput.value});fetch('/api/settings?'+q,{method:'POST'}).then(update)}
ids.forEach(id=>document.getElementById(id).addEventListener('change',changed));update();setInterval(update,500);
</script></body></html>
)rawliteral";

String jsonState()
{
    const ControllerSettings &settings = controllerSettings();
    const ControllerStatus &status = controllerStatus();
    String json = "{";
    json += "\"state\":\"" + String(controllerStateName()) + "\",";
    json += "\"depthOfField\":" + String(settings.depthOfField) + ",";
    json += "\"stepsPerShot\":" + String(settings.stepsPerShot) + ",";
    json += "\"shootDistance\":" + String(settings.shootDistance) + ",";
    json += "\"delaySeconds\":" + String(settings.delaySeconds) + ",";
    json += "\"stepsPerMicron\":" + String(settings.stepsPerMicron, 2) + ",";
    json += "\"magnification\":" + String(settings.magnification, 1) + ",";
    json += "\"aperture\":" + String(settings.aperture, 1) + ",";
    json += "\"effectiveAperture\":" + String(settings.effectiveAperture, 2) + ",";
    json += "\"numericalAperture\":" + String(settings.numericalAperture, 2) + ",";
    json += "\"objectiveBaseTubeLength\":" + String(settings.objectiveBaseTubeLength, 1) + ",";
    json += "\"objectiveActualTubeLength\":" + String(settings.objectiveActualTubeLength, 1) + ",";
    json += "\"objectiveDesignMagnification\":" + String(settings.objectiveDesignMagnification, 1) + ",";
    json += "\"reverseFrontFocalLength\":" + String(settings.reverseFrontFocalLength, 1) + ",";
    json += "\"reverseRearFocalLength\":" + String(settings.reverseRearFocalLength, 1) + ",";
    json += "\"totalShots\":" + String(settings.totalShots) + ",";
    json += "\"currentShot\":" + String(status.currentShot) + ",";
    json += "\"remainingShots\":" + String(status.remainingShots) + ",";
    json += "\"distanceTravelled\":" + String(status.distanceTravelled) + ",";
    json += "\"endpointSteps\":" + String(status.endpointSteps) + ",";
    json += "\"stepsMode\":" + String(settings.stepsMode ? "true" : "false") + ",";
    json += "\"opticalMode\":\"" + String(settings.opticalMode == OPTICAL_MACRO_LENS ? "macro" : settings.opticalMode == OPTICAL_OBJECTIVE_LENS ? "objective" : "reverse") + "\",";
    json += "\"sensorType\":" + String(settings.sensorType) + ",";
    json += "\"error\":\"" + status.error + "\"}";
    return json;
}

void handleNotFound()
{
    if (accessPointMode)
    {
        server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
        server.send(302, "text/plain", "Redirecting to MacroController");
        return;
    }
    server.send(404, "text/plain", "Not found");
}

void sendOk()
{
    server.send(200, "application/json", jsonState());
}

void applySettings()
{
    if (server.hasArg("distance")) controllerSetDistance(server.arg("distance").toInt());
    if (server.hasArg("delay")) controllerSetDelay(server.arg("delay").toInt());
    if (server.hasArg("stepsPerMicron")) controllerSetStepsPerMicron(server.arg("stepsPerMicron").toFloat());
    if (server.hasArg("magnification")) controllerSetMagnification(server.arg("magnification").toFloat());
    if (server.hasArg("aperture")) controllerSetAperture(server.arg("aperture").toFloat());
    if (server.hasArg("na")) controllerSetNumericalAperture(server.arg("na").toFloat());
    if (server.hasArg("baseTube")) controllerSetObjectiveBaseTubeLength(server.arg("baseTube").toFloat());
    if (server.hasArg("actualTube")) controllerSetObjectiveActualTubeLength(server.arg("actualTube").toFloat());
    if (server.hasArg("objectiveMag")) controllerSetObjectiveDesignMagnification(server.arg("objectiveMag").toFloat());
    if (server.hasArg("frontFocal")) controllerSetReverseFrontFocalLength(server.arg("frontFocal").toFloat());
    if (server.hasArg("rearFocal")) controllerSetReverseRearFocalLength(server.arg("rearFocal").toFloat());
    if (server.hasArg("method"))
    {
        OpticalMode mode = OPTICAL_MACRO_LENS;
        if (server.arg("method") == "objective") mode = OPTICAL_OBJECTIVE_LENS;
        if (server.arg("method") == "reverse") mode = OPTICAL_REVERSE_LENS;
        controllerSetOpticalMode(mode);
    }
    if (server.hasArg("sensor")) controllerSetSensorType(server.arg("sensor").toInt());
    if (server.hasArg("mode")) controllerSetStepsMode(server.arg("mode") == "steps");
    sendOk();
}
}

void webServerBegin()
{
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false);
    WiFi.disconnect(true, true);
    delay(100);
    WiFi.mode(WIFI_STA);
    bool connected = false;

    Serial.println("Wi-Fi: connecting to " + String(WIFI_SSID));
    if (!WIFI_FORCE_AP && String(WIFI_SSID) != "YOUR_WIFI_NAME")
    {
#if WIFI_USE_STATIC_IP
        IPAddress ip, gateway, subnet, dns1, dns2;
        ip.fromString(WIFI_STATIC_IP);
        gateway.fromString(WIFI_GATEWAY);
        subnet.fromString(WIFI_SUBNET);
        dns1.fromString(WIFI_DNS_PRIMARY);
        dns2.fromString(WIFI_DNS_SECONDARY);
        if (!WiFi.config(ip, gateway, subnet, dns1, dns2))
        {
            Serial.println("Wi-Fi: static IP configuration failed");
        }
#endif
        connected = connectStation(15000);
        if (!connected)
        {
            Serial.println("Wi-Fi static attempt failed, retrying with DHCP");
            Serial.println("Wi-Fi status: " + String(WiFi.status()));
            WiFi.disconnect(true, false);
            delay(100);
            WiFi.config(0U, 0U, 0U);
            connected = connectStation(20000);
        }
    }

    if (connected)
    {
        address = WiFi.localIP().toString();
        Serial.println("Wi-Fi connected: " + address);
        Serial.println("Wi-Fi gateway: " + WiFi.gatewayIP().toString());
        Serial.println("Wi-Fi subnet: " + WiFi.subnetMask().toString());
        Serial.println("Wi-Fi RSSI: " + String(WiFi.RSSI()));
    }
    else
    {
        Serial.println("Wi-Fi station connection failed, starting fallback AP");
        WiFi.disconnect(true, true);
        delay(100);
        WiFi.mode(WIFI_AP);
        IPAddress apIp(192, 168, 4, 1);
        IPAddress apGateway(192, 168, 4, 1);
        IPAddress apSubnet(255, 255, 255, 0);
        if (!WiFi.softAPConfig(apIp, apGateway, apSubnet))
        {
            Serial.println("Fallback AP: IP configuration failed");
        }
        accessPointMode = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, 1, false, 4);
        delay(200);
        address = WiFi.softAPIP().toString();
        Serial.println("Fallback AP: " + String(WIFI_AP_SSID));
        Serial.println("Fallback AP result: " + String(accessPointMode ? "started" : "failed"));
        Serial.println("Open: http://" + address + "/");
    }

    server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", INDEX_HTML); });
    server.on("/generate_204", HTTP_GET, []() { server.sendHeader("Location", "/", true); server.send(302, "text/plain", "Redirecting"); });
    server.on("/hotspot-detect.html", HTTP_GET, []() { server.sendHeader("Location", "/", true); server.send(302, "text/plain", "Redirecting"); });
    server.on("/fwlink", HTTP_GET, []() { server.sendHeader("Location", "/", true); server.send(302, "text/plain", "Redirecting"); });
    server.on("/api/state", HTTP_GET, []() { sendOk(); });
    server.on("/api/diagnostics", HTTP_GET, []() {
        String response = "mode=" + String(accessPointMode ? "AP" : "STA") + "\n";
        response += "status=" + String(WiFi.status()) + "\n";
        response += "local_ip=" + WiFi.localIP().toString() + "\n";
        response += "ap_ip=" + WiFi.softAPIP().toString() + "\n";
        response += "ap_clients=" + String(WiFi.softAPgetStationNum()) + "\n";
        server.send(200, "text/plain", response);
    });
    server.on("/api/settings", HTTP_POST, []() { applySettings(); });
    server.on("/api/run/start", HTTP_POST, []() { controllerStartRun(); sendOk(); });
    server.on("/api/run/stop", HTTP_POST, []() { controllerStop(); sendOk(); });
    server.on("/api/camera/test", HTTP_POST, []() { controllerCameraTest(); sendOk(); });
    server.on("/api/endpoints/clear", HTTP_POST, []() { controllerClearEndpoints(); sendOk(); });
    server.on("/api/endpoints/start", HTTP_POST, []() { controllerSaveEndpointStart(); sendOk(); });
    server.on("/api/endpoints/end", HTTP_POST, []() { controllerSaveEndpointEnd(); sendOk(); });
    server.on("/api/jog", HTTP_POST, []() { controllerStartManualMove(server.arg("direction").toInt(), server.arg("speed").c_str()); sendOk(); });
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("Web server started: http://" + address + "/");
}

void webServerTick()
{
    server.handleClient();
}

String webServerAddress()
{
    return address;
}
