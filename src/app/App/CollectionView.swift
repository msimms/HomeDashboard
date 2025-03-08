//
//  CollectionView.swift
//  Created by Michael Simms on 3/7/25.
//

import SwiftUI

struct CollectionView: View {
	@ObservedObject var collection: Collection
	var title: String = ""
	var yLabel: String = ""
	var data: Array<(UInt64, Double)> = []
	var color: Color = Color.red
	var formatter: ((_ num: Double) -> String)?

	func formatElapsedTime(numSeconds: Double) -> String {
		if self.data.count > 0 {
			let elapsedSecs = numSeconds - Double(self.data.first!.0)
			return StringUtils.formatAsHHMMSS(numSeconds: elapsedSecs)
		}
		return ""
	}

	var body: some View {
		VStack(alignment: .center) {
			LineGraphView(points: self.data, color: self.color, xFormatter: self.formatElapsedTime, yFormatter: self.formatter)
		}
		.padding()
	}
}
