//
//  SummartView.swift
//  Created by Michael Simms on 4/4/25.
//

import SwiftUI

struct CollectionSummaryView: View {
	@StateObject var collection: Collection
	
	var body: some View {
		VStack() {
			Text(self.collection.displayName)
				.font(.system(size: 24))
				.bold()
			ForEach(self.collection.data.sorted(by: { $0.key < $1.key }), id: \.key) { key, value in
				if value.isEmpty {
					Text("\(key): \"No data")
				}
				else {
					let last = value.last!
					let value: Double = last.1
					let ts: UInt64 = last.0
					let ts_str: String = StringUtils.formatDate(ts: Double(ts))
					HStack() {
						Text("\(key): ")
							.bold()
						Text("\(value, specifier: "%.2f")")
						Text("Updated on \(ts_str)")
							.foregroundColor(.gray)
					}
				}
			}
		}
	}
}

struct SummaryView: View {
	@StateObject var app = CommonApp.shared

	var body: some View {
		ScrollView() {
			ForEach(self.app.collections) { collection in
				CollectionSummaryView(collection: collection)
					.padding()
			}
		}
    }
}

#Preview {
    SummaryView()
}
