/* 
  graph_web_page.h

  This file contains the web page code which is used to display graphs
  as part of the graph_web_server example.
  This uses HTML, CSS, javascript inside a C character string.

  Copyright 2020-2023 Metriful Ltd.
  Licensed under the MIT License - for further details see LICENSE.txt

  For code examples, datasheet and user guide, visit
  https://github.com/metriful/sensor
*/

#ifndef GRAPH_WEB_PAGE_H
#define GRAPH_WEB_PAGE_H

#define QUOTE(...) #__VA_ARGS__

// This is the HTTP response header for the web page.
const char * pageHeader = "HTTP/1.1 200 OK\r\n"
                          "Content-type: text/html\r\n"
                          "Connection: close\r\n\r\n";

// HTTP response header for error.
const char * errorResponseHTTP = "HTTP/1.1 400 Bad Request\r\n\r\n";

// HTTP response header for data transfer.
const char * dataHeader = "HTTP/1.1 200 OK\r\n"
                          "Content-type: application/octet-stream\r\n"
                          "Connection: close\r\n\r\n";

// This is the full web page text.
const char * graphWebPage = QUOTE(
<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <title>Indoor Environment Data</title>
  <script src='https://cdn.plot.ly/plotly-1.56.0.min.js'></script>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    .tx {
      font-family: Verdana, sans-serif;
      text-align: center;
    }
    .v {text-align: right; width: 5.5rem;}
    table, th, td {font-size: 1rem; font-family: Verdana, sans-serif;}
    table {margin-left:auto; margin-right:auto;}
    th, td {
      padding: 0.025rem 0.5rem;
      text-align: left; vertical-align: bottom;
    }
  </style>
</head>

<body style='background-color:#ededed;' onload='plotBufferedData()'>
  <h3 class='tx'>Indoor Environment Data</h3>
  <div id='textData' class='tx' style='display: none;'></div>
  <div id='error' class='tx'>Incomplete load: please refresh the page.</div>
  <div id='plotlyError' class='tx'></div>
  <div id='grid' style='display: flex;'></div>
  <br>
  <div class='tx'>
    <button type='button' id='viewButton' onclick='toggleView()'>
        View text data</button>
    &nbsp;&nbsp;&nbsp;
    <button type='button' onclick='makeCSVfile()'>Download CSV data</button>
  </div>
  <br>
  <p class='tx'><a href='https://www.sensor.metriful.com'>
      sensor.metriful.com</a></p>
  <br>
  <a id='CSVlink' href='' style='visibility:hidden;'></a>
  <script>
    var maxDataLength = 1000;
    var xValues = [];
    var data = [];
    var names = ['Air Quality Index', 'Temperature', 'Pressure',
                 'Humidity', 'Sound Level', 'Illuminance',
                 'Breath VOC', 'Particulates'];
    var units = new Map([['AQI', ''], ['T', '\u00B0C'], ['P', 'Pa'],
                        ['H', '%'], ['SPL', 'dBA'], ['lux', 'lux'],
                        ['bVOC', 'ppm'], ['part', '\u00B5g/m\u00B3']]);
    var titles = [];
    const decimalPlaces = [1, 1, 0, 1, 1, 2, 2, 2];
    const AQIposition = 0;
    var Ngraphs = 0;
    // Choose whether to show graphs in two columns, or a single column:
    const singleColumn = screen.width < 600;
    const plotlyAvailable = !(typeof (Plotly) == 'undefined');
    var viewGraphs = true;
    var includeParticles = true;
    var delay_ms = 0;

    // Switch between graph and text views
    function toggleView()
    {
      viewGraphs = !viewGraphs;
      if (viewGraphs)
      {
        document.getElementById(
            'viewButton').innerHTML = 'View text data';
      }
      else
      {
        document.getElementById(
            'viewButton').innerHTML = 'View graphs';
      }
      if (plotlyAvailable && viewGraphs)
      {
        document.getElementById('grid').style.display = 'flex';
        document.getElementById('textData').style.display = 'none';
        document.getElementById('plotlyError').style.display = 'none';
      }
      else
      {
        document.getElementById('grid').style.display = 'none';
        document.getElementById('textData').style.display = 'block';
        if ((!plotlyAvailable) && viewGraphs)
        {
          document.getElementById(
              'plotlyError').style.display = 'block';
        }
        else
        {
          document.getElementById(
              'plotlyError').style.display = 'none';
        }
      }
    }

    // Get time as a string in the format HH:MM:SS
    function makeTimeString(date)
    {
      return (date.getHours().toString().padStart(2, '0')
             + ':' + date.getMinutes().toString().padStart(2, '0')
             + ':' + date.getSeconds().toString().padStart(2, '0'));
    }

    // Get date as a string in the format YYYY-mm-DD
    function makeDateString(date)
    {
      return (date.getFullYear().toString()
             + '-' + (date.getMonth() + 1).toString().padStart(2, '0')
             + '-' + date.getDate().toString().padStart(2, '0'));
    }

    // Get time and date as a string in the format YYYY-mm-DD HH:MM:SS
    function makeTimeDateString(date)
    {
      return (makeDateString(date) + ' ' + makeTimeString(date));
    }

    // Make graphs using Plotly
    function plotGraph(plotName, i)
    {
      P = document.getElementById(plotName);
      Plotly.newPlot(P, [{
        x: xValues,
        y: data[i],
        mode: 'lines'
      }], {
        title: {
          text: titles[i],
          font: {
              family: 'verdana, sans-serif',
              size: 15
          },
          xref: 'paper',
          x: (singleColumn ? 0 : 0.5),
          yref: 'paper',
          y: 1,
          yanchor: 'bottom',
          pad: { b: 15 }
        },
        plot_bgcolor: '#f5f6f7',
        paper_bgcolor: '#ededed',
        margin: {
          l: 60,
          r: 30,
          b: 0,
          t: 40
        },
        xaxis: {
          nticks: (singleColumn ? 3 : 7),
          showline: true,
          automargin: true,
          mirror: 'ticks',
          linewidth: 1
        },
        yaxis: {
          automargin: true,
          showline: true,
          mirror: 'ticks',
          linewidth: 1
        },
        autosize: true
      },
        {
          responsive: true, displaylogo: false,
          modeBarButtonsToRemove: ['toggleSpikelines',
              'hoverClosestCartesian', 'hoverCompareCartesian',
              'zoomIn2d', 'zoomOut2d', 'autoScale2d']
        });
    }

    // Provide a text interpretation of the air quality index value
    function interpretAQI(AQI)
    {
      if (AQI < 50) {
        return 'Good';
      }
      else if (AQI < 100) {
        return 'Acceptable';
      }
      else if (AQI < 150) {
        return 'Substandard';
      }
      else if (AQI < 200) {
        return 'Poor';
      }
      else if (AQI < 300) {
        return 'Bad';
      }
      else {
        return 'Very bad';
      }
    }

    // Display data as text
    function createTextData()
    {
      const j = xValues.length - 1;
      let t = '<br>Last update at: ' + makeTimeDateString(new Date())
              + '<br><br>';
      t += '<table><tr><td>Air Quality</td><td class="v">'
           + interpretAQI(data[AQIposition][j]) + '</td><td></td></tr>';
      for (let i = 0; i < Ngraphs; i++) {
        t += '<tr><td>' + names[i] + '</td><td class="v">' 
              + data[i][j].toFixed(decimalPlaces[i]) + '</td><td>'
              + units.get(Array.from(units.keys())[i]) + '</td></tr>';
      }
      t += '</table>';
      document.getElementById('textData').innerHTML = t;
    }

    // Set graph title strings with units
    function createGraphTitles()
    {
      for (let i = 0; i < Ngraphs; i++)
      {
        let unit = units.get(Array.from(units.keys())[i]);
        if (unit === '')
        {
          titles.push(names[i]);
        }
        else
        {
          titles.push(names[i] + ' / ' + unit);
        }
      }
    }

    // Unpack received data into the data array
    function extractAndDecodeData(dataView, bufferLength)
    {
      data = [];
      let byteOffset = 0;
      for (let i = 0; i < Ngraphs; i++)
      {
        data.push([]);
        for (let v = 0; v < bufferLength; v++)
        {
          data[i].push(dataView.getFloat32(byteOffset, true));
          byteOffset += 4;
        }
      }
    }

    // Find approximate time for each data point, based on the current
    // time and the known cycle delay time.
    function assignTimeData(bufferLength)
    {
      let t = Date.now();
      xValues = new Array(bufferLength);
      for (var i = bufferLength; i > 0; i--)
      {
        xValues[i - 1] = makeTimeDateString(new Date(t));
        t = t - delay_ms;
      }
    }

    function createGraphGrid()
    {
      let width_pc = singleColumn ? 100 : 50;
      let height_vh = singleColumn ? 33.3 : 50;
      let columnHtml = "<div class='column' style='flex: "
                       + width_pc.toString() + "%'>";
      let mainHtml = columnHtml;
      for (let i = 0; i < Ngraphs; i++)
      {
        if ((!singleColumn) && (i == Math.ceil(Ngraphs / 2)))
        {
          mainHtml += "</div>" + columnHtml;
        }
        mainHtml += "<div style='height: max(" + height_vh.toString()
                    + "vh,225px)'><div id='plot" + i.toString()
                    + "' style='height:90%'></div></div>";
      }
      mainHtml += "</div>";
      document.getElementById('grid').innerHTML = mainHtml;
    }

    // Do a GET request for all the buffered data and generate the graphs
    function plotBufferedData()
    {
      var xmlhttp = new XMLHttpRequest();
      xmlhttp.onreadystatechange = function ()
      {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200)
        {
          const body = xmlhttp.response;
          if (body.byteLength < 6)
          {
            // The correct response should have at least 6 bytes
            return;
          }
          // Get the time interval between new data
          delay_ms = (new Uint16Array(body.slice(0, 2)))[0] * 1000;
          const particleSensorByte = (new Uint8Array(
                                      body.slice(2, 3)))[0];
          Ngraphs = units.size; // number of graphs to plot
          if (particleSensorByte == 0)
          {
            // There is no particle sensor, so omit one graph
            Ngraphs -= 1;
            includeParticles = false;
          }
          else if (particleSensorByte == 1)
          {
            // A PPD42 sensor is used: change the units 
            units.set('part', 'ppL');
          }
          const useFahrenheit = (new Uint8Array(body.slice(3, 4)))[0];
          if (useFahrenheit != 0)
          {
            // Change temperature unit to Fahrenheit (default is Celsius)
            units.set('T', '\u00B0F');
          }
          createGraphTitles();
          // Check length of remaining data:
          const bufferLength = (new Uint16Array(body.slice(4, 6)))[0];
          let expectedBytes = 6 + (Ngraphs * 4 * bufferLength);
          if (expectedBytes != body.byteLength)
          {
            return;
          }
          document.getElementById('error').innerHTML = '';
          // Extract and decode data, starting at byte 6
          extractAndDecodeData(new DataView(body, 6), bufferLength);
          assignTimeData(bufferLength);
          if (bufferLength > maxDataLength)
          {
            maxDataLength = bufferLength;
          }
          // Display the data as graphs and/or text
          createTextData();
          if (plotlyAvailable)
          {
            createGraphGrid();
            for (let i = 0; i < Ngraphs; i++)
            {
              plotGraph('plot' + i.toString(), i);
            }
          }
          else
          {
            // Plotly could not be loaded
            document.getElementById('textData').style.display = 'block';
            document.getElementById('plotlyError').innerHTML = 
                '<br>Graphs cannot be displayed because the Plotly.js library'
                + ' could not be loaded.<br>Connect to the internet, or cache'
                + ' the script for offline use.<br><br>';
          }
          // Schedule the data update so the page will keep showing new data
          setTimeout(getLatestData, delay_ms);
        }
      };
      xmlhttp.open('GET', '/1', true);
      xmlhttp.responseType = 'arraybuffer';
      xmlhttp.send();
    }

    // Do a GET request for only the last value of each variable, and plot it.
    // This function runs periodically, at the same interval as data are read
    // on the MS430.
    // NOTE: if a 3 second cycle is used, most browsers will NOT call the
    // function every 3 seconds unless the browser window is "in focus" (in
    // view of the user and selected). If the window is minimized or in a
    // background tab, the delay between function calls is often greater.
    // You can get missing data by manually refreshing the page.
    function getLatestData()
    {
      var xmlhttp = new XMLHttpRequest();
      xmlhttp.onreadystatechange = function ()
      {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200)
        {
          const receivedData = new Float32Array(xmlhttp.response);
          // Only attempt data extraction if the data length is as expected:
          if (receivedData.length == Ngraphs)
          {
            for (let i = 0; i < Ngraphs; i++)
            {
              if (xValues.length == maxDataLength)
              {
                data[i].shift();
              }
              data[i].push(receivedData[i]);
            }

            if (xValues.length == maxDataLength)
            {
              xValues.shift();
            }
            xValues.push(makeTimeDateString(new Date()));

            createTextData();
            if (plotlyAvailable && viewGraphs)
            {
              for (let i = 0; i < Ngraphs; i++)
              {
                plotGraph('plot' + i.toString(), i);
              }
            }
          }
          // Reschedule this function to run again after the cycle time.
          setTimeout(getLatestData, delay_ms);
        }
      };
      xmlhttp.open('GET', '/2', true);
      xmlhttp.responseType = 'arraybuffer';
      xmlhttp.send();
    }

    // Make a "comma separated values" file containing all data and start
    // the download. This file can be opened with most spreadsheet software
    // and text editors.
    function makeCSVfile()
    {
      let csvData = '\uFEFF';  // UTF-8 byte order mark
      csvData += '"Time and Date"';
      for (let i = 0; i < Ngraphs; i++)
      {
        csvData += ',"' + titles[i] + '"';
      }
      csvData += '\r\n';
      for (let n = 0; n < xValues.length; n++)
      {
        csvData += '"' + xValues[n] + '"';
        for (let i = 0; i < Ngraphs; i++)
        {
          csvData += ',"' + data[i][n].toFixed(decimalPlaces[i]) + '"';
        }
        csvData += '\r\n';
      }
      let f = document.getElementById('CSVlink');
      URL.revokeObjectURL(f.href);
      f.href = URL.createObjectURL(new Blob([csvData],
                                   { type: 'text/csv;charset=utf-8' }));
      f.download = 'data.csv';
      f.click();
    }
  </script>
</body>
</html>
);

#endif
