// ============================================================
//  مُحوِّل الدوال — خادم C++ (cpp-httplib)
//  يحوِّل "— دالة اسم الدالة // وش يسوّي الموقع"
//  إلى موقع كامل بأي لغة (HTML / Python / C++ / PHP)
//
//  التجميع (لينكس/ماك):
//    wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
//    g++ -std=c++17 -O2 server.cpp -o server -lpthread
//    ./server
//
//  التجميع (ويندوز + MinGW):
//    g++ -std=c++17 -O2 server.cpp -o server.exe -lws2_32
//    server.exe
//
//  بعدها افتح: http://localhost:8080
// ============================================================

#include "httplib.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ---------- بنية الدالة ----------
struct Spec {
    std::string name;
    std::string desc;
};

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// ---------- قراءة الدوال من النص ----------
static std::vector<Spec> parseSpecs(const std::string& input) {
    std::vector<Spec> out;
    std::istringstream ss(input);
    std::string line;
    const std::string marker = "دالة";
    while (std::getline(ss, line)) {
        size_t p = line.find(marker);
        if (p == std::string::npos) continue;
        std::string rest = line.substr(p + marker.size());
        size_t c = rest.find("//");
        std::string name = (c == std::string::npos) ? trim(rest) : trim(rest.substr(0, c));
        std::string desc = (c == std::string::npos) ? "" : trim(rest.substr(c + 2));
        if (name.empty()) name = "موقع بدون اسم";
        out.push_back({name, desc});
    }
    return out;
}

// ---------- تهريب النصوص ----------
static std::string esc(const std::string& s) { // HTML escape
    std::string r;
    for (char ch : s) {
        switch (ch) {
            case '&': r += "&amp;"; break;
            case '<': r += "&lt;";  break;
            case '>': r += "&gt;";  break;
            case '"': r += "&quot;"; break;
            default:  r += ch;
        }
    }
    return r;
}

static std::string pyesc(const std::string& s) { // Python escape
    std::string r;
    for (char ch : s) {
        if (ch == '\\') r += "\\\\";
        else if (ch == '"') r += "\\\"";
        else r += ch;
    }
    return r;
}

static std::string cppesc(const std::string& s) { // C++ escape
    std::string r;
    for (char ch : s) {
        if (ch == '\\') r += "\\\\";
        else if (ch == '"') r += "\\\"";
        else r += ch;
    }
    return r;
}

static std::string phpesc(const std::string& s) { // PHP escape
    std::string r;
    for (char ch : s) {
        if (ch == '\\') r += "\\\\";
        else if (ch == '\'') r += "\\'";
        else r += ch;
    }
    return r;
}

static std::string jesc(const std::string& s) { // JSON escape
    std::string r;
    for (char ch : s) {
        switch (ch) {
            case '"':  r += "\\\""; break;
            case '\\': r += "\\\\"; break;
            case '\n': r += "\\n";  break;
            case '\r': break;
            case '\t': r += "\\t";  break;
            default:   r += ch;
        }
    }
    return r;
}

// ---------- توليد موقع HTML ----------
static std::string genHTML(const std::vector<Spec>& specs, const std::string& title) {
    std::ostringstream o;
    o << "<!DOCTYPE html>\n<html lang=\"ar\" dir=\"rtl\">\n<head>\n"
      << "<meta charset=\"UTF-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
      << "<title>" << esc(title) << "</title>\n"
      << "<style>\n"
      << "body{font-family:Tahoma,Arial;background:#0a1512;color:#d9f7e9;margin:0;padding:2rem}\n"
      << "h1{color:#4ade80}\n"
      << ".card{background:#0e2419;border:1px solid #1f7a4d;border-radius:14px;padding:1.2rem;margin:1rem 0}\n"
      << ".card h2{margin:0 0 .4rem;color:#86efac}\n"
      << "p{line-height:1.8}\n"
      << "</style>\n</head>\n<body>\n"
      << "<h1>" << esc(title) << "</h1>\n";
    for (const auto& s : specs)
        o << "<div class=\"card\"><h2>" << esc(s.name) << "</h2><p>" << esc(s.desc) << "</p></div>\n";
    o << "</body>\n</html>\n";
    return o.str();
}

