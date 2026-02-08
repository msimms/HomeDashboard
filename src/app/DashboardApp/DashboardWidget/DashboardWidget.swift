//
//  DashboardWidget.swift
//  Created by Michael Simms on 2/7/26.
//

import WidgetKit
import SwiftUI

struct Provider: AppIntentTimelineProvider {
	func placeholder(in context: Context) -> StatusEntry {
		StatusEntry(date: Date(), configuration: ConfigurationAppIntent())
	}

	func snapshot(for configuration: ConfigurationAppIntent, in context: Context) async -> StatusEntry {
		StatusEntry(date: Date(), configuration: configuration)
	}

	func timeline(for configuration: ConfigurationAppIntent, in context: Context) async -> Timeline<StatusEntry> {
		var entries: [StatusEntry] = []

		// Generate a timeline consisting of five entries an hour apart, starting from the current date.
		let currentDate = Date()
		for hourOffset in 0 ..< 5 {
			let entryDate = Calendar.current.date(byAdding: .hour, value: hourOffset, to: currentDate)!
			let entry = StatusEntry(date: entryDate, configuration: configuration)
			entries.append(entry)
		}

		return Timeline(entries: entries, policy: .atEnd)
	}

//    func relevances() async -> WidgetRelevances<ConfigurationAppIntent> {
//        // Generate a list containing the contexts this widget is relevant in.
//    }
}

struct StatusEntry: TimelineEntry {
	let date: Date
	let configuration: ConfigurationAppIntent
}

struct DashboardWidgetEntryView : View {
    var entry: Provider.Entry

    var body: some View {
		VStack {
			Text(self.entry.date, style: .time)
			HStack {
				Text(self.entry.configuration.attribute)
				Text(String(self.entry.configuration.value))
				Text(self.entry.configuration.units)
			}
		}
	}
}

struct DashboardWidget: Widget {
    let kind: String = "DashboardWidget"

    var body: some WidgetConfiguration {
		AppIntentConfiguration(kind: kind, intent: ConfigurationAppIntent.self, provider: Provider()) { entry in
			DashboardWidgetEntryView(entry: entry)
				.containerBackground(.fill.tertiary, for: .widget)
		}
		.supportedFamilies([.accessoryInline, .accessoryCircular, .accessoryRectangular])
	}
}

extension ConfigurationAppIntent {
	fileprivate static var smiley: ConfigurationAppIntent {
		let intent = ConfigurationAppIntent()
		intent.attribute = "CO2"
		intent.units = "ppm"
		return intent
	}

	fileprivate static var starEyes: ConfigurationAppIntent {
		let intent = ConfigurationAppIntent()
		intent.attribute = "Temp"
		intent.units = "C"
		return intent
	}
}

#Preview(as: .systemSmall) {
	DashboardWidget()
} timeline: {
	StatusEntry(date: .now, configuration: .smiley)
	StatusEntry(date: .now, configuration: .starEyes)
}
