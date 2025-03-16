//
//  CollectionView.swift
//  Created by Michael Simms on 3/7/25.
//

import SwiftUI

struct CollectionView: View {
	@ObservedObject var collection: Collection
	var yLabel: String = ""
	var color: Color = Color.red

	func formatElapsedTime(numSeconds: Double) -> String {
		if self.collection.data.count > 0 {
			let elapsedSecs = numSeconds - Double(self.collection.data.first!.0)
			return StringUtils.formatAsHHMMSS(numSeconds: elapsedSecs)
		}
		return ""
	}

	func formatDouble(value: Double) -> String {
		return String(format: "%0.1f", value)
	}

	var body: some View {
		VStack(alignment: .center) {
			Text(self.collection.name)
				.bold()
			LineGraphView(points: self.collection.data, color: self.color, xFormatter: self.formatElapsedTime, yFormatter: self.formatDouble)
		}
		.padding()
	}
}