// ---------- توليد Python + Flask ----------
static std::string genPython(const std::vector<Spec>& specs, const std::string& title) {
    std::ostringstream o;
    o << "from flask import Flask\n\n"
      << "app = Flask(__name__)\n\n"
      << "SECTIONS = [\n";
    for (const auto& s : specs)
        o << "    {\"name\": \"" << pyesc(esc(s.name)) << "\", \"desc\": \"" << pyesc(esc(s.desc)) << "\"},\n";
    o << "]\n\n"
      << "def build():\n"
      << "    cards = \"\"\n"
      << "    for s in SECTIONS:\n"
      << "        cards += '<div class=\"card\"><h2>' + s[\"name\"] + '</h2><p>' + s[\"desc\"] + '</p></div>'\n"
      << "    return '<!DOCTYPE html><html lang=\"ar\" dir=\"rtl\"><head><meta charset=\"UTF-8\">'\n"
      << "           '<title>" << pyesc(esc(title)) << "</title>'\n"
      << "           '<style>body{font-family:Tahoma;background:#0a1512;color:#d9f7e9;padding:2rem}'\n"
      << "           '.card{background:#0e2419;border:1px solid #1f7a4d;border-radius:14px;padding:1.2rem;margin:1rem 0}'\n"
      << "           '.card h2{color:#86efac}</style></head><body><h1>" << pyesc(esc(title)) << "</h1>'\n"
      << "           + cards + '</body></html>'\n\n"
      << "@app.route(\"/\")\n"
      << "def index():\n"
      << "    return build()\n\n"
      << "if __name__ == \"__main__\":\n"
      << "    app.run(debug=True, port=5000)\n";
    return o.str();
}

// ---------- توليد C++ (cpp-httplib) ----------
static std::string genCpp(const std::vector<Spec>& specs, const std::string& title) {
    std::ostringstream o;
    o << "#include \"httplib.h\"\n"
      << "#include <string>\n\n"
      << "int main() {\n"
      << "    httplib::Server svr;\n\n"
      << "    std::string cards;\n";
    for (const auto& s : specs)
        o << "    cards += \"<div class=\\\"card\\\"><h2>" << cppesc(esc(s.name))
          << "</h2><p>" << cppesc(esc(s.desc)) << "</p></div>\";\n";
    o << "\n"
      << "    const std::string page =\n"
      << "        \"<!DOCTYPE html><html lang=\\\"ar\\\" dir=\\\"rtl\\\"><head><meta charset=\\\"UTF-8\\\">\"\n"
      << "        \"<title>" << cppesc(esc(title)) << "</title>\"\n"
      << "        \"<style>body{font-family:Tahoma;background:#0a1512;color:#d9f7e9;padding:2rem}\"\n"
      << "        \".card{background:#0e2419;border:1px solid #1f7a4d;border-radius:14px;padding:1.2rem;margin:1rem 0}\"\n"
      << "        \".card h2{color:#86efac}</style></head><body><h1>" << cppesc(esc(title)) << "</h1>\"\n"
      << "        + cards + \"</body></html>\";\n\n"
      << "    svr.Get(\"/\", [&](const httplib::Request&, httplib::Response& res) {\n"
      << "        res.set_content(page, \"text/html; charset=utf-8\");\n"
      << "    });\n\n"
      << "    svr.listen(\"0.0.0.0\", 8080);\n"
      << "    return 0;\n"
      << "}\n";
    return o.str();
}

