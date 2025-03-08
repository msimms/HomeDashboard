//
//  ContentView.swift
//  Created by Michael Simms on 2/22/25.
//

import SwiftUI

struct ContentView: View {
	var body: some View {
		VStack(alignment: .center) {
			if CommonApp.shared.collections.count > 0 {
				List {
					ForEach(CommonApp.shared.collections) { item in
						CollectionView(collection: item)
					}
				}
			}
			else {
				Text("No Data")
			}
		}
        .padding()
    }
}

#Preview {
    ContentView()
}
