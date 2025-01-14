// -*- coding: utf-8 -*-
//
// MIT License
//
// Copyright (c) 2020-2021 Mike Simms
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

/// @function draw_graph
/// A function that allows the graph to be updated is returned.
function draw_graph(data, element_id, title, units, color, graph_height) {
    let parent = "#charts";
    let parent_width = document.getElementById("charts").offsetWidth;

    let margin = { top: 20, right: 20, bottom: 40, left: 50 },
        width = parent_width - margin.left - margin.right,
        height = graph_height - margin.top - margin.bottom;
    
    let svg_width = width + margin.left + margin.right;
    let svg_height = height + margin.top + margin.bottom;

    let tooltip = d3.select("#charts")
        .append("div")
            .attr("id", element_id + "_tooltip")
            .style("opacity", 0)
            .style("position", "absolute")
            .style("visibility", "hidden")
            .style("z-index", 1)
            .style("cursor", "pointer")
    let mouseover = function() {
        tooltip
            .style("opacity", 0.7)
            .style("visibility", "visible")
    }
    let mousemove = function() {
        let coordinates = d3.mouse(this);
        let x = Math.floor((coordinates[0] / width) * data.length);

        if (x < data.length) {
            tooltip
                .html("<b>" + data[x].x + " secs = " + data[x].y.toFixed(2) + " " + units + "</b>")
                .style("top", (event.pageY) + "px")
                .style("left", (event.pageX) + "px")
        }
    }
    let mouseleave = function() {
    }

    // Set up the SVG.
    var svg = d3.select(parent)
        .append("svg")
            .attr("id", element_id)
            .attr("preserveAspectRatio", "xMinYMin meet")
            .attr("viewBox", "0 0 " + svg_width  + " " + svg_height)
            .attr("width", svg_width)
            .attr("height", svg_height)
            .on('mouseover', mousemove)
            .on('mousemove', mouseover)
            .on('mouseleave', mouseleave)
        .append("g")
            .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    // Define scales.
    var x_scale = d3.scaleLinear()
        .domain([d3.min(data, d => d.x), d3.max(data, d => d.x)])
        .range([0, width]);
    var y_scale = d3.scaleLinear()
        .domain([d3.min(data, d => d.y), d3.max(data, d => d.y)])
        .range([height, 0]);

    // Fill the background.
    svg.append("rect")
        .attr("width", width)
        .attr("height", height)
        .attr("fill", "transparent");

    // Draw the area under the line.
    var area = d3.area()
        .x(d => x_scale(d.x))
        .y0(height)
        .y1(d => y_scale(d.y));
    svg.append("path")
        .datum(data)
        .attr("fill", color)
        .attr("d", area);

    // Draw the initial data line.
    var line = d3.line()
        .x(d => x_scale(d.x))
        .y(d => y_scale(d.y));
    svg.append("path")
        .datum(data)
        .attr("fill", "none")
        .attr("stroke", color)
        .attr("stroke-width", 4)
        .attr("d", line);

    // Add the grid lines.
    let x_axis_grid = d3.axisBottom(x_scale)
        .tickSize(-height)
        .tickSizeOuter(0)
        .tickFormat('')
        .ticks(width / 50);
    svg.append('g')
        .attr('class', 'x axis-grid')
        .attr('transform', 'translate(0,' + height + ')')
        .call(x_axis_grid);

    // Add the X axis.
    let x_axis = svg.append("g")
        .attr("class", "x_axis")
        .attr("transform", "translate(0," + height + ")")
        .call(d3.axisBottom(x_scale));

    // Add the title.
    if (title.length > 0) {
        svg.append("text")
            .attr("class", "axis")
            .attr("id", element_id + "_title")
            .attr("transform", "translate(" + (width / 2) + "," + (height + margin.top - 4) + ")")
            .attr("dy", "1em")
            .style("text-anchor", "middle")
            .text(title);
    }

    // Add the Y axis.
    let y_axis = svg.append("g")
        .attr("class", "y_axis")
        .call(d3.axisLeft(y_scale));

    // Add the Y axis label.
    if (units.length > 0) {
        svg.append("text")
            .attr("class", "axis")
            .attr("transform", "rotate(-90)")
            .attr("y", 0 - (margin.left))
            .attr("x", 0 - (height / 2))
            .attr("dy", "1em")
            .style("text-anchor", "middle")
            .text(units);
    }

    // Function to update chart.
    function update(new_data) {
        x_scale.domain([0, d3.max(new_data, d => d.x)]);
        y_scale.domain([d3.min(new_data, d => d.y), d3.max(new_data, d => d.y)]);

        // Update the line and area.
        svg.select("path")
            .datum(new_data)
            .attr("d", line)
            .attr("d", area);

        // Update the axis scales.
        svg.select(".y_axis")
            .call(d3.axisLeft(y_scale));
        svg.select(".x_axis")
            .call(d3.axisBottom(x_scale));
    }
    return update;
}

