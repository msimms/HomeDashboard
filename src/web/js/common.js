// -*- coding: utf-8 -*-
//
// # MIT License
// 
// Copyright (c) 2017 Mike Simms
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// @function is_numeric
function is_numeric(num) {
    return !isNaN(num)
}

/// @function unix_time_to_local_string
function unix_time_to_local_string(unix_time) {
    let date = new Date(unix_time);
    return date.toLocaleString();
}

/// @function Sends an HTTP GET request and waits for the response.
function send_get_request_async(url, callback) {
    let xml_http = new XMLHttpRequest();
    let content_type = "application/json; charset=utf-8";

    xml_http.open("GET", url, true);
    xml_http.setRequestHeader('Content-Type', content_type);
    xml_http.onreadystatechange = function() {
        if (xml_http.readyState == XMLHttpRequest.DONE) {
            callback(xml_http.status, xml_http.responseText);
        }
    }
    xml_http.send();
}
