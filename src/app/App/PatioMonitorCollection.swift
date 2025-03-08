//
//  PatioMonitorCollection.swift
//  Created by Michael Simms on 3/4/25.
//

class PatioMonitorCollection : Collection {
	override init() {
		super.init(name: "patio_monitor")
	}
	required init(from decoder: any Decoder) throws {
		fatalError("init(from:) has not been implemented")
	}
}
