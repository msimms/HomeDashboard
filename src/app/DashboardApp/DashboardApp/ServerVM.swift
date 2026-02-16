//
//  ServerVM.swift
//  Created by Michael Simms on 2/8/26.
//

import Foundation
import Combine

class ServerVM : ObservableObject {
	static let shared = ServerVM()

	@Published var ts: String?
	@Published var indoorCo2ppm: Int?
	@Published var indoorTempC: Float?
	@Published var indoorHumidity: Float?
	@Published var outdoorTempC: Float?
	@Published var outdoorHumidity: Float?
	@Published var moistureSensor1: Float?
	@Published var moistureSensor2: Float?

	private init() {
		self.update()
	}

	func fetchStatus() {
		Task { @MainActor in
			do {
				let item = try await APIClient.fetchIndoorStatus()

				let isoFormatter = ISO8601DateFormatter()
				isoFormatter.timeZone = TimeZone.current // Use local time
				self.ts = isoFormatter.string(from: Date(timeIntervalSince1970: TimeInterval(item.ts)))

				// Update published properties with decoded values
				self.indoorCo2ppm = item.co2_ppm
				self.indoorTempC = item.temp_c
				self.indoorHumidity = item.humidity

			} catch {
				// Handle error as needed, e.g., log or update state
			}
		}

		Task { @MainActor in
			do {
				let item = try await APIClient.fetchPatioStatus()

				// Update published properties with decoded values
				self.outdoorTempC = item.temp_c
				self.outdoorHumidity = item.humidity
				self.moistureSensor1 = item.moisture_sensor_1
				self.moistureSensor2 = item.moisture_sensor_2
			} catch {
			}
		}
	}

	func update() {
		self.fetchStatus()

		Timer.scheduledTimer(withTimeInterval: 600, repeats: true) { _ in
			self.fetchStatus()
		}
	}
}
