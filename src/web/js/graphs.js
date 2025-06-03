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
function draw_graph(data, element_id, title, units, color, graph_height, y_axis_labels) {
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
            .style("background-color", "gray")
            .style("border", "solid")
            .style("border-width", "1px")
            .style("border-radius", "5px")
            .style("padding", "10px")
    let mouseover = function() {
        tooltip
            .style("opacity", 0.75)
            .style("visibility", "visible")
    }
    let mousemove = function() {
        let coordinates = d3.mouse(this);
        let x = Math.floor((coordinates[0] / width) * data.length);

        if (x < data.length) {
            if (typeof data[x].y == "string") {
                y_str = data[x].y;
            }
            else {
                y_str = data[x].y.toFixed(2);
            }

            tooltip
                .html("<b>" + Date(data[x].x) + ", " + y_str + " " + units + "</b>")
                .style("top", (event.pageY) + "px")
                .style("left", (event.pageX) + "px")
        }
    }
    let mouseleave = function() {
        tooltip
            .style("visibility", "hidden")
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

    // Define the gradient.
    var gradient = svg
        .append("linearGradient")
            .attr("id", "gradient_" + element_id)
            .attr("y1", height * 0.5)
            .attr("y2", height)
            .attr("x1", "0")
            .attr("x2", "0")
            .attr("gradientUnits", "userSpaceOnUse");
    gradient
        .append("stop")
            .attr("offset", "0")
            .attr("stop-color", color)
            .attr("stop-opacity", 1.0);
    gradient
        .append("stop")
            .attr("offset", "1")
            .attr("stop-color", color)
            .attr("stop-opacity", 0.5);

    // Define scales.
    var x_scale = d3.scaleTime()
        .domain([new Date(d3.min(data, d => d.x)), new Date(d3.max(data, d => d.x))])
        .range([0, width]);

    // If we were given labels then we have a non-numeric graph.
    if (y_axis_labels.length > 0) {
        var y_scale = d3.scaleBand()
            .domain(y_axis_labels)
            .range([height, 0]);
    }
    else {
        var y_scale = d3.scaleLinear()
            .domain([d3.min(data, d => d.y), d3.max(data, d => d.y)])
            .range([height, 0]);
    }

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
    var area_path = svg.append("path")
        .datum(data)
        .attr('fill', 'url(#gradient_' + element_id + ')')
        .attr("d", area);

    // Draw the initial data line.
    var line = d3.line()
        .x(d => x_scale(d.x))
        .y(d => y_scale(d.y));
    var line_path = svg.append("path")
        .datum(data)
        .attr("fill", "none")
        .attr("stroke", color)
        .attr("stroke-width", 2)
        .attr("d", line)
        .attr("id", "pointline");

    // Add the grid lines.
    var x_axis_grid = d3.axisBottom(x_scale)
        .tickFormat(d3.timeFormat("%X"));
    svg.append('g')
        .attr('class', 'x axis-grid')
        .attr('transform', 'translate(0,' + height + ')')
        .call(x_axis_grid);
    if (y_axis_labels.length == 0) { 
        let y_axis_grid = d3.axisLeft(y_scale)
            .tickSize(-width)
            .tickSizeOuter(0)
            .tickFormat('')
            .ticks(height / 100);
        svg.append('g')
            .attr('class', 'y axis-grid')
            .call(y_axis_grid);
    }

    // Add the X axis.
    let x_axis = svg.append("g")
        .attr("class", "x_axis")
        .attr("transform", "translate(0," + height + ")")
        .call(d3.axisBottom(x_scale));

    // Add the title and the X axis label.
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

    // Set the zoom and Pan features: how much you can zoom, on which part, and what to do when there is a zoom
    var zoom = d3.zoom()
        .scaleExtent([.5, 20])  // This control how much you can unzoom (x0.5) and zoom (x20)
        .extent([[0, 0], [width, height]])
        .on("zoom", rescale_chart);

    // This add an invisible rect on top of the chart area. This rect can recover pointer events: necessary to understand when the user zoom
    svg.append("rect")
        .attr("width", width)
        .attr("height", height)
        .style("fill", "none")
        .style("pointer-events", "all")
        .attr('transform', 'translate(' + margin.left + ',' + margin.top + ')')
        .call(zoom);

    // A function that updates the chart when the user zoom and thus new boundaries are available
    function rescale_chart() {

        // Recover the new scale.
        var new_x_scale = d3.event.transform.rescaleX(x_scale);
        var new_y_scale = d3.event.transform.rescaleY(y_scale);

        x_axis_grid.scale(new_x_scale);

        // Update axes with these new boundaries.
        x_axis.call(d3.axisBottom(new_x_scale));
        y_axis.call(d3.axisLeft(new_y_scale));

        area_path.attr('x', function(d) { return d3.event.transform.applyX(new_x_scale(d.x)); });
    }

    // Function to update chart.
    function update(new_data) {
        data = data.concat(new_data); // Need to do this so that tooltips work

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

    // Add the title and the X axis label.
    if (title.length > 0) {
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
        data = data.concat(new_data); // Need to do this so that tooltips work

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
