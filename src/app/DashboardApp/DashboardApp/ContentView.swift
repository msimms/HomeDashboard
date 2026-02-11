//
//  ContentView.swift
//  Created by Michael Simms on 2/7/26.
//

import SwiftUI

struct ContentView: View {
	@ObservedObject var serverVM: ServerVM = ServerVM.shared

	var body: some View {
		VStack(alignment: .center) {

			// Indoor Air Quality
			Image(systemName: "house")
				.imageScale(.large)
				.foregroundStyle(.tint)
			VStack() {
				HStack() {
					if let value = self.serverVM.indoorCo2ppm {
						Text(String(format: "CO\u{00B2} %u ppm", value))
					}
				}
				HStack() {
					if let value = self.serverVM.indoorTempC {
						Text(String(format: "Temperature %.2f \u{00B0}C", value))
					}
				}
				HStack() {
					if let value = self.serverVM.indoorHumidity {
						Text(String(format: "Humidity %.2f %%", value))
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
						Text(String(format: "Temperature %.2f \u{00B0}C", value))
					}
				}
				HStack() {
					if let value = self.serverVM.outdoorHumidity {
						Text(String(format: "Humidity %.2f %%", value))
					}
				}
			}
			.padding(10)
		}
        .padding()
		.opacity(0.8)
		.background(
			Image("Image")
				.resizable()
				.edgesIgnoringSafeArea(.all)
				.scaledToFit()
				.opacity(0.7)
				.onReceive(NotificationCenter.default.publisher(for: UIDevice.orientationDidChangeNotification)) { _ in
				}
		)
    }
}

#Preview {
    ContentView()
}
