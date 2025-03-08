//
//  CollectionFactory.swift
//  Created by Michael Simms on 3/5/25.
//

enum CollectionFactoryError: Error {
	case runtimeError(String)
}

class CollectionFactory {

	/// Constructor
	init() {
	}

	func createHandler(name: String) throws -> Collection {
		if name.caseInsensitiveCompare("indoor_air_quality") == .orderedSame {
			return IndoorAirQualityCollection()
		}
		if name.caseInsensitiveCompare("patio_monitor") == .orderedSame {
			return PatioMonitorCollection()
		}
		throw CollectionFactoryError.runtimeError("Collection not found!")
	}
}
