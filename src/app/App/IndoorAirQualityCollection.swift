//
//  IndoorAirQualityCollection.swift
//  Created by Michael Simms on 3/4/25.
//

import Foundation

class IndoorAirQualityCollection : Collection {
	override init() {
		super.init(name: "indoor_air_quality", displayName: "Indoor Air Quality")
	}
	required init(from decoder: any Decoder) throws {
		fatalError("init(from:) has not been implemented")
	}
	override func retrieve(database: Database) {
		Task {
			await database.fetchDocumentsWithTimestampAtLeast(collection_name: self.name, timestamp: 0, handler: { doc in
				if let ts = doc["ts"]?.doubleValue {
					if let temp_c = doc["temp_c"]?.doubleValue {
						self.update(data_key: "Temperature (C)", ts: UInt64(ts), value: temp_c)
					}
					if let humidity = doc["humidity"]?.doubleValue {
						self.update(data_key: "Humidity (%)", ts: UInt64(ts), value: humidity)
					}
					if let co2_ppm = doc["co2_ppm"]?.int32Value {
						self.update(data_key: "CO2 (PPM)", ts: UInt64(ts), value: Double(co2_ppm))
					}
				}
			})
		}
	}
}
