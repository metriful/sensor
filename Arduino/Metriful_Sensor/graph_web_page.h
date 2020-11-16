/* 
   graph_web_page.h

   This file contains the web page code which is used to display graphs
   as part of the graph_web_server example. This is the code from the 
   file 'graph_web_page.html' (HTML/CSS/javascript), which has been 
   minified and put into a C character string.

   Copyright 2020 Metriful Ltd. 
   Licensed under the MIT License - for further details see LICENSE.txt

   For code examples, datasheet and user guide, visit 
   https://github.com/metriful/sensor
*/

#ifndef GRAPH_WEB_PAGE_H
#define GRAPH_WEB_PAGE_H

const char * graphWebPage = "HTTP/1.1 200 OK\r\n" 
                     "Content-type: text/html\r\n" 
                     "Connection: close\r\n\r\n"
"<!DOCTYPE html>"
"<html>"
"<!--Copyright 2020 Metriful Ltd. Licensed under the MIT License.-->"
"<head>"
"<meta charset='UTF-8'>"
"<title>Indoor Environment Data</title>"
"<script src='https://cdn.plot.ly/plotly-1.56.0.min.js' charset='utf-8'></script>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<style>"
".tx {"
" font-family: Verdana, sans-serif;"
" text-align:center;"
" font-weight:normal;"
"}"
"</style>"
"</head>"
"<body style='background-color:#ededed;' onload='plotBufferedData()'>"
"<h3 class='tx'>Indoor Environment Data</h3>"
"<div id='tdata' class='tx'></div>"
"<div id='error' class='tx'>Incomplete load: please refresh the page.</div>"
"<div id='grid' style='display: flex;'></div>"
"<br>"
"<div class='tx'>"
"<button type='button' onclick='makeCSVfile()'>Download CSV data</button>"
"</div>"
"<br>"
"<p class='tx'><a href='https://www.sensor.metriful.com'>sensor.metriful.com</a></p>"
"<br>"
"<a id='lnk' href='' style='visibility:hidden;'></a>"
"<script>"
"var max_data_length=1e3,x_values=[],data=[],names=[\"Air Quality Index\",\"Temperature\",\"Pressure\",\"Humidity\",\"Sound Level\",\"Illuminance\",\"Breath VOC\",\"Particulates\"],units=new Map([[\"AQI\",\"\"],[\"T\",\"\u00B0C\"],[\"P\",\"Pa\"],[\"H\",\"%\"],[\"SPL\",\"dBA\"],[\"lux\",\"lux\"],[\"bVOC\",\"ppm\"],[\"part\",\"\u00B5g/m\u00B3\"]]),titles=[];const decimalPlaces=[1,1,0,1,1,2,2,2],AQI_position=0;var Ngraphs=0,isMobile=!1,doPlot=!0,includeParticles=!0,delay_ms=0,errorString=\"Incomplete load: please refresh the page.\";function padString(e){return 1==e.length?\"0\"+e:e}function makeTimeString(e){return d=new Date(e),padString(d.getHours().toString())+\":\"+padString(d.getMinutes().toString())+\":\"+padString(d.getSeconds().toString())}function makeTimeDateString(e){return d=new Date(e),d.getFullYear().toString()+\"-\"+padString((d.getMonth()+1).toString())+\"-\"+padString(d.getDate().toString())+\" \"+makeTimeString(e)}function plotGraph(e,t){P=document.getElementById(e),Plotly.newPlot(P,[{x:x_values,y:data[t],mode:\"lines\"}],{title:{text:titles[t],font:{family:\"verdana, sans-serif\",size:15},xref:\"paper\",x:isMobile?0:.5,yref:\"paper\",y:1,yanchor:\"bottom\",pad:{b:15}},plot_bgcolor:\"#f5f6f7\",paper_bgcolor:\"#ededed\",margin:{l:60,r:30,b:0,t:40},xaxis:{nticks:isMobile?3:7,showline:!0,automargin:!0,mirror:\"ticks\",linewidth:1},yaxis:{automargin:!0,showline:!0,mirror:\"ticks\",linewidth:1},autosize:!0},{responsive:!0,displaylogo:!1,modeBarButtonsToRemove:[\"toggleSpikelines\",\"hoverClosestCartesian\",\"hoverCompareCartesian\",\"zoomIn2d\",\"zoomOut2d\",\"autoScale2d\"]})}function interpretAQI(e){return e<50?\"Good\":e<100?\"Acceptable\":e<150?\"Substandard\":e<200?\"Poor\":e<300?\"Bad\":\"Very bad\"}function showTextData(){var e=x_values.length-1;const t=Date.now(),a=new Date(t);var r=\"<br>\"+makeTimeString(t)+\" \"+a.toDateString()+\"<br><br>\";r+=\"Air Quality: \"+interpretAQI(data[AQI_position][e])+\"<br><br>\";for(var i=0;i<Ngraphs;i++)r+=names[i]+\": \"+data[i][e].toFixed(decimalPlaces[i])+\" \"+units.get(Array.from(units.keys())[i])+\"<br><br>\";document.getElementById(\"tdata\").innerHTML=r}function plotBufferedData(){document.getElementById(\"error\").innerHTML=\"\",doPlot=!(\"undefined\"==typeof Plotly);var e=new XMLHttpRequest;e.onreadystatechange=function(){if(4==e.readyState&&200==e.status){const d=e.response;if(d.byteLength<5)return void(document.getElementById(\"error\").innerHTML=errorString);delay_ms=1e3*new Uint16Array(d.slice(0,2))[0];const m=new Uint8Array(d.slice(2,3))[0];Ngraphs=units.size,0==(15&m)?(Ngraphs-=1,includeParticles=!1):1==(15&m)&&units.set(\"part\",\"ppL\"),0!=(16&m)&&units.set(\"T\",\"\u00B0F\");for(var t=0;t<Ngraphs;t++){var a=units.get(Array.from(units.keys())[t]);\"\"===a?titles.push(names[t]):titles.push(names[t]+\" / \"+a)}const p=new Uint16Array(d.slice(3,5))[0];if(5+4*Ngraphs*p!=d.byteLength)return void(document.getElementById(\"error\").innerHTML=errorString);const c=new DataView(d,5);var r=0;for(t=0;t<Ngraphs;t++){data.push([]);for(var i=0;i<p;i++)data[t].push(c.getFloat32(r,!0)),r+=4}var n=Date.now();x_values=new Array(p);for(t=p;t>0;t--)x_values[t-1]=makeTimeDateString(n),n-=delay_ms;if(p>max_data_length&&(max_data_length=p),isMobile=isMobileDevice(),doPlot){var o=isMobile?33.3:50,s=\"<div class='column' style='flex: \"+(isMobile?100:50).toString()+\"%'>\",l=s;for(t=0;t<Ngraphs;t++)isMobile||t!=Math.ceil(Ngraphs/2)||(l+=\"</div>\"+s),l+=\"<div style='height:\"+o.toString()+\"vh'><div id='plot\"+t.toString()+\"' style='height:90%'></div></div>\";l+=\"</div>\",document.getElementById(\"grid\").innerHTML=l;for(t=0;t<Ngraphs;t++)plotGraph(\"plot\"+t.toString(),t)}else showTextData(),document.getElementById(\"error\").innerHTML=\"<br>Graphs are not displayed because the Plotly.js library could not be loaded.<br>Connect to the internet, or cache the script for offline use.<br><br>\";setTimeout(getLatestData,delay_ms)}},e.open(\"GET\",\"/1\",!0),e.responseType=\"arraybuffer\",e.send()}function getLatestData(){var e=new XMLHttpRequest;e.onreadystatechange=function(){if(4==e.readyState&&200==e.status){const a=new Float32Array(e.response);if(a.length==Ngraphs){for(var t=0;t<Ngraphs;t++)x_values.length==max_data_length&&data[t].shift(),data[t].push(a[t]);if(x_values.length==max_data_length&&x_values.shift(),x_values.push(makeTimeDateString(Date.now())),doPlot)for(t=0;t<Ngraphs;t++)plotGraph(\"plot\"+t.toString(),t);else showTextData()}setTimeout(getLatestData,delay_ms)}},e.open(\"GET\",\"/2\",!0),e.responseType=\"arraybuffer\",e.send()}function isMobileDevice(){let e=!1;var t;return t=navigator.userAgent||navigator.vendor||window.opera,(/(android|bb\\d+|meego).+mobile|avantgo|bada\\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\\.(browser|link)|vodafone|wap|windows ce|xda|xiino/i.test(t)||/1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\\-(n|u)|c55\\/|capi|ccwa|cdm\\-|cell|chtm|cldc|cmd\\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\\-s|devi|dica|dmob|do(c|p)o|ds(12|\\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\\-|_)|g1 u|g560|gene|gf\\-5|g\\-mo|go(\\.w|od)|gr(ad|un)|haie|hcit|hd\\-(m|p|t)|hei\\-|hi(pt|ta)|hp( i|ip)|hs\\-c|ht(c(\\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\\-(20|go|ma)|i230|iac( |\\-|\\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\\/)|klon|kpt |kwc\\-|kyo(c|k)|le(no|xi)|lg( g|\\/(k|l|u)|50|54|\\-[a-w])|libw|lynx|m1\\-w|m3ga|m50\\/|ma(te|ui|xo)|mc(01|21|ca)|m\\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\\-2|po(ck|rt|se)|prox|psio|pt\\-g|qa\\-a|qc(07|12|21|32|60|\\-[2-7]|i\\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\\-|oo|p\\-)|sdk\\/|se(c(\\-|0|1)|47|mc|nd|ri)|sgh\\-|shar|sie(\\-|m)|sk\\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\\-|v\\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\\-|tdg\\-|tel(i|m)|tim\\-|t\\-mo|to(pl|sh)|ts(70|m\\-|m3|m5)|tx\\-9|up(\\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\\-|your|zeto|zte\\-/i.test(t.substr(0,4)))&&(e=!0),e}function makeCSVfile(){for(var e='\"Time and Date\"',t=0;t<Ngraphs;t++)e+=',\"'+titles[t]+'\"';e+=\"\\r\\n\";for(var a=0;a<x_values.length;a++){e+='\"'+x_values[a]+'\"';for(t=0;t<Ngraphs;t++)e+=',\"'+data[t][a].toFixed(decimalPlaces[t])+'\"';e+=\"\\r\\n\"}var r=document.getElementById(\"lnk\");URL.revokeObjectURL(r.href),r.href=URL.createObjectURL(new Blob([e],{type:\"text/csv;charset=utf-8;\"})),r.download=\"data.csv\",r.click()}"
"</script>"
"</body>"
"</html>";

#endif
