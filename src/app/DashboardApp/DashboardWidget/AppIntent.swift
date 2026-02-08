//
//  AppIntent.swift
//  Created by Michael Simms on 2/7/26.
//

import WidgetKit
import AppIntents

struct ConfigurationAppIntent: WidgetConfigurationIntent {
    static var title: LocalizedStringResource { "Configuration" }
    static var description: IntentDescription { "This is an example widget." }

    // An example configurable parameter.
	@Parameter(title: "Attribute", default: "")
	var attribute: String
	@Parameter(title: "Value", default: 0)
	var value: Int
	@Parameter(title: "Units", default: "")
	var units: String
}
