//
//  ContentView.swift
//  Created by Michael Simms on 2/7/26.
//

import SwiftUI

struct ContentView: View {
	@ObservedObject var serverVM: ServerVM = ServerVM.shared

	var body: some View {
        VStack {

			// Indoor Air Quality
			Image(systemName: "house")
				.imageScale(.large)
				.foregroundStyle(.tint)
			VStack() {
				HStack() {
					if let value = self.serverVM.indoorCo2ppm {
						Text("CO\u{00B2}")
						Text(String(format: "%u", value))
						Text("ppm")
					}
				}
				HStack() {
					if let value = self.serverVM.indoorTempC {
						Text("Temperature")
						Text(String(format: "%.2f", value))
						Text("\u{00B0}C")
					}
				}
				HStack() {
					if let value = self.serverVM.indoorHumidity {
						Text("Humidity")
						Text(String(format: "%.2f", value))
						Text("%")
					}
				}
			}
			.padding(10)

			// Patio
			Image(systemName: "tree")
				.imageScale(.large)
				.foregroundStyle(.tint)
			VStack() {
				HStack() {
					if let value = self.serverVM.outdoorTempC {
						Text("Temperature")
						Text(String(format: "%.2f", value))
						Text("\u{00B0}C")
					}
				}
				HStack() {
					if let value = self.serverVM.outdoorHumidityC {
						Text("Humidity")
						Text(String(format: "%.2f", value))
						Text("%")
					}
				}
			}
			.padding(10)
        }
        .padding()
    }
}

#Preview {
    ContentView()
}
