//
//  ServerVM.swift
//  Created by Michael Simms on 2/8/26.
//

import Foundation
import Combine

class ServerVM : ObservableObject {
	static let shared = ServerVM()

	@Published var indoorCo2ppm: Int?
	@Published var indoorTempC: Float?
	@Published var indoorHumidity: Float?
	@Published var outdoorTempC: Float?
	@Published var outdoorHumidityC: Float?

	private init() {
		self.update()
	}

	func fetchStatus() {
		Task { @MainActor in
			do {
				let item = try await APIClient.fetchIndoorStatus()

				// Update published properties with decoded values
				self.indoorCo2ppm = item.co2_ppm
				self.indoorTempC = item.temp_c
				self.indoorHumidity = item.humidity

			} catch {
				// Handle error as needed, e.g., log or update state
			}
		}
	}

	func update() {
		self.fetchStatus()

		Timer.scheduledTimer(withTimeInterval: 60, repeats: true) { _ in
			self.fetchStatus()
		}
	}
}
