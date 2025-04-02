//
//  ContentView.swift
//  Created by Michael Simms on 2/22/25.
//

import SwiftUI

struct ContentView: View {
	@StateObject var app = CommonApp.shared
	@State private var selectedItem = ""

	var body: some View {
		TabView(selection: $selectedItem) {
			ForEach(self.app.collections) { item in
				NavigationStack() {
					ScrollView() {
						CollectionView(collection: item)
							.padding()
					}
				}
				.tabItem {
					Text(item.displayName)
					Image(systemName: "house")
				}.tag(item.name)
			}
		}
    }
}

#Preview {
    ContentView()
}
