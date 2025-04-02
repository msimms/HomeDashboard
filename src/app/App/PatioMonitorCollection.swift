//
//  PatioMonitorCollection.swift
//  Created by Michael Simms on 3/4/25.
//

import Foundation

class PatioMonitorCollection : Collection {
	override init() {
		super.init(name: "patio_monitor", displayName: "Patio Monitor")
	}
	required init(from decoder: any Decoder) throws {
		fatalError("init(from:) has not been implemented")
	}
	override func retrieve(database: Database) {
		Task {
			await database.fetchDocumentsWithTimestampAtLeast(collection_name: self.name, timestamp: 0, handler: { doc in
				if let ts = doc["ts"]?.doubleValue {
					if let wind_speed_ms = doc["wind speed ms"]?.doubleValue {
						self.update(data_key: "Wind Speed (m/s)", ts: UInt64(ts), value: wind_speed_ms)
					}
					if let temperature = doc["temperature"]?.doubleValue {
						self.update(data_key: "Temperature (C)", ts: UInt64(ts), value: temperature)
					}
					if let humidity = doc["humidity"]?.doubleValue {
						self.update(data_key: "Humidity (%)", ts: UInt64(ts), value: humidity)
					}
					if let moisture_sensor_1 = doc["moisture_sensor_1"]?.doubleValue {
						self.update(data_key: "Moisture Sensor 1", ts: UInt64(ts), value: moisture_sensor_1)
					}
					if let moisture_sensor_2 = doc["moisture_sensor_2"]?.doubleValue {
						self.update(data_key: "Moisture Sensor 2", ts: UInt64(ts), value: moisture_sensor_2)
					}
				}
			})
		}
	}
}
