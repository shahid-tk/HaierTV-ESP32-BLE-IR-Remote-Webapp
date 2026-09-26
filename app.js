const SERVICE_UUID="7b7e0001-1234-4567-89ab-123456789000";
const COMMAND_UUID="7b7e0002-1234-4567-89ab-123456789000";
const DEVICE_NAME="M5-IR-Blaster";

let device=null;
let commandCharacteristic=null;

const $=id=>document.getElementById(id);
const logEl=$("log");
const connectionText=$("connectionText");
const connectBtn=$("connectBtn");

function log(message){
  const now=new Date().toLocaleTimeString();
  logEl.textContent=`[${now}] ${message}\n`+logEl.textContent;
}
function setConnected(value){
  connectionText.textContent=value?"Connected":"Disconnected";
  connectionText.className=`status ${value?"connected":"disconnected"}`;
  connectBtn.textContent=value?"Disconnect":"Connect BLE";
}
async function connectBLE(){
  if(!navigator.bluetooth) throw new Error("Web Bluetooth is not supported by this browser.");
  device=await navigator.bluetooth.requestDevice({
    filters:[{name:DEVICE_NAME}],
    optionalServices:[SERVICE_UUID]
  });
  device.addEventListener("gattserverdisconnected",()=>{
    commandCharacteristic=null;
    setConnected(false);
    log("BLE disconnected");
  });
  const server=await device.gatt.connect();
  const service=await server.getPrimaryService(SERVICE_UUID);
  commandCharacteristic=await service.getCharacteristic(COMMAND_UUID);
  setConnected(true);
  log(`Connected to ${device.name||DEVICE_NAME}`);
}
async function disconnectBLE(){
  if(device?.gatt?.connected) device.gatt.disconnect();
  commandCharacteristic=null;
  setConnected(false);
  log("Disconnected");
}
async function sendCommand(command){
  if(!commandCharacteristic){
    log("Not connected — connect to M5-IR-Blaster first.");
    return;
  }
  try{
    const bytes=new TextEncoder().encode(command);
    await commandCharacteristic.writeValue(bytes);
    log(`Sent ${command}`);
  }catch(err){
    log(`Send error: ${err.message}`);
  }
}

connectBtn.addEventListener("click",async()=>{
  try{
    if(commandCharacteristic){await disconnectBLE();return;}
    await connectBLE();
  }catch(err){
    log(`BLE error: ${err.message}`);
  }
});

document.querySelectorAll("[data-cmd]").forEach(button=>{
  button.addEventListener("click",()=>sendCommand(button.dataset.cmd));
});

document.querySelectorAll('[data-action="unknown"]').forEach(button=>{
  button.addEventListener("click",()=>log(`${button.textContent.trim()}: code not assigned yet`));
});

$("clearLog").addEventListener("click",()=>logEl.textContent="Log cleared.");

let deferredPrompt=null;
window.addEventListener("beforeinstallprompt",e=>{
  e.preventDefault();
  deferredPrompt=e;
  $("installHint").textContent="This remote can be installed from your browser's Install App / Add to Home Screen option.";
});
if("serviceWorker" in navigator){
  navigator.serviceWorker.register("./sw.js").catch(err=>console.warn("SW:",err));
}
if(location.protocol!=="https:" && location.hostname!=="localhost"){
  $("installHint").textContent="GitHub Pages serves this over HTTPS, which is required for Web Bluetooth.";
}
