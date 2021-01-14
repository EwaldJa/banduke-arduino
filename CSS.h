void append_page_header() {
  webpage  = F("<!DOCTYPE html><html>");
  webpage += F("<head>");
  webpage += F("<title>BanDuke</title>"); // NOTE: 1em = 16px
  webpage += F("<meta name='viewport' content='user-scalable=yes,initial-scale=1.0,width=device-width'>");
  webpage += F("<style>");
  webpage += F("body{max-width:65%;margin:0 auto;font-family:arial;font-size:105%;text-align:center;color:blue;background-color:#d3dee6;}");
  webpage += F("ul{list-style-type:none;margin:0.1em;padding:0;border-radius:0.375em;overflow:hidden;background-color:#f79400;font-size:1em;}");
  webpage += F("li{float:left;border-radius:0.375em;border-right:0.06em solid #282829;color:white;}last-child {border-right:none;font-size:85%}");
  webpage += F("li a{display: block;border-radius:0.375em;padding:0.44em 0.44em;text-decoration:none;font-size:85%;color:white;}");
  webpage += F("li a:hover{background-color:#c77a06;border-radius:0.375em;font-size:85%;color:white;}");
  webpage += F("section {font-size:0.88em;}");
  webpage += F("h1{color:white;border-radius:0.5em;font-size:1em;padding:0.2em 0.2em;background:#f79400;}");
  webpage += F("h2{color:white;font-size:1.0em;background:#f79400;}");
  webpage += F("h3{font-size:0.8em;color:#282829;}");
  webpage += F("table{font-family:arial,sans-serif;font-size:0.9em;border-collapse:collapse;width:92%;color:#282829;}"); 
  webpage += F("th,td {border:0.06em solid #282829;text-align:left;padding:0.3em;border-bottom:0.06em solid #282829;}"); 
  webpage += F("tr:nth-child(odd) {background-color:#c2c2c2;}");
  webpage += F("tr:nth-child(even) {background-color:#e0e0e0;}");
  webpage += F(".rcorners_n {border-radius:0.5em;background:#f79400;padding:0.3em 0.3em;width:20%;color:white;font-size:75%;}");
  webpage += F(".rcorners_m {border-radius:0.5em;background:#f79400;padding:0.3em 0.3em;width:50%;color:white;font-size:75%;}");
  webpage += F(".rcorners_w {border-radius:0.5em;background:#f79400;padding:0.3em 0.3em;width:70%;color:white;font-size:75%;}");
  webpage += F(".column{float:left;width:50%;height:45%;}");
  webpage += F(".row:after{content:'';display:table;clear:both;}");
  webpage += F("*{box-sizing:border-box;}");
  webpage += F("footer{background-color:#282829;color:white;text-align:center;padding:0.3em 0.3em;border-radius:0.375em;font-size:60%;}");
  webpage += F("button{border-radius:0.5em;background:#f79400;padding:0.3em 0.3em;width:20%;color:white;font-size:130%;}");
  webpage += F(".buttons {border-radius:0.5em;background:#f79400;padding:0.3em 0.3em;width:15%;color:white;font-size:80%;}");
  webpage += F(".buttonsm{border-radius:0.5em;background:#f79400;padding:0.3em 0.3em;width:9%; color:white;font-size:70%;}");
  webpage += F(".buttonm {border-radius:0.5em;background:#f79400;padding:0.3em 0.3em;width:15%;color:white;font-size:70%;}");
  webpage += F(".buttonw {border-radius:0.5em;background:#f79400;padding:0.3em 0.3em;width:40%;color:white;font-size:70%;}");
  webpage += F("a{font-size:75%;}");
  webpage += F("p{font-size:75%;}");
  webpage += F(".action_td{border:none;background-color:#d3dee6;}");
  webpage += F(".gg-youtube{color:#282829;box-sizing:border-box;position:relative;display:block;transform:scale(var(--ggs,1));width:16px;height:10px;box-shadow:0 0 0 2px;border-radius:15px 15px 15px 15px/45px 45px 45px 45px}.gg-youtube::before{content:\"\";display:block;box-sizing:border-box;position:absolute;left:7px;top:2px;border-left:4px solid currentColor;border-top:3px solid transparent;border-bottom:3px solid transparent}");
  webpage += F(".gg-software-download{color:#282829;box-sizing:border-box;position:relative;display:block;transform:scale(var(--ggs,1));width:16px;height:6px;border:2px solid;border-top:0;border-bottom-left-radius:2px;border-bottom-right-radius:2px;margin-top:8px}.gg-software-download::after{content:\"\";display:block;box-sizing:border-box;position:absolute;width:8px;height:8px;border-left:2px solid;border-bottom:2px solid;transform:rotate(-45deg);left:2px;bottom:4px}.gg-software-download::before{content:\"\";display:block;box-sizing:border-box;position:absolute;border-radius:3px;width:2px;height:10px;background:currentColor;left:5px;bottom:5px}");
  webpage += F(".gg-trash{color:#ed0000;box-sizing:border-box;position:relative;display:block;transform:scale(var(--ggs,1));width:10px;height:12px;border:2px solid transparent;box-shadow:0 0 0 2px,inset -2px 0 0,inset 2px 0 0;border-bottom-left-radius:1px;border-bottom-right-radius:1px;margin-top:4px}.gg-trash::after,.gg-trash::before{content:\"\";display:block;box-sizing:border-box;position:absolute}.gg-trash::after{background:currentColor;border-radius:3px;width:16px;height:2px;top:-4px;left:-5px}.gg-trash::before{width:10px;height:4px;border:2px solid;border-bottom:transparent;border-top-left-radius:2px;border-top-right-radius:2px;top:-7px;left:-2px}");
  webpage += F("</style></head><body><h1>BanDuke "); webpage += String(ServerVersion) + "</h1>";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void append_page_footer(){ // Saves repeating many lines of code for HTML page footers
  webpage += F("<br><br><a href='/'><button>Home</button></a>");
  webpage += F("<a href='/download'><button>Download</button></a>");
  webpage += F("<a href='/delete'><button>Delete</button></a>");
  webpage += F("<a href='/dir'><button>Directory</button></a><br><br>");
  webpage += F("<footer>&copy; BanDuke 2020</footer><br><br>");
  webpage += F("</body></html>");
}
