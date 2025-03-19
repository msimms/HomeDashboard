//
//  CollectionView.swift
//  Created by Michael Simms on 3/7/25.
//

import SwiftUI

struct CollectionView: View {
	@ObservedObject var collection: Collection
	var yLabel: String = ""
	var color: Color = Color.red

	var body: some View {
		VStack(alignment: .center) {
			Text(self.collection.name)
				.bold()
			ForEach(self.collection.data.sorted(by: { $0.key < $1.key }), id: \.key) { key, value in
				Section(header: Text(key)) {
					LineGraphView(points: value, color: self.color, xFormatter: StringUtils.formatDate, yFormatter: StringUtils.formatDouble)
				}
				.frame(height: 256)
			}
		}
		.padding()
	}
}
