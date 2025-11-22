// -*- coding: utf-8 -*-
//
// MIT License
//
// Copyright (c) 2020-2025 Mike Simms
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

// Contains generic graphing code.

/// Encapsulates graph settings to reduce the number of things we need to pass to the draw functions.
class GraphSettings {
    constructor() {
        this.element_id = "";
        this.label = "";
        this.unit_label = "";
        this.color = "Yellow";
        this.height = 100;
        this.x_axis_label = "secs";
        this.y_axis_labels = [];
        this.multiline = false;
        this.fill = true; // Are we coloring in the area under the line?
        this.num_columns = 1;
        this.min_loaded_x = null; // We don't have data for x values less than this
        this.max_loaded_x = null; // We don't have data for x values greater than this
        this.more_data_func = null; // Call this to get more data (settings, min_x, max_x)
        this.update_func = null; // Called to append data to the graph
    }
}

/// @function graph_data_sort
function graph_data_sort(a, b) {
    if (a.x < b.x)
        return 1;
    if (a.x > b.x)
        return -1;
    return 0;
}

/// @function draw_graph
/// A function that allows the graph to be updated is returned.
function draw_graph(data, settings, column_index = 0) {
    let parent = "#charts";
    let parent_width = document.getElementById("charts").offsetWidth;

    let margin = { top: 20, right: 0, bottom: 40, left: 80 };

    let total_width = parent_width - margin.left - margin.right; // usable width
    let column_width = total_width / settings.num_columns; // total divided into columns

    let left = (column_index * column_width) + margin.left;
    let height = settings.height - margin.top - margin.bottom;

    let svg_width = column_width;
    let svg_height = height + margin.top + margin.bottom;

    // Scale the data to milliseconds.
    data = data.map(function(element) { return { 'x': element.x * 1000, 'y': element.y }; });

    let tooltip = d3.select("#charts")
        .append("div")
            .attr("id", settings.element_id + "_tooltip")
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
        let x = Math.floor((coordinates[0] / column_width) * data.length);

        if (x >= 0 && x < data.length) {
            let value = data[x].y;
            var y_str = "";

            if (typeof value == "string") {
                y_str = value;
            }
            else if (typeof value == "number") {
                y_str = value.toFixed(2);
            }

            if (y_str.length > 0) {
                tooltip
                    .html("<b>" + unix_time_to_local_string(data[x].x) + ", " + y_str + " " + settings.unit_label + "</b>")
                    .style("top", (event.pageY) + "px")
                    .style("left", (event.pageX) + "px")
            }
        }
    }
    let mouseleave = function() {
        tooltip
            .style("visibility", "hidden")
    }

    // Set up the SVG.
    var svg = d3.select(parent)
        .append("svg")
            .attr("id", settings.element_id)
            .attr("preserveAspectRatio", "xMinYMin meet")
            .attr("viewBox", "0 0 " + svg_width  + " " + svg_height)
            .attr("width", svg_width)
            .attr("height", svg_height)
            .on('mouseover', mouseover)
            .on('mousemove', mousemove)
            .on('mouseleave', mouseleave)
        .append("g")
            .attr("transform", "translate(" + left + "," + margin.top + ")");

    // Root group in plot coords (no scale yet).
    const g = svg.append("g");

    // Add a clipPath: everything out of this area won't be drawn.
    const clip_id = "plot-clip";
    svg.append("defs")
        .append("clipPath")
            .attr("id", clip_id)
            .attr("clipPathUnits", "userSpaceOnUse")
        .append("rect")
            .attr("x", 0)
            .attr("y", 0)
            .attr("width", column_width)
            .attr("height", height);

    // Create the scatter variable: where both the circles and the brush take place.
    const plot = g.append("g").attr("clip-path", `url(#${clip_id})`);

    // Define the gradient.
    if (settings.fill) {
        var gradient = g
            .append("linearGradient")
                .attr("id", "gradient_" + settings.element_id)
                .attr("y1", height * 0.5)
                .attr("y2", height)
                .attr("x1", "0")
                .attr("x2", "0")
                .attr("gradientUnits", "userSpaceOnUse");
        gradient
            .append("stop")
                .attr("offset", "0")
                .attr("stop-color", settings.color)
                .attr("stop-opacity", 1.0);
        gradient
            .append("stop")
                .attr("offset", "1")
                .attr("stop-color", settings.color)
                .attr("stop-opacity", 0.5);
    }

    // Define scales.
    settings.min_loaded_x = d3.min(data, d => d.x);
    settings.max_loaded_x = d3.max(data, d => d.x);
    var x_scale = d3.scaleLinear()
        .domain([settings.min_loaded_x, settings.max_loaded_x])
        .range([0, column_width]);

    // If we were given labels then we have a non-numeric graph.
    if (settings.y_axis_labels.length > 0) {
        var y_scale = d3.scaleBand()
            .domain(settings.y_axis_labels)
            .range([height, 0]);
    }
    else {
        var max_y = d3.max(data, d => d.y);
        if (max_y < 1.0) {
            max_y = 1.0;
        }
        var y_scale = d3.scaleLinear()
            .domain([d3.min(data, d => d.y), max_y])
            .range([height, 0]);
    }

    // Define the line.
    var line = d3.line()
        .x(d => x_scale(d.x))
        .y(d => y_scale(d.y));

    // Draw the area under the line.
    if (settings.fill) {
        var area = d3.area()
            .x(d => x_scale(d.x))
            .y0(height)
            .y1(d => y_scale(d.y));
        var area_path = plot.append("path")
            .datum(data)
            .attr('fill', 'url(#gradient_' + settings.element_id + ')')
            .attr("d", area);
    }
    else {
        var area_path = plot.append("path")
            .datum(data)
            .attr("stroke", settings.color)
            .attr("stroke-width", 3)
            .attr("d", line)
    }

    // Draw the initial data line.
    if (settings.fill) {
        var line_path = plot.append("path")
            .datum(data)
            .attr("fill", settings.color)
            .attr("stroke", settings.color)
            .attr("stroke-width", 3)
            .attr("d", line)
            .attr("id", "pointline");
    }
    else {
        var line_path = plot.append("path")
            .datum(data)
            .attr("stroke", settings.color)
            .attr("stroke-width", 3)
            .attr("d", line)
            .attr("id", "pointline");
    }

    // Add the grid lines.
    let x_axis_grid = d3.axisBottom(x_scale)
        .tickFormat(d3.timeFormat("%X"));
    svg.append('g')
        .attr('class', 'x axis-grid')
        .attr('transform', 'translate(0,' + height + ')')
        .call(x_axis_grid);
    var num_y_ticks = 5;
    if (settings.y_axis_labels.length > 0) { 
        num_y_ticks = settings.y_axis_labels.length;
    }
    let y_axis_grid = d3.axisLeft(y_scale)
        .tickSize(-column_width)
        .tickSizeOuter(0)
        .tickFormat('')
        .ticks(num_y_ticks);
    svg.append('g')
        .attr('class', 'y axis-grid')
        .call(y_axis_grid);

    // Add the X axis.
    let x_axis = svg.append("g")
        .attr("class", "x_axis")
        .attr("transform", "translate(0," + height + ")")
        .call(x_axis_grid);

    // Add the title and the X axis label.
    if (settings.label.length > 0) {
        svg.append("text")
            .attr("class", "axis")
            .attr("id", settings.element_id + "_title")
            .attr("transform", "translate(" + (column_width / 2) + "," + (height + margin.top - 4) + ")")
            .attr("dy", "1em")
            .style("text-anchor", "middle")
            .text(settings.label);
    }

    // Add the Y axis.
    let y_axis = svg.append("g")
        .attr("class", "y_axis")
        .call(d3.axisLeft(y_scale));

    // Add the Y axis label.
    if (settings.unit_label.length > 0) {
        svg.append("text")
            .attr("class", "axis")
            .attr("transform", "rotate(-90)")
            .attr("y", 0 - (left))
            .attr("x", 0 - (height / 2))
            .attr("dy", "1em")
            .style("text-anchor", "middle")
            .text(settings.unit_label);
    }

    // Set the zoom and Pan features: how much you can zoom, on which part, and what to do when there is a zoom.
    var zoom = d3.zoom()
        .scaleExtent([.5, 20])  // This controls how much you can unzoom (x0.5) and zoom (x20)
        .extent([[0, 0], [column_width, height]])
        .on("zoom", rescale_chart);

    // This add an invisible rect on top of the chart area.
    // This rect can recover pointer events: necessary to understand when the user zooms.
    svg.append("rect")
        .attr("width", column_width)
        .attr("height", height)
        .style("fill", "none")
        .style("pointer-events", "all")
        .attr('transform', 'translate(' + left + ',' + margin.top + ')')
        .call(zoom);

    // A function that updates the chart when the user zoom and thus new boundaries are available.
    function rescale_chart() {

        // Get the axis scales.
        let [min_x, max_x] = x_scale.domain();

        // Do we need more data?
        if (min_x < settings.min_loaded_x || max_x > settings.max_loaded_x) {
            if (settings.more_data_func) {
                if (min_x < settings.min_loaded_x) {
                    settings.min_loaded_x = min_x;
                }
                if (max_x > settings.max_loaded_x) {
                    settings.max_loaded_x = max_x;
                }
                settings.more_data_func(settings, settings.min_loaded_x, settings.max_loaded_x);
            }
        }
        else {
            // Rescale the axes.
            x_scale = d3.event.transform.rescaleX(x_scale);
            y_scale = d3.event.transform.rescaleY(y_scale);

            // Re-render the line using the rescaled axes.
            const zline = d3.line()
                .x(d => x_scale(d.x))
                .y(d => y_scale(d.y));
            if (settings.fill) {
                area = d3.area()
                    .x(d => x_scale(d.x))
                    .y0(height)
                    .y1(d => y_scale(d.y));
                area_path.attr("d", zline)
                    .attr('fill', 'url(#gradient_' + settings.element_id + ')')
                    .attr("d", area);
            }
            else {
                area_path.attr("d", zline);
                line_path.attr("d", zline);
            }
        }
    }

    // Function to update chart.
    function update(new_data) {

        // Scale the data to milliseconds.
        new_data = new_data.map(function(element) { return { 'x': element.x * 1000, 'y': element.y }; });

        // Concatenate. Need to do this so that tooltips work.
        data = data.concat(new_data);

        // Sort and remove duplicates.
        data.reduce((res, item) => (res.every(resItem => resItem.x != item.x) ? res.push(item) : true, res), [])
            .sort(graph_data_sort);

        // Re-scale.
        x_scale.domain([d3.min(data, d => d.x), d3.max(data, d => d.x)]);
        if (settings.y_axis_labels.length == 0) {
            y_scale.domain([d3.min(data, d => d.y), d3.max(data, d => d.y)]);
        }

        // Update the line and area.
        if (settings.fill) {
            plot.select("path")
                .datum(new_data)
                .attr("d", line)
                .attr("d", area)
                .attr("id", "pointline");
        }
        else {
            plot.select("path")
                .datum(new_data)
                .attr("d", line)
                .attr("id", "pointline");
        }

        // Update the axis scales.
        x_axis.call(d3.axisBottom(x_scale));
        y_axis.call(d3.axisLeft(y_scale));
    }
    settings.update_func = update;
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
        .attr("fill", "transparent");

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
