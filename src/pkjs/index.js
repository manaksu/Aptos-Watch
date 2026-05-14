/*
 * Aptos Watchface -- PebbleKit JS
 *   Key 0: KEY_BG    0=Black  1=Dark Grey  2=Navy  3=ePaper
 *   Key 1: KEY_CASE  0=All Caps  1=Mixed
 *   Key 2: KEY_SYM   0=White  1=Grey  2=Red  3=Amber
 *   Key 3: KEY_TEX   0=Clean  1=Texture
 */
function loadCfg() {
  return {
    bg:  +(localStorage.getItem('bg')  || '0'),
    cas: +(localStorage.getItem('cas') || '0'),
    sym: +(localStorage.getItem('sym') || '0'),
    tex: +(localStorage.getItem('tex') || '0')
  };
}
function saveCfg(c) {
  localStorage.setItem('bg',  c.bg);
  localStorage.setItem('cas', c.cas);
  localStorage.setItem('sym', c.sym);
  localStorage.setItem('tex', c.tex);
}
function sendMsg(c) {
  Pebble.sendAppMessage(
    { 0: c.bg, 1: c.cas, 2: c.sym, 3: c.tex },
    function() { console.log('ok'); },
    function(e) { console.log('fail', e); }
  );
}
function buildConfig(c) {
  function radio(name, opts, sel) {
    return opts.map(function(l, i) {
      return '<label class="opt"><input type="radio" name="' + name
        + '" value="' + i + '"' + (i === sel ? ' checked' : '')
        + '><span>' + l + '</span></label>';
    }).join('');
  }
  var h = '<!DOCTYPE html><html><head>'
    + '<meta charset="utf-8">'
    + '<meta name="viewport" content="width=device-width,initial-scale=1">'
    + '<style>'
    + 'body{margin:0;font:15px/1.6 -apple-system,sans-serif;background:#0d0d0d;color:#ccc;padding:20px}'
    + 'h3{font-size:11px;text-transform:uppercase;letter-spacing:.08em;color:#555;margin:22px 0 8px}'
    + 'h3:first-child{margin-top:0}'
    + '.opt{display:flex;align-items:center;gap:12px;background:#1a1a1a;border-radius:8px;padding:13px;margin:5px 0;cursor:pointer}'
    + '.opt input{accent-color:#aaa;width:18px;height:18px;flex-shrink:0;margin:0}'
    + '.opt span{font-size:14px}'
    + '#s{display:block;width:100%;padding:14px;background:#222;color:#fff;border:1px solid #3a3a3a;border-radius:8px;font-size:15px;margin-top:24px;cursor:pointer;box-sizing:border-box}'
    + '</style></head><body>'
    + '<h3>Background</h3>'
    + radio('bg',  ['Black', 'Dark Grey', 'Navy', 'ePaper', 'Cream Terminal'], c.bg)
    + '<h3>Text Case</h3>'
    + radio('cas', ['All Caps', 'Mixed'], c.cas)
    + '<h3>Symbol Color</h3>'
    + radio('sym', ['White', 'Grey', 'Red', 'Amber'], c.sym)
    + '<h3>Background Texture</h3>'
    + radio('tex', ['Clean', 'Grain'], c.tex)
    + '<button id="s">Save</button>'
    + '<script>'
    + 'document.getElementById("s").onclick=function(){'
    + 'function g(n){var e=document.querySelector("input[name="+n+"]:checked");return e?+e.value:0;}'
    + 'location.href="pebblejs://close#"+encodeURIComponent(JSON.stringify({'
    + 'bg:g("bg"),cas:g("cas"),sym:g("sym"),tex:g("tex")}));'
    + '};<\/script></body></html>';
  return 'data:text/html,' + encodeURIComponent(h);
}
Pebble.addEventListener('ready', function() { console.log('Aptos ready'); });
Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL(buildConfig(loadCfg()));
});
Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response || e.response === '' || e.response === 'CANCELLED') return;
  var raw = e.response;
  if (raw.indexOf('#') !== -1) raw = raw.substring(raw.lastIndexOf('#') + 1);
  var c;
  try { c = JSON.parse(decodeURIComponent(raw)); } catch(err) { return; }
  saveCfg(c);
  sendMsg(c);
});
