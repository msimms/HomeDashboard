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
	@Parameter(title: "", default: "")
	var attribute1: String
	@Parameter(title: "", default: "")
	var attribute2: String
}