// ---------- توليد PHP ----------
static std::string genPHP(const std::vector<Spec>& specs, const std::string& title) {
    std::ostringstream o;
    o << "<?php\n"
      << "$sections = [\n";
    for (const auto& s : specs)
        o << "    ['name' => '" << phpesc(esc(s.name)) << "', 'desc' => '" << phpesc(esc(s.desc)) << "'],\n";
    o << "];\n?>\n"
      << "<!DOCTYPE html>\n<html lang=\"ar\" dir=\"rtl\">\n"
      << "<head><meta charset=\"UTF-8\"><title>" << esc(title) << "</title>\n"
      << "<style>body{font-family:Tahoma;background:#0a1512;color:#d9f7e9;padding:2rem}"
      << ".card{background:#0e2419;border:1px solid #1f7a4d;border-radius:14px;padding:1.2rem;margin:1rem 0}"
      << ".card h2{color:#86efac}</style></head>\n<body>\n"
      << "<h1>" << esc(title) << "</h1>\n"
      << "<?php foreach ($sections as $s): ?>\n"
      << "<div class=\"card\"><h2><?php echo $s['name']; ?></h2><p><?php echo $s['desc']; ?></p></div>\n"
      << "<?php endforeach; ?>\n"
      << "</body>\n</html>\n";
    return o.str();
}

// ---------- قراءة قيمة من JSON (بسيطة) ----------
static std::string jsonGet(const std::string& body, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t p = body.find(needle);
    if (p == std::string::npos) return "";
    p = body.find(':', p + needle.size());
    if (p == std::string::npos) return "";
    p = body.find('"', p + 1);
    if (p == std::string::npos) return "";
    size_t q = p + 1;
    std::string out;
    while (q < body.size()) {
        char ch = body[q];
        if (ch == '\\') {
            ++q;
            if (q >= body.size()) break;
            char e = body[q];
            switch (e) {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': break;
                case '\\': out += '\\'; break;
                case '"': out += '"'; break;
                default: out += e;
            }
            ++q;
            continue;
        }
        if (ch == '"') break;
        out += ch;
        ++q;
    }
    return out;
}

