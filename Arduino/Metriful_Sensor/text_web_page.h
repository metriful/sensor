/* 
  text_web_page.h

  This file contains parts of the web page code which is used in the
  web_server example.

  Copyright 2020-2023 Metriful Ltd.
  Licensed under the MIT License - for further details see LICENSE.txt

  For code examples, datasheet and user guide, visit
  https://github.com/metriful/sensor
*/

#ifndef TEXT_WEB_PAGE_H
#define TEXT_WEB_PAGE_H

#define QUOTE(...) #__VA_ARGS__

// This is the HTTP response header. Variable = refresh time in seconds.
const char * responseHeader = "HTTP/1.1 200 OK\r\n"
                              "Content-type: text/html\r\n"
                              "Connection: close\r\n"
                              "Refresh: %u\r\n\r\n";

// This is the web page up to the start of the table data. No variables.
const char * pageStart = QUOTE(
<!DOCTYPE HTML>
<html>
    <head>
        <meta charset='UTF-8'>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Metriful Sensor Demo</title>
        <style>
            h1 {font-size: 1.5rem;}
            h2 {font-size: 1rem; margin-top: 2rem;}
            a {padding: 0.5rem; font-size: 1rem; display:block;}
            table, th, td {font-size: 1rem;}
            table {margin-left:auto; margin-right:auto;}
            body {padding: 0 1rem; font-family: Verdana, sans-serif;
                  background-color:#ededed; text-align:center;}
            th, td {padding: 0.025rem 0.5rem; text-align: left;}
            .v1 {text-align: right; width: 5rem;}
            .v2 {text-align: right; width: 6.5rem;}
            .v3 {text-align: right; width: 5rem;}
            .v4 {text-align: right; width: 5rem;}
            .v5 {text-align: right; width: 5.5rem;}
        </style>
    </head>
    <body>
        <h1>Indoor Environment Data</h1>
);

// Start of a data table. Variable = title.
const char * tableStart = QUOTE(   
    <p>
    <h2>%s</h2>
    <table>
);

// A table row.
// Variables = data name, class number, value, unit
const char * tableRow = QUOTE(   
    <tr>
        <td>%s</td>
        <td class='v%u'>%s</td>
        <td>%s</td>
    </tr>
);

// End of a data table. No variables.
const char * tableEnd = QUOTE(   
    </table>
    </p>
);

// End of the web page. No variables.
const char * pageEnd = QUOTE(
    <p style="margin-top: 2rem;">
        <a href="https://sensor.metriful.com">sensor.metriful.com</a>
    </p>
    </body>
</html>
);

#endif