/// @function draw_bar_chart
function draw_bar_chart(data, title, units, color, graph_height) {
    let parent = "#charts";
    let parent_width = document.getElementById("charts").offsetWidth;

    // Set the dimensions and margins of the graph.
    let margin = { top: 20, right: 20, bottom: 40, left: 50 },
        width = parent_width - margin.left - margin.right,
        height = graph_height - margin.top - margin.bottom;

    // Set up the SVG.
    var svg = d3.select(parent)
        .append("svg")
            .attr("id", title)
            .attr("width", width + margin.left + margin.right)
            .attr("height", height + margin.top + margin.bottom)
        .append("g")
            .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    // Fill the background.
    svg.append("rect")
        .attr("width", width)
        .attr("height", height)
        .attr("fill", "Gainsboro");

    // Add the X axis.
    var x_axis = d3.scaleBand()
        .range([ 0, width ])
        .domain(d3.range(0, data.length)).range([0, width])
        .padding(0.2);
    svg.append("g")
        .attr("class", "x_axis")
        .attr("transform", "translate(0," + height + ")")
        .call(d3.axisBottom(x_axis))
        .selectAll("text")
            .style("text-anchor", "end");

    // Add the title.
    if (title.length > 0) {
        // Add the X axis label.
        svg.append("text")
            .attr("class", "axis")
            .attr("transform", "translate(" + (width / 2) + "," + (height + margin.top - 4) + ")")
            .attr("dy", "1em")
            .style("text-anchor", "middle")
            .text(title);
    }
    
    // Add the Y axis.
    var y_axis = d3.scaleLinear()
        .domain([0, d3.max(data, d => d.y)])
        .range([ height, 0]);
    svg.append("g")
        .attr("class", "y_axis")
        .call(d3.axisLeft(y_axis));

    // Add the Y axis label.
    if (units.length > 0) {
        svg.append("text")
            .attr("class", "axis")
            .attr("transform", "rotate(-90)")
            .attr("y", 0 - (margin.left))
            .attr("x", 0 - (height / 2))
            .attr("dy", "1em")
            .style("text-anchor", "middle")
            .text(units);
    }

    // Bars
    svg.selectAll("bar")
        .attr("class", "bar")
        .data(data)
        .enter()
        .append("rect")
            .attr("x", function(d, i) { return x_axis(i); })
            .attr("y", function(d) { return y_axis(d.y); })
            .attr("width", x_axis.bandwidth())
            .attr("height", function(d) { return height - y_axis(d.y); })
            .attr("fill", color);            

    // Function to update chart.
    function update(new_data) {
        x_axis.domain(d3.range(0, new_data.length)).range([0, width]);
        y_axis.domain([0, d3.max(new_data, d => d.y)]);

        // Update the bar.
        svg.select("bar")
            .datum(new_data);

        // Update the axis scales.
        svg.select(".y_axis")
            .call(d3.axisLeft(x_axis));
        svg.select(".x_axis")
            .call(d3.axisBottom(y_axis));
    }
    return update;
}
