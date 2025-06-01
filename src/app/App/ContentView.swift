//
//  ContentView.swift
//  Created by Michael Simms on 2/22/25.
//

import SwiftUI

struct ContentView: View {
	@StateObject var app = CommonApp.shared
	@State private var selectedItem = ""
	@State private var addedSummaryView = false

	var body: some View {
		TabView(selection: $selectedItem) {
			
			// Add the Summary View.
			NavigationStack() {
				ScrollView() {
					SummaryView()
						.padding()
				}
			}
			.tabItem {
				Text("Summary")
				Image(systemName: "clipboard")
			}.tag("Summary")

			// Add the view for each collection.
			ForEach(self.app.collections) { collection in
				NavigationStack() {
					ScrollView() {
						CollectionView(collection: collection)
							.padding()
					}
				}
				.tabItem {
					Text(collection.displayName)
					Image(systemName: collection.symbolName)
				}.tag(collection.name)
			}

			// Add the Alerts View.
			NavigationStack() {
				ScrollView() {
					AlertsView()
						.padding()
				}
			}
			.tabItem {
				Text("Summary")
				Image(systemName: "clipboard")
			}.tag("Summary")
		}
    }
}

#Preview {
    ContentView()
}
