//
//  ChartView.swift
//  Created by Michael Simms on 8/17/26.
//

import SwiftUI
import Charts

struct ChartView: View {
	var times: Array<Double>
	var values: Array<Double>

	var body: some View {
		Chart {
			ForEach(0..<min(times.count, values.count), id: \.self) { i in
				LineMark(
					x: .value("Time", times[i]),
					y: .value("Value", values[i])
				)
			}
		}
		.chartXAxisLabel("Time", position: .bottom, alignment: .center)
		.chartYAxisLabel("Volts", position: .leading, alignment: .center)
		.chartXScale(domain: .automatic(includesZero: false))
		.chartYScale(domain: .automatic(includesZero: false))
		.padding()
		.gesture(
			MagnifyGesture()
				.onChanged { value in
					// Update domain based on value.magnification
				}
		)
	}
}

