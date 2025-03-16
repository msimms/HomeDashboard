//
//  ContentView.swift
//  Created by Michael Simms on 2/22/25.
//

import SwiftUI

struct ContentView: View {
	@StateObject var app = CommonApp.shared
	
	var body: some View {
		VStack(alignment: .center) {
			ForEach(app.collections) { item in
				CollectionView(collection: item)
			}
		}
        .padding()
    }
}

#Preview {
    ContentView()
}