// ============================================================
//  صفحة الويب (الواجهة) — مدمجة داخل الـ C++ نفسه
// ============================================================
static const std::string PAGE = R"PAGE(
<!DOCTYPE html>
<html lang="ar" dir="rtl">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>مُحوِّل الدوال — من دالة إلى موقع</title>
<style>
  * { box-sizing:border-box; margin:0; padding:0; }
  body {
    font-family:'Segoe UI', Tahoma, Arial, sans-serif;
    background:radial-gradient(1200px 600px at 80% -10%, #0f2b1d 0%, #070d0a 55%);
    color:#d9f7e9; min-height:100vh; padding:2rem 1rem;
  }
  .wrap { max-width:900px; margin:0 auto; }
  header { text-align:center; margin-bottom:2rem; }
  header h1 { color:#4ade80; font-size:clamp(1.4rem, 4vw, 2.1rem); }
  header p { color:#7fb99a; margin-top:.5rem; }
  .box {
    background:#0b2418;
    border:1px solid #1f7a4d;
    border-radius:22px;
    padding:1.4rem;
    box-shadow:0 10px 40px rgba(0,0,0,.45);
  }
  textarea {
    width:100%; min-height:170px;
    background:transparent; border:none; outline:none; resize:vertical;
    color:#4ade80; caret-color:#4ade80;
    font-family:Consolas, 'Courier New', monospace; font-size:1.05rem; line-height:1.9;
  }
  textarea::placeholder { color:#2f7a52; }
  .hint { font-size:.85rem; color:#7fb99a; margin-top:.6rem; }
  .preview { margin-top:1rem; border-top:1px dashed #1f7a4d; padding-top:1rem; }
  .preview h3, .sect h3 { color:#86efac; font-size:.95rem; margin-bottom:.6rem; }
  .pcard {
    background:rgba(74,222,128,.10);
    border:1px dashed rgba(74,222,128,.45);
    border-radius:12px; padding:.7rem .9rem; margin:.45rem 0;
    opacity:.75;
  }
  .pcard .n { color:#4ade80; font-weight:bold; }
  .pcard .d { color:#a7d8be; font-size:.9rem; }
  .pcard .ok { float:left; color:#86efac; font-size:.8rem; }
  .bad { color:#f87171; font-size:.85rem; margin-top:.4rem; }
  .btn {
    display:inline-block; border:none; cursor:pointer;
    background:#16a34a; color:#eafff3;
    font-weight:bold; font-size:1rem;
    padding:.8rem 2.2rem; border-radius:12px;
    transition:.2s; font-family:inherit;
  }
  .btn:hover { background:#22c55e; transform:translateY(-1px); }
  .btn:disabled { opacity:.4; cursor:not-allowed; transform:none; }
  .sect { margin-top:1.6rem; }
  label { color:#86efac; display:block; margin-bottom:.4rem; font-size:.95rem; }
  select {
    width:100%; background:#0b2418; color:#4ade80;
    border:1px solid #1f7a4d; border-radius:12px;
    padding:.8rem; font-size:1rem; font-family:inherit; outline:none;
  }
  .result {
    margin-top:1.4rem; background:#050b08; border:1px solid #1f7a4d;
    border-radius:16px; overflow:hidden;
  }
  .result .bar {
    display:flex; justify-content:space-between; align-items:center; flex-wrap:wrap; gap:.5rem;
    background:#0b2418; padding:.6rem 1rem; border-bottom:1px solid #1f7a4d;
  }
  .result .bar span { color:#7fb99a; font-size:.85rem; }
  .result .bar .acts button {
    background:#14532d; border:1px solid #1f7a4d; color:#4ade80;
    border-radius:8px; padding:.4rem .9rem; cursor:pointer; margin-right:.4rem;
    font-family:inherit; font-size:.85rem;
  }
  .result .bar .acts button:hover { background:#1f7a4d; color:#eafff3; }
  pre {
    max-height:420px; overflow:auto; padding:1rem;
    color:#86efac; font-family:Consolas, 'Courier New', monospace;
    font-size:.85rem; line-height:1.6; direction:ltr; text-align:left;
    white-space:pre;
  }
  .footer { text-align:center; margin-top:2.4rem; color:#3f6b52; font-size:.8rem; }
  .hidden { display:none; }
</style>
</head>
<body>
<div class="wrap">
  <header>
    <h1>⚡ مُحوِّل الدوال — من دالة إلى موقع</h1>
    <p>اكتب دالة بصيغة «— دالة اسم // وش يسوّي الموقع» وحوّلها لموقع كامل بأي لغة</p>
  </header>

  <div class="box">
    <textarea id="inp" placeholder="— دالة موقع ترحيبي // موقع فيه عنوان كبير وزر يغيّر اللون&#10;— دالة صفحة عني // صفحة فيها نبذة عني وسيرة سريعة"></textarea>
    <div class="hint">الصيغة: <b style="color:#4ade80">— دالة اسم الدالة // وش يسوّي الموقع</b> — وكل سطر = دالة</div>

    <div class="preview" id="preview"></div>

    <div style="margin-top:1.2rem; text-align:center">
      <button class="btn" id="btn1">تحويل</button>
    </div>
  </div>

  <div class="sect hidden" id="listSec">
    <h3>📋 قائمة المواقع المحوّلة</h3>
    <div id="list"></div>
    <label style="margin-top:1rem">اختر لغة الموقع:</label>
    <select id="lang">
      <option value="html">HTML + JavaScript (صفحة وحدة)</option>
      <option value="python">Python + Flask</option>
      <option value="cpp">C++ (cpp-httplib)</option>
      <option value="php">PHP</option>
    </select>
    <div style="margin-top:1rem; text-align:center">
      <button class="btn" id="btn2">تحويل الموقع</button>
    </div>
  </div>

  <div class="result hidden" id="result">
    <div class="bar">
      <span id="fname"></span>
      <span class="acts">
        <button id="copy">نسخ</button>
        <button id="dl">تحميل ملف</button>
      </span>
    </div>
    <pre id="code"></pre>
  </div>

  <div class="footer">مُحوِّل الدوال — خادم C++ يخدم الصفحة ويولّد الكود 🌿</div>
</div>

<script>
const $ = id => document.getElementById(id);
const inp = $('inp'), preview = $('preview'), list = $('list');

function parse(text) {
  const specs = [];
  text.split('\n').forEach(line => {
    const i = line.indexOf('دالة');
    if (i === -1) return;
    let rest = line.slice(i + 4);
    const c = rest.indexOf('//');
    let name = (c === -1 ? rest : rest.slice(0, c)).trim();
    let desc = c === -1 ? '' : rest.slice(c + 2).trim();
    if (!name) name = 'موقع بدون اسم';
    specs.push({ name, desc });
  });
  return specs;
}

function renderPreview() {
  const specs = parse(inp.value);
  if (!specs.length) {
    preview.innerHTML = '<div class="bad">ما لقينا دالة… اكتب سطر مثل: — دالة اسم // وصف</div>';
    return;
  }
  preview.innerHTML = '<h3>👁 الدوال اللي انفهمت (شفافة):</h3>' + specs.map((s, i) =>
    '<div class="pcard"><span class="ok">✓ مفهومة</span><span class="n">دالة ' + (i + 1) + ': ' + s.name + '</span><br><span class="d">' + (s.desc || '(بدون وصف)') + '</span></div>'
  ).join('');
}

inp.addEventListener('input', renderPreview);
inp.addEventListener('keydown', e => { if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') $('btn1').click(); });
renderPreview();

$('btn1').addEventListener('click', () => {
  const specsCache = parse(inp.value);
  if (!specsCache.length) { alert('اكتب دالة وحدة على الأقل بصيغة: — دالة اسم // وصف'); return; }
  list.innerHTML = specsCache.map((s, i) =>
    '<div class="pcard" style="opacity:1"><span class="ok">موقع ' + (i + 1) + '</span><span class="n">' + s.name + '</span><br><span class="d">' + (s.desc || '(بدون وصف)') + '</span></div>'
  ).join('');
  $('listSec').classList.remove('hidden');
  $('listSec').scrollIntoView({ behavior: 'smooth' });
});

$('btn2').addEventListener('click', async () => {
  const btn = $('btn2');
  btn.disabled = true; btn.textContent = '... جاري التوليد';
  try {
    const r = await fetch('/api/convert', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ text: inp.value, lang: $('lang').value })
    });
    const data = await r.json();
    if (!data.ok) { alert('ما صار تحويل: ' + (data.error || 'خطأ')); return; }
    $('code').textContent = data.code;
    $('fname').textContent = '📄 ' + data.filename + ' — ' + data.count + ' دوال';
    $('result').classList.remove('hidden');
    $('result').scrollIntoView({ behavior: 'smooth' });
  } catch (e) {
    alert('الخادم ما استجاب… تأكد إنك شغّل ملف server');
  } finally {
    btn.disabled = false; btn.textContent = 'تحويل الموقع';
  }
});

$('copy').addEventListener('click', async () => {
  try {
    await navigator.clipboard.writeText($('code').textContent);
    alert('تم النسخ ✅');
  } catch (e) { alert('ما اننسخ تلقائي، انسخه يدوي'); }
});

$('dl').addEventListener('click', () => {
  const name = $('fname').textContent.replace('📄 ', '').split('—')[0].trim();
  const blob = new Blob([$('code').textContent], { type: 'text/plain;charset=utf-8' });
  const a = document.createElement('a');
  a.href = URL.createObjectURL(blob);
  a.download = name;
  a.click();
  URL.revokeObjectURL(a.href);
});
</script>
</body>
</html>
)PAGE";

// ============================================================
//  الخادم
// ============================================================
int main() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(PAGE, "text/html; charset=utf-8");
    });

    svr.Post("/api/convert", [](const httplib::Request& req, httplib::Response& res) {
        const std::string text = jsonGet(req.body, "text");
        const std::string lang = jsonGet(req.body, "lang");

        std::vector<Spec> specs = parseSpecs(text);
        if (specs.empty()) {
            res.set_content("{\"ok\":false,\"error\":\"no functions found\"}", "application/json; charset=utf-8");
            return;
        }

        const std::string title = specs[0].name;
        std::string code, filename;

        if (lang == "python") { code = genPython(specs, title); filename = "app.py"; }
        else if (lang == "cpp") { code = genCpp(specs, title); filename = "site.cpp"; }
        else if (lang == "php") { code = genPHP(specs, title); filename = "index.php"; }
        else { code = genHTML(specs, title); filename = "index.html"; }

        std::string out = "{\"ok\":true,\"filename\":\"" + jesc(filename)
                        + "\",\"count\":" + std::to_string(specs.size())
                        + ",\"code\":\"" + jesc(code) + "\"}";
        res.set_content(out, "application/json;
