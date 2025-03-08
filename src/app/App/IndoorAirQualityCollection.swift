//
//  IndoorAirQualityCollection.swift
//  Created by Michael Simms on 3/4/25.
//

class IndoorAirQualityCollection : Collection {
	override init() {
		super.init(name: "indoor_air_quality")
	}
	required init(from decoder: any Decoder) throws {
		fatalError("init(from:) has not been implemented")
	}
}
