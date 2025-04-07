//
//  SummartView.swift
//  Created by Michael Simms on 4/4/25.
//

import SwiftUI

struct CollectionSummaryView: View {
	let collection: Collection
	
	var body: some View {
		VStack() {
			Text(collection.displayName)
				.bold()
			ForEach(self.collection.data.sorted(by: { $0.key < $1.key }), id: \.key) { key, value in
				if value.isEmpty {
					Text("\(key): \"No data")
				}
				else {
					Text("\(key): \(value.last?.1 ?? 0)")
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
