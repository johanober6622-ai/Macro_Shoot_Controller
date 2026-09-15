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
</style></head><body><main class="shell">
<header class="topbar"><div class="brand">MacroController</div><div class="connection">Live controller</div></header>
<section class="status"><div class="metric"><div id="state" class="value">idle</div><span class="caption">state</span></div><div class="metric"><div id="remaining" class="value">0</div><span class="caption">shots left</span></div><div class="metric"><div id="distance" class="value">0</div><span class="caption">distance</span></div><div class="metric"><div id="dof" class="value">0</div><span class="caption">DoF</span></div><div class="metric"><div id="shots" class="value">0</div><span class="caption">total shots</span></div></section>
<div class="layout"><section class="panel"><h2>Setup</h2><div class="settings">
<label class="field">Distance <input id="distanceInput" type="number" min="0" max="5000"></label>
<label class="field">Delay (s) <input id="delayInput" type="number" min="0" max="3600"></label>
<label class="field">Steps / micron <input id="stepsInput" type="number" min="0.01" step="0.01"></label>
<label class="field">Magnification <input id="magInput" type="number" min="0.1" step="0.1"></label>
<label class="field">F-stop <input id="fstopInput" type="number" min="0.1" step="0.1"></label>
<label class="field">Objective NA <select id="naInput" title="Numerical aperture used for microscope-objective depth-of-field calculation."><option>0.10</option><option>0.14</option><option>0.20</option><option>0.25</option><option>0.30</option><option>0.40</option></select></label>
<label class="field">Method <select id="methodInput" title="Choose Lens for magnification and f-stop, or Objective for numerical-aperture calculation."><option value="lens">Lens</option><option value="objective">Objective</option></select></label>
<label class="field">Mode <select id="modeInput" title="Distance calculates shots from total travel; Start / stop uses the manually recorded endpoints."><option value="distance">Distance</option><option value="steps">Start / stop</option></select></label>
</div><div class="summary">Calculated: <b id="steps">0</b> steps/shot</div></section>
<section class="panel"><h2>Run controls</h2><div class="actions"><button id="runToggle" class="primary" onclick="toggleRun()">Start / Stop</button><button onclick="post('/api/camera/test')">Camera test</button></div><p id="error" class="error"></p><p class="hint">Start uses the values shown above. The same button stops an active run.</p></section></div>
<div class="layout"><section class="panel"><h2>Manual movement</h2><div class="actions"><button class="jog" onclick="jog(-1,'long')">Long reverse</button><button class="jog" onclick="jog(1,'long')">Long forward</button><button class="jog" onclick="jog(-1,'fast')">Fast reverse</button><button class="jog" onclick="jog(1,'fast')">Fast forward</button><button class="jog" onclick="jog(-1,'slow')">Step reverse</button><button class="jog" onclick="jog(1,'slow')">Step forward</button></div></section><section class="panel endpoint"><h2>Endpoints</h2><div class="actions"><button id="clearEndpoints" onclick="post('/api/endpoints/clear')">Clear</button><button id="saveStart" onclick="post('/api/endpoints/start')">Save start</button><button id="saveEnd" onclick="post('/api/endpoints/end')">Save end + return</button></div><p class="hint">Recorded steps: <b id="endpoint">0</b></p></section></div><div class="footer">Updates every 500 ms</div></main>
<script>
const ids=['distanceInput','delayInput','stepsInput','magInput','fstopInput','naInput','methodInput','modeInput'];const clearEndpoints=document.getElementById('clearEndpoints');const saveStart=document.getElementById('saveStart');const saveEnd=document.getElementById('saveEnd');let ready=false;
const busyStates=['manual_move','trigger','camera_wait','moving','settling','returning','stopping'];
async function post(url){await fetch(url,{method:'POST'});await update();if(url==='/api/endpoints/clear'){clearEndpoints.textContent='Cleared';clearEndpoints.classList.add('saved');saveStart.textContent='Save start';saveStart.classList.remove('saved');saveEnd.textContent='Save end + return';saveEnd.classList.remove('saved')}else if(url==='/api/endpoints/start'){saveStart.textContent='Start saved';saveStart.classList.add('saved');saveEnd.textContent='Save end + return';saveEnd.classList.remove('saved')}else if(url==='/api/endpoints/end'){saveEnd.textContent='End saved';saveEnd.classList.add('saved')}}
async function jog(direction,speed){await fetch(`/api/jog?direction=${direction}&speed=${speed}`,{method:'POST'});await update()}
async function toggleRun(){const active=busyStates.includes(state.textContent);await post(active?'/api/run/stop':'/api/run/start')}
function updateModeControls(){const stepsMode=modeInput.value==='steps';distanceInput.disabled=stepsMode;clearEndpoints.disabled=!stepsMode;saveStart.disabled=!stepsMode;saveEnd.disabled=!stepsMode}
async function update(){const r=await fetch('/api/state');const d=await r.json();
 state.textContent=d.state;remaining.textContent=d.remainingShots;distance.textContent=d.distanceTravelled;dof.textContent=d.depthOfField;steps.textContent=d.stepsPerShot;shots.textContent=d.totalShots;endpoint.textContent=d.endpointSteps;error.textContent=d.error||'';
 const active=busyStates.includes(d.state);runToggle.textContent=active?'Stop':'Start';runToggle.className=active?'danger':'primary';
 if(!ready){distanceInput.value=d.shootDistance;delayInput.value=d.delaySeconds;stepsInput.value=d.stepsPerMicron;magInput.value=d.magnification;fstopInput.value=d.fStop;naInput.value=d.numericalAperture.toFixed(2);methodInput.value=d.objectiveMode?'objective':'lens';modeInput.value=d.stepsMode?'steps':'distance';ready=true}updateModeControls()}
function changed(){const q=new URLSearchParams({distance:distanceInput.value,delay:delayInput.value,stepsPerMicron:stepsInput.value,magnification:magInput.value,fStop:fstopInput.value,na:naInput.value,method:methodInput.value,mode:modeInput.value});fetch('/api/settings?'+q,{method:'POST'}).then(update)}
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
    json += "\"fStop\":" + String(settings.fStop, 1) + ",";
    json += "\"numericalAperture\":" + String(settings.numericalAperture, 2) + ",";
    json += "\"totalShots\":" + String(settings.totalShots) + ",";
    json += "\"currentShot\":" + String(status.currentShot) + ",";
    json += "\"remainingShots\":" + String(status.remainingShots) + ",";
    json += "\"distanceTravelled\":" + String(status.distanceTravelled) + ",";
    json += "\"endpointSteps\":" + String(status.endpointSteps) + ",";
    json += "\"stepsMode\":" + String(settings.stepsMode ? "true" : "false") + ",";
    json += "\"objectiveMode\":" + String(settings.objectiveMode ? "true" : "false") + ",";
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
    if (server.hasArg("fStop")) controllerSetFStop(server.arg("fStop").toFloat());
    if (server.hasArg("na")) controllerSetNumericalAperture(server.arg("na").toFloat());
    if (server.hasArg("method")) controllerSetObjectiveMode(server.arg("method") == "objective");
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
