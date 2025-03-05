//
//  ContentView.swift
//  Created by Michael Simms on 2/22/25.
//

import SwiftUI

struct ContentView: View {
	private var common = CommonApp.shared

	var body: some View {
        VStack {
            Image(systemName: "globe")
                .imageScale(.large)
                .foregroundStyle(.tint)
            Text("Hello, world!")
        }
        .padding()
    }
}

#Preview {
    ContentView()
}
