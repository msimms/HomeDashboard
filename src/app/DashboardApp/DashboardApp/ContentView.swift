//
//  ContentView.swift
//  Created by Michael Simms on 2/7/26.
//

import SwiftUI

struct ContentView: View {
	@ObservedObject var serverVM: ServerVM = ServerVM.shared

	var body: some View {
		VStack(alignment: .center) {

			HStack() {
				Spacer()
			}

			// Indoor Air Quality
			Image(systemName: "clock")
				.imageScale(.large)
				.foregroundStyle(.tint)
			VStack() {
				if let value = self.serverVM.ts {
					Text(value)
						.modifier(CommonStyle())
				}
			}

			// Indoor Air Quality
			Image(systemName: "house")
				.imageScale(.large)
				.foregroundStyle(.tint)
			VStack() {
				if let value = self.serverVM.indoorCo2ppm {
					Text(String(format: "CO\u{00B2} %u ppm", value))
						.modifier(CommonStyle())
				}
				if let value = self.serverVM.indoorTempC {
					Text(String(format: "Temperature %.2f \u{00B0}C (%.2f \u{00B0}F)", value, value * 1.8 + 32.0))
						.modifier(CommonStyle())
				}
				if let value = self.serverVM.indoorHumidity {
					Text(String(format: "Humidity %.2f %%", value))
						.modifier(CommonStyle())
				}
			}
			.padding(10)

			// Patio
			Image(systemName: "tree")
				.imageScale(.large)
				.foregroundStyle(.tint)
			VStack() {
				if let value = self.serverVM.outdoorTempC {
					Text(String(format: "Temperature %.2f \u{00B0}C", value))
						.modifier(CommonStyle())
				}
				if let value = self.serverVM.outdoorHumidity {
					Text(String(format: "Humidity %.2f %%", value))
						.modifier(CommonStyle())
				}
				if let value = self.serverVM.windSpeedMs {
					Text(String(format: "Wind Speed %.2f m/s", value))
						.modifier(CommonStyle())
				}
				if let value = self.serverVM.moistureSensor1 {
					Text(String(format: "Moisture Sensor #1 %.2f", value))
						.modifier(CommonStyle())
				}
				if let value = self.serverVM.moistureSensor2 {
					Text(String(format: "Moisture Sensor #2 %.2f", value))
						.modifier(CommonStyle())
				}
			}
			.padding(10)
		}
        .padding()
		.opacity(0.8)
		.frame(
			minWidth: 0,
			maxWidth: .infinity,
			minHeight: 0,
			maxHeight: .infinity,
			alignment: .center
		)
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
