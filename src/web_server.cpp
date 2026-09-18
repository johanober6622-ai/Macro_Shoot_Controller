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
<style>#clearEndpoints{background:#f8dddd;color:#9f3030;border-color:#d87575}#saveStart{background:#dce9f8;color:#245a8d;border-color:#77a9d5}#saveEnd{background:#d9efdf;color:#267244;border-color:#6cb486}#clearEndpoints.saved,#saveStart.saved,#saveEnd.saved{background:#2f8a61;color:#fff;border-color:#2f8a61}.travelForward.active{background:#2f8a61;color:#fff;border-color:#2f8a61}.travelReverse.active{background:#c45151;color:#fff;border-color:#c45151}#cameraTest.testing{background:#f1c56b;color:#59430b;border-color:#e3b452}#cameraTest.sent{background:#2f8a61;color:#fff;border-color:#2f8a61}</style>
<title>Macro Camera Slider Controller</title>
<style>
:root{font-family:"Bahnschrift","Trebuchet MS",sans-serif;color:#173042;background:#dfe8e7;--ink:#173042;--muted:#66808a;--line:#d7e4e4;--panel:#fbfdfc;--teal:#087f78;--teal-dark:#075c5b;--amber:#e6a23c;--red:#c65151}*{box-sizing:border-box}body{margin:0;min-height:100vh;background:radial-gradient(circle at 12% 0%,#fff 0,#edf4f2 38%,#dbe6e5 100%);font-size:14px;letter-spacing:.01em}.shell{max-width:1240px;margin:auto;padding:30px 30px 40px}.topbar{align-items:flex-end;margin-bottom:24px}.brand{font-size:clamp(24px,3vw,36px);line-height:1;font-weight:800;letter-spacing:-.03em;color:var(--ink)}.brand:after{content:" / FOCUS STACKING SYSTEM";display:block;margin-top:8px;color:var(--teal);font-size:10px;font-weight:700;letter-spacing:.18em}.connection{padding:8px 12px;border:1px solid #b8d7d2;border-radius:999px;background:#eef9f5;color:var(--teal-dark);font-size:11px;font-weight:700;letter-spacing:.1em;text-transform:uppercase}.status{gap:0;background:transparent;border:1px solid #cbdcda;border-radius:14px;box-shadow:0 12px 34px #24464a18}.metric{min-height:88px;padding:17px 19px;background:rgba(251,253,252,.94);border-right:1px solid var(--line)}.metric:last-child{border-right:0}.metric .value{font-size:28px;line-height:1;font-weight:800;color:var(--ink);font-variant-numeric:tabular-nums}.metric:first-child{background:#e7f5ef}.metric:first-child .value{color:var(--teal-dark)}.caption{margin-top:9px;color:var(--muted);font-size:10px;font-weight:700;letter-spacing:.13em}.layout{gap:18px;margin-top:18px}.panel{padding:22px;background:rgba(251,253,252,.94);border:1px solid #cbdcda;border-radius:14px;box-shadow:0 12px 34px #24464a12}.panel h2{margin:0 0 18px;color:var(--ink);font-size:16px;letter-spacing:.02em}.settingsGroup{gap:12px 20px;border-top:1px solid var(--line);padding-top:16px}.settingsGroup h3{color:var(--teal-dark);font-size:10px;letter-spacing:.14em}.field{min-height:40px;color:#42606a;font-size:13px;font-weight:600}.field input,.field select{width:148px}.field input,.field select,button{padding:10px 11px;border:1px solid #bfd1d1;border-radius:8px;background:#f4f9f8;color:var(--ink);transition:border-color .18s,box-shadow .18s,background .18s,transform .18s}.field input:focus,.field select:focus{outline:0;border-color:var(--teal);box-shadow:0 0 0 3px #087f7820;background:#fff}.field input:disabled,button:disabled{opacity:.42}.actions{gap:10px}.actions button{min-height:42px;box-shadow:0 3px 0 #17304212}button{font-weight:800;letter-spacing:.01em}button:not(:disabled):hover{transform:translateY(-1px);box-shadow:0 5px 12px #1730421c}button:not(:disabled):active{transform:translateY(1px);box-shadow:none}.primary{background:var(--teal);border-color:var(--teal);color:#fff}.danger{background:var(--red);border-color:var(--red);color:#fff}.jog{background:#fff5df;border-color:#e8bd72;color:#825615}.summary{border-top-color:var(--line);margin-top:20px;padding-top:14px;color:var(--muted)}.summary b{color:var(--ink);font-size:17px}.hint{color:var(--muted);line-height:1.5}.error{font-weight:700}.footer{margin-top:22px;color:#789198;font-size:11px;text-align:right;letter-spacing:.08em;text-transform:uppercase}.travelTest{background:#f5faf8;border-radius:10px;margin-top:16px;padding:14px}.travelForward.active{background:var(--teal);border-color:var(--teal);color:#fff}.travelReverse.active{background:var(--red);border-color:var(--red);color:#fff}#cameraTest.testing{background:#f5c96b;color:#59430b;border-color:#e3b452}#cameraTest.sent{background:var(--teal);color:#fff;border-color:var(--teal)}
@media(max-width:760px){.shell{padding:20px 14px 28px}.topbar{align-items:flex-start;gap:16px;flex-direction:column}.brand{font-size:28px}.status{grid-template-columns:repeat(2,1fr)}.metric{border-right:1px solid var(--line);border-bottom:1px solid var(--line)}.metric:nth-child(2n){border-right:0}.metric:nth-last-child(-n+1){border-bottom:0}.layout{grid-template-columns:1fr}.settingsGroup{grid-template-columns:1fr}.field input,.field select{width:46%}.panel{padding:18px}}
</style>
<style>
:root{font-family:"Segoe UI",Arial,sans-serif;color:#24313a;background:#edf1f3}*{box-sizing:border-box}body{margin:0;min-height:100vh;background:linear-gradient(135deg,#f7fafb,#e5ecef);font-size:14px}.shell{max-width:1120px;margin:auto;padding:18px 22px}.topbar{display:flex;align-items:center;justify-content:space-between;margin-bottom:14px}.brand{font-size:22px;font-weight:700;letter-spacing:.2px}.connection{color:#5b6b73;font-size:12px}.panel{background:#fff;border:1px solid #d7e0e4;border-radius:10px;box-shadow:0 3px 12px #24313a12;padding:14px}.status{display:grid;grid-template-columns:1.4fr repeat(4,1fr);gap:1px;background:#d7e0e4;border-radius:8px;overflow:hidden}.metric{background:#fff;padding:12px 14px}.metric .value{font-size:25px;font-weight:700;line-height:1.1}.metric:first-child .value{color:#287a58;text-transform:capitalize}.caption{display:block;color:#788891;font-size:11px;text-transform:uppercase;letter-spacing:.6px;margin-top:5px}.layout{display:grid;grid-template-columns:1.35fr .65fr;gap:14px;margin-top:14px}.panel h2{font-size:14px;margin:0 0 12px;color:#43545d}.settings{display:grid;grid-template-columns:repeat(2,1fr);gap:9px 14px}.field{display:flex;align-items:center;justify-content:space-between;gap:10px;color:#52636b}.field input,.field select{width:130px}.field input,.field select,button{font:inherit;padding:8px 9px;border:1px solid #c6d2d7;border-radius:6px;background:#f8fafb;color:#24313a}.field input:disabled,button:disabled{cursor:not-allowed;opacity:.45}select{cursor:help}button{cursor:pointer;font-weight:600;min-height:36px}.actions{display:grid;grid-template-columns:repeat(2,1fr);gap:8px}.actions button{width:100%}.primary{background:#2f8a61;color:#fff;border-color:#2f8a61}.danger{background:#c45151;color:#fff;border-color:#c45151}.jog{background:#f1c56b;border-color:#e3b452}.summary{border-top:1px solid #e4eaed;margin-top:14px;padding-top:10px;color:#5c6c74}.summary b{color:#24313a}.error{min-height:18px;margin:10px 0 0;color:#b13d3d}.hint{color:#7a8a91;font-size:12px;margin:8px 0 0}.endpoint{margin-top:14px}.endpoint .actions{grid-template-columns:repeat(3,1fr)}.footer{color:#7a8a91;font-size:11px;text-align:right;margin-top:10px}@media(max-width:760px){.shell{padding:12px}.status{grid-template-columns:repeat(3,1fr)}.status .metric:first-child{grid-column:span 3}.layout{grid-template-columns:1fr}.settings{grid-template-columns:repeat(2,1fr)}}@media(max-width:480px){.settings{grid-template-columns:1fr}.field input,.field select{width:145px}}
</style><style>.settings{display:flex;flex-direction:column;gap:12px}.settingsGroup{display:grid;grid-template-columns:repeat(2,1fr);gap:9px 14px;border-top:1px solid #e4eaed;padding-top:10px}.settingsGroup:first-child{border-top:0;padding-top:0}.settingsGroup h3{grid-column:1/-1;font-size:12px;text-transform:uppercase;letter-spacing:.5px;color:#788891;margin:0}.field input[readonly]{background:#e9eef0;color:#43545d}</style></head><body><main class="shell">
<header class="topbar"><div class="brand">Macro Camera Slider Controller</div><div class="connection">Live Controller / Ready</div></header>
<style>
:root{font-family:"Bahnschrift","Trebuchet MS",sans-serif;color:#173042;--ink:#173042;--muted:#66808a;--line:#d7e4e4;--teal:#087f78;--teal-dark:#075c5b;--red:#c65151}body{background:radial-gradient(circle at 12% 0%,#fff 0,#edf4f2 38%,#dbe6e5 100%);letter-spacing:.01em}.shell{max-width:1240px;padding:30px 30px 40px}.topbar{align-items:flex-end;margin-bottom:24px}.brand{font-size:clamp(24px,3vw,36px);line-height:1;font-weight:800;letter-spacing:-.03em;color:var(--ink)}.brand:after{content:" / FOCUS STACKING SYSTEM";display:block;margin-top:8px;color:var(--teal);font-size:10px;font-weight:700;letter-spacing:.18em}.connection{padding:8px 12px;border:1px solid #b8d7d2;border-radius:999px;background:#eef9f5;color:var(--teal-dark);font-size:11px;font-weight:700;letter-spacing:.1em;text-transform:uppercase}.status{gap:0;background:transparent;border:1px solid #cbdcda;border-radius:14px;box-shadow:0 12px 34px #24464a18}.metric{min-height:88px;padding:17px 19px;background:rgba(251,253,252,.94);border-right:1px solid var(--line)}.metric:last-child{border-right:0}.metric .value{font-size:28px;line-height:1;font-weight:800;color:var(--ink);font-variant-numeric:tabular-nums}.metric:first-child{background:#e7f5ef}.metric:first-child .value{color:var(--teal-dark)}.caption{margin-top:9px;color:var(--muted);font-size:10px;font-weight:700;letter-spacing:.13em}.layout{gap:18px;margin-top:18px}.panel{padding:22px;background:rgba(251,253,252,.94);border:1px solid #cbdcda;border-radius:14px;box-shadow:0 12px 34px #24464a12}.panel h2{margin:0 0 18px;color:var(--ink);font-size:16px;letter-spacing:.02em}.settingsGroup{gap:12px 20px;border-top:1px solid var(--line);padding-top:16px}.settingsGroup h3{color:var(--teal-dark);font-size:10px;letter-spacing:.14em}.field{min-height:40px;color:#42606a;font-size:13px;font-weight:600}.field input,.field select{width:148px}.field input,.field select,button{padding:10px 11px;border:1px solid #bfd1d1;border-radius:8px;background:#f4f9f8;color:var(--ink);transition:border-color .18s,box-shadow .18s,background .18s,transform .18s}.field input:focus,.field select:focus{outline:0;border-color:var(--teal);box-shadow:0 0 0 3px #087f7820;background:#fff}.actions{gap:10px}.actions button{min-height:42px;box-shadow:0 3px 0 #17304212}button{font-weight:800}button:not(:disabled):hover{transform:translateY(-1px);box-shadow:0 5px 12px #1730421c}button:not(:disabled):active{transform:translateY(1px);box-shadow:none}.primary{background:var(--teal);border-color:var(--teal);color:#fff}.danger{background:var(--red);border-color:var(--red);color:#fff}.jog{background:#fff5df;border-color:#e8bd72;color:#825615}.summary{border-top-color:var(--line);margin-top:20px;padding-top:14px;color:var(--muted)}.summary b{color:var(--ink);font-size:17px}.hint{color:var(--muted);line-height:1.5}.footer{margin-top:22px;color:#789198;font-size:11px;text-align:right;letter-spacing:.08em;text-transform:uppercase}.travelTest{background:#f5faf8;border-radius:10px;margin-top:16px;padding:14px}.travelForward.active{background:var(--teal);border-color:var(--teal);color:#fff}.travelReverse.active{background:var(--red);border-color:var(--red);color:#fff}
@media(max-width:760px){.shell{padding:20px 14px 28px}.topbar{align-items:flex-start;gap:16px;flex-direction:column}.brand{font-size:28px}.status{grid-template-columns:repeat(2,1fr)}.metric{border-right:1px solid var(--line);border-bottom:1px solid var(--line)}.metric:nth-child(2n){border-right:0}.layout{grid-template-columns:1fr}.settingsGroup{grid-template-columns:1fr}.field input,.field select{width:46%}.panel{padding:18px}}
.setupLayout{grid-template-columns:1.15fr .85fr}.status{grid-template-columns:1.4fr repeat(6,1fr)}.utilityPanel .actions{grid-template-columns:repeat(2,1fr);gap:7px}.utilityPanel .actions button{min-height:32px;padding:6px 5px;font-size:11px;white-space:nowrap}.runActions{grid-column:1/-1;width:100%;max-width:100%;margin:0 auto;grid-template-columns:repeat(3,minmax(0,1fr))}#resetDefaults{background:#dce9f8;color:#245a8d;border-color:#77a9d5}.endpointGroup,.manualMovement{grid-column:1/-1}.endpointGroup .actions{width:100%;max-width:100%;margin:0 auto;grid-template-columns:repeat(3,minmax(0,1fr));justify-items:stretch}.endpointGroup .actions button{width:auto;text-align:center}.endpointSummary{display:flex;justify-content:space-between;gap:12px}.endpointSummary .hint{margin:10px 0 0}.endpointGroup .actions button,.field[data-hint]{position:relative}.field[data-hint]:has(input:disabled),.field[data-hint]:has(select:disabled){opacity:.42}.endpointGroup .actions button:hover:after,.endpointGroup .actions button:focus-visible:after,.field[data-hint]:hover:after,.field[data-hint]:focus-within:after{content:attr(data-hint);position:absolute;z-index:2;bottom:calc(100% + 8px);left:50%;width:200px;padding:8px 10px;transform:translateX(-50%);border-radius:6px;background:#173042;color:#fff;font-size:11px;font-weight:500;line-height:1.35;text-align:left;white-space:normal;box-shadow:0 3px 12px #24313a33}.panel h2,.settingsGroup h3,.endpointGroup h3{text-align:center;font-weight:800}.endpointGroup{margin-top:14px;border-top:1px solid var(--line);padding-top:14px}.caption .unit{text-transform:lowercase}
</style>
<section class="status"><div class="metric"><div id="state" class="value">Idle</div><span class="caption">State</span></div><div class="metric"><div id="capturedFrames" class="value">0</div><span class="caption">Captured Frames</span></div><div class="metric"><div id="remaining" class="value">0</div><span class="caption">Capture Frames Left</span></div><div class="metric"><div id="distance" class="value">0</div><span class="caption">Distance Moved (<span class="unit">mm</span>)</span></div><div class="metric"><div id="stepLength" class="value">0</div><span class="caption">Step Length (<span class="unit">&micro;m</span>)</span></div><div class="metric"><div id="shots" class="value">0</div><span class="caption">Total Capture Frames</span></div><div class="metric"><div id="estimatedTime" class="value">0:00</div><span class="caption">Estimated Time</span></div></section>
<div class="layout setupLayout"><section class="panel"><h2>Setup</h2><div class="settings">
<div class="settingsGroup"><h3>Capture Setup</h3><label class="field" data-hint="Stacking Distance calculates shots from total travel. Start / Stop uses the manually recorded endpoints.">Mode <select id="modeInput"><option value="distance">Stacking Distance</option><option value="steps">Start / Stop</option></select></label><label class="field" data-hint="Set the length of travel for the stack to shoot.">Stacking Distance (mm) <input id="distanceInput" type="number" min="0" max="60"></label><label class="field" data-hint="Choose the optical formula that matches your lens setup.">Optical Mode <select id="methodInput"><option value="macro">Macro Lens</option><option value="objective">Microscope Objective</option><option value="reverse">Stacked Lenses</option></select></label><label class="field" data-hint="Camera settling time after a move.">Settling Time (s) <input id="delayInput" type="number" min="0" max="3600"></label><label class="field" data-hint="Steps/&#956;m = stepper motor steps per rotation x microsteps / leadscrew travel in &#956;m.">Steps / &#956;m <input id="stepsInput" type="number" min="0.01" step="0.01"></label><label class="field">Sensor Type <select id="sensorInput"><option value="0">Full Frame</option><option value="1">APS-C</option><option value="2">Micro Four Thirds</option></select></label></div>
<div class="settingsGroup"><h3>Optical Values</h3><label class="field" data-hint="Not used in Microscope Objective mode.">Nominal Aperture <select id="apertureInput"><option value="0.7">f/0.7</option><option value="0.8">f/0.8</option><option value="1.0">f/1.0</option><option value="1.1">f/1.1</option><option value="1.2">f/1.2</option><option value="1.4">f/1.4</option><option value="1.6">f/1.6</option><option value="1.8">f/1.8</option><option value="2.0">f/2.0</option><option value="2.2">f/2.2</option><option value="2.5">f/2.5</option><option value="2.8">f/2.8</option><option value="3.2">f/3.2</option><option value="3.5">f/3.5</option><option value="4.0">f/4.0</option><option value="4.5">f/4.5</option><option value="5.0">f/5.0</option><option value="5.6" selected>f/5.6</option><option value="6.3">f/6.3</option><option value="7.1">f/7.1</option><option value="8.0">f/8.0</option><option value="9.0">f/9.0</option><option value="10.0">f/10</option><option value="11.0">f/11</option><option value="13.0">f/13</option><option value="14.0">f/14</option><option value="16.0">f/16</option><option value="18.0">f/18</option><option value="20.0">f/20</option><option value="22.0">f/22</option></select></label><label class="field" data-hint="Standard Macro Lens magnification when using a Macro Lens; otherwise the calculated magnification.">Magnification <input id="magInput" type="number" min="0.1" step="0.1"></label><label class="field" data-hint="Calculated depth of field for the current optical settings.">Depth Of Field (&#956;m) <input id="depthOfFieldInput" type="number" readonly></label><label class="field" data-hint="For a Microscope Objective: magnification divided by two times numerical aperture. For other optical modes: nominal aperture times magnification + 1.">Effective Aperture <input id="effectiveApertureInput" type="number" readonly></label><label class="field" data-hint="Fraction of the depth of field used as the step distance between captured frames.">Sequence Step Fraction of DoF <select id="stepFractionInput"><option value="0.5">0.5</option><option value="0.6">0.6</option><option value="0.7">0.7</option><option value="0.8">0.8</option><option value="0.9" selected>0.9</option></select></label><label class="field" data-hint="Calculated distance the carriage moves between captured frames.">Step Length (&#956;m) <input id="stepLengthInput" type="number" readonly></label></div>
<div class="settingsGroup"><h3>Microscope Objective</h3><label class="field objectiveField" data-hint="As per the marking on the objective lens, typically 160 mm.">Base Tube Length (mm) <input id="baseTubeInput" type="number" min="10.01" step="0.1"></label><label class="field objectiveField" data-hint="Length of the tube that the objective is mounted on; it can be longer or shorter than the base length.">Actual Tube Length (mm) <input id="actualTubeInput" type="number" min="10.01" step="0.1"></label><label class="field objectiveField" data-hint="Numerical aperture specified for the microscope objective.">Numerical Aperture (NA) <input id="naInput" type="number" min="0.01" max="1" step="0.01"></label><label class="field objectiveField" data-hint="Objective magnification marked on the microscope objective.">Objective Design Magnification <input id="objectiveMagInput" type="number" min="0.1" step="0.1"></label></div>
<div class="settingsGroup"><h3>Stacked Lenses</h3><label class="field reverseField" data-hint="The front lens is closest to the object being photographed, and its aperture is controlled.">Front Lens Focal Length (mm) <input id="frontFocalInput" type="number" min="0.1" step="0.1"></label><label class="field reverseField" data-hint="Lens mounted on the camera and stopped fully open.">Rear Lens Focal Length (mm) <input id="rearFocalInput" type="number" min="0.1" step="0.1"></label></div>
</div><div class="summary">Calculated: <b id="steps">0</b> Stepper Steps / Captured Frame</div></section>
<section class="panel"><h2>Run Controls</h2><div class="settingsGroup"><h3>Image Capture</h3><div class="actions runActions"><button id="runToggle" class="primary" onclick="toggleRun()">Start Capture Sequence</button><button id="cameraTest" onclick="cameraTest()">Camera Test</button><button id="resetDefaults" onclick="resetDefaults()">Reset to Defaults</button></div></div><div class="settingsGroup travelTest"><h3>Motor Travel Test</h3><label class="field" data-hint="Enter the distance the motor should travel for the test.">Test Distance (mm) <input id="travelTestInput" type="number" min="0.1" max="5000" step="0.1" value="8"></label><div class="actions"><button id="travelForward" class="jog travelForward" onclick="travelTest(1)">Travel Forward</button><button id="travelReverse" class="jog travelReverse" onclick="travelTest(-1)">Travel Reverse</button></div><label class="field" data-hint="After a travel test, measure the actual distance moved and enter it here to correct the Steps / &#956;m calibration.">Measured Distance (mm) <input id="measuredDistanceInput" type="number" min="0.1" step="0.1"></label><div class="actions"><button id="applyCalibration" onclick="applyCalibration()">Apply Calibration</button></div><div class="endpointGroup"><h3>Start/Stop Endpoint Set</h3><div class="actions"><button id="clearEndpoints" data-hint="Clears the saved stop and start positions. Do this before any saving." aria-label="Clear saved start and stop positions" onclick="post('/api/endpoints/clear')">Clear</button><button id="saveStart" data-hint="Saves the start position." aria-label="Save start position" onclick="post('/api/endpoints/start')">Save Start</button><button id="saveEnd" data-hint="Saves the endpoint of Start/Stop mode, then returns to the saved start position." aria-label="Save endpoint and return to saved start position" onclick="post('/api/endpoints/end')">Save End + Return</button></div><div class="endpointSummary"><p class="hint">Recorded Stepper Motor Steps: <b id="endpoint">0</b></p><p class="hint">Capture Frames: <b id="endpointFrames">0</b></p></div></div></div><div class="manualMovement utilityPanel"><h2>Manual Movement</h2><div class="actions"><button class="jog" onclick="jog(-1,'long')">Long Reverse</button><button class="jog" onclick="jog(1,'long')">Long Forward</button><button class="jog" onclick="jog(-1,'fast')">Fast Reverse</button><button class="jog" onclick="jog(1,'fast')">Fast Forward</button><button class="jog" onclick="jog(-1,'slow')">Step Reverse</button><button class="jog" onclick="jog(1,'slow')">Step Forward</button></div></div><p id="error" class="error"></p><p class="hint">Start Capture Sequence Uses The Values Shown Above. Camera Test Fires The Camera Without Moving The Motor.</p></section></div>
<div class="footer">Updates Every 500 ms</div></main>
<script>
const ids=['distanceInput','delayInput','stepsInput','magInput','apertureInput','effectiveApertureInput','baseTubeInput','actualTubeInput','objectiveMagInput','naInput','frontFocalInput','rearFocalInput','methodInput','sensorInput','modeInput','stepFractionInput'];const clearEndpoints=document.getElementById('clearEndpoints');const saveStart=document.getElementById('saveStart');const saveEnd=document.getElementById('saveEnd');const cameraTestButton=document.getElementById('cameraTest');const resetDefaultsButton=document.getElementById('resetDefaults');const travelForward=document.getElementById('travelForward');const travelReverse=document.getElementById('travelReverse');const stepLength=document.getElementById('stepLength');const endpointFrames=document.getElementById('endpointFrames');let ready=false;
let captureSequenceActive=false;
async function post(url){await fetch(url,{method:'POST'});await update();if(url==='/api/endpoints/clear'){clearEndpoints.textContent='Cleared';clearEndpoints.classList.add('saved');saveStart.textContent='Save Start';saveStart.classList.remove('saved');saveEnd.textContent='Save End + Return';saveEnd.classList.remove('saved')}else if(url==='/api/endpoints/start'){saveStart.textContent='Start Saved';saveStart.classList.add('saved');saveEnd.textContent='Save End + Return';saveEnd.classList.remove('saved')}else if(url==='/api/endpoints/end'){saveEnd.textContent='End Saved';saveEnd.classList.add('saved')}}
async function cameraTest(){cameraTestButton.disabled=true;cameraTestButton.textContent='Testing Camera...';cameraTestButton.className='testing';await fetch('/api/camera/test',{method:'POST'});cameraTestButton.textContent='Camera Test Sent';cameraTestButton.className='sent';setTimeout(()=>{cameraTestButton.disabled=false;cameraTestButton.textContent='Camera Test';cameraTestButton.className=''},1500);await update()}
async function resetDefaults(){await fetch('/api/settings/reset',{method:'POST'});ready=false;await update()}
async function jog(direction,speed){await fetch(`/api/jog?direction=${direction}&speed=${speed}`,{method:'POST'});await update()}
async function travelTest(direction){const distance=travelTestInput.value;travelForward.classList.toggle('active',direction>0);travelReverse.classList.toggle('active',direction<0);await fetch(`/api/travel-test?direction=${direction}&distance=${distance}`,{method:'POST'});await update()}
async function applyCalibration(){const measured=measuredDistanceInput.value;await fetch(`/api/calibrate?measured=${measured}`,{method:'POST'});ready=false;await update()}
async function toggleRun(){await post(captureSequenceActive?'/api/run/stop':'/api/run/start')}
function updateModeControls(){const stepsMode=modeInput.value==='steps';const opticalMode=methodInput.value;distanceInput.disabled=stepsMode;clearEndpoints.disabled=!stepsMode;saveStart.disabled=!stepsMode;saveEnd.disabled=!stepsMode;apertureInput.disabled=opticalMode==='objective';magInput.disabled=opticalMode!=='macro';document.querySelectorAll('.objectiveField').forEach(e=>e.querySelector('input').disabled=opticalMode!=='objective');document.querySelectorAll('.reverseField').forEach(e=>e.querySelector('input').disabled=opticalMode!=='reverse')}
async function update(){const r=await fetch('/api/state');const d=await r.json();
 state.textContent=d.state;capturedFrames.textContent=d.capturedFrames;remaining.textContent=d.remainingShots;distance.textContent=(d.distanceTravelled/1000).toFixed(2);stepLength.textContent=d.stepDistance.toFixed(0);steps.textContent=d.stepsPerShot;shots.textContent=d.totalShots;estimatedTime.textContent=`${Math.floor(d.estimatedSequenceSeconds/60)}:${String(d.estimatedSequenceSeconds%60).padStart(2,'0')}`;endpoint.textContent=d.endpointSteps;endpointFrames.textContent=d.totalShots;error.textContent=d.error||'';magInput.value=d.magnification;depthOfFieldInput.value=d.depthOfField;effectiveApertureInput.value=d.effectiveAperture.toFixed(2);stepLengthInput.value=d.stepDistance.toFixed(0);
 captureSequenceActive=d.captureSequenceActive;runToggle.textContent=captureSequenceActive?'Stop Capture Sequence':'Start Capture Sequence';runToggle.className=captureSequenceActive?'danger':'primary';runToggle.disabled=!captureSequenceActive&&d.state!=='idle';cameraTestButton.disabled=d.state!=='idle';resetDefaultsButton.disabled=d.state!=='idle';if(!captureSequenceActive){travelForward.classList.remove('active');travelReverse.classList.remove('active')}
 if(!ready){distanceInput.value=d.shootDistance;delayInput.value=d.delaySeconds;stepsInput.value=d.stepsPerMicron;apertureInput.value=d.aperture;baseTubeInput.value=d.objectiveBaseTubeLength;actualTubeInput.value=d.objectiveActualTubeLength;objectiveMagInput.value=d.objectiveDesignMagnification;naInput.value=d.numericalAperture;frontFocalInput.value=d.reverseFrontFocalLength;rearFocalInput.value=d.reverseRearFocalLength;methodInput.value=d.opticalMode;sensorInput.value=d.sensorType;modeInput.value=d.stepsMode?'steps':'distance';stepFractionInput.value=d.stepFraction;ready=true}updateModeControls()}
function changed(){const q=new URLSearchParams({distance:distanceInput.value,delay:delayInput.value,stepsPerMicron:stepsInput.value,magnification:magInput.value,aperture:apertureInput.value,baseTube:baseTubeInput.value,actualTube:actualTubeInput.value,objectiveMag:objectiveMagInput.value,na:naInput.value,frontFocal:frontFocalInput.value,rearFocal:rearFocalInput.value,method:methodInput.value,sensor:sensorInput.value,mode:modeInput.value,stepFraction:stepFractionInput.value});fetch('/api/settings?'+q,{method:'POST'}).then(update)}
ids.forEach(id=>document.getElementById(id).addEventListener('change',changed));update();setInterval(update,500);
</script></body></html>
)rawliteral";

String jsonState()
{
    const ControllerSettings &settings = controllerSettings();
    const ControllerStatus &status = controllerStatus();
    String json = "{";
    json += "\"state\":\"" + String(controllerStateName()) + "\",";
    json += "\"captureSequenceActive\":" + String(status.captureSequenceActive ? "true" : "false") + ",";
    json += "\"capturedFrames\":" + String(status.capturedFrames) + ",";
    json += "\"depthOfField\":" + String(settings.depthOfField) + ",";
    json += "\"stepFraction\":" + String(settings.stepFraction, 1) + ",";
    json += "\"stepDistance\":" + String(settings.stepDistance, 0) + ",";
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
    json += "\"estimatedSequenceSeconds\":" + String(settings.estimatedSequenceSeconds) + ",";
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
    if (server.hasArg("stepFraction")) controllerSetStepFraction(server.arg("stepFraction").toFloat());
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

    server.on("/", HTTP_GET, []() {
        server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
        server.sendHeader("Pragma", "no-cache");
        server.send_P(200, "text/html", INDEX_HTML);
    });
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
    server.on("/api/settings/reset", HTTP_POST, []() { controllerResetToDefaults(); sendOk(); });
    server.on("/api/run/start", HTTP_POST, []() { controllerStartRun(); sendOk(); });
    server.on("/api/run/stop", HTTP_POST, []() { controllerStop(); sendOk(); });
    server.on("/api/camera/test", HTTP_POST, []() { controllerCameraTest(); sendOk(); });
    server.on("/api/endpoints/clear", HTTP_POST, []() { controllerClearEndpoints(); sendOk(); });
    server.on("/api/endpoints/start", HTTP_POST, []() { controllerSaveEndpointStart(); sendOk(); });
    server.on("/api/endpoints/end", HTTP_POST, []() { controllerSaveEndpointEnd(); sendOk(); });
    server.on("/api/jog", HTTP_POST, []() { controllerStartManualMove(server.arg("direction").toInt(), server.arg("speed").c_str()); sendOk(); });
    server.on("/api/travel-test", HTTP_POST, []() { controllerStartTravelTest(server.arg("direction").toInt(), server.arg("distance").toFloat()); sendOk(); });
    server.on("/api/calibrate", HTTP_POST, []() { controllerCalibrateStepsPerMicron(server.arg("measured").toFloat()); sendOk(); });
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

bool webServerIsAccessPoint()
{
    return accessPointMode;
}

bool webServerHasClient()
{
    return accessPointMode ? WiFi.softAPgetStationNum() > 0 : WiFi.status() == WL_CONNECTED;
}

String webServerApSsid()
{
    return String(WIFI_AP_SSID);
}

String webServerApPassword()
{
    return String(WIFI_AP_PASSWORD);
}
