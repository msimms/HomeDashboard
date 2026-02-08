//
//  DashboardWidget.swift
//  Created by Michael Simms on 2/7/26.
//

import WidgetKit
import SwiftUI

struct Provider: AppIntentTimelineProvider {

	// Static preview data.
	func placeholder(in context: Context) -> StatusEntry {
		StatusEntry(date: Date(), configuration: ConfigurationAppIntent())
	}

	// Quick snapshot for the gallery.
	func snapshot(for configuration: ConfigurationAppIntent, in context: Context) async -> StatusEntry {
		StatusEntry(date: Date(), configuration: configuration)
	}

	// Timeline with entries and a refresh policy.
	func timeline(for configuration: ConfigurationAppIntent, in context: Context) async -> Timeline<StatusEntry> {
		var entries: [StatusEntry] = []

		// Generate a timeline consisting of five entries an hour apart,
		// starting from the current date.
		do {
			let item = try await APIClient.fetchIndoorStatus()
			let intent = ConfigurationAppIntent()
			intent.attribute = "CO2"
			intent.value = item.co2_ppm;
			intent.units = "ppm"

			let currentDate = Date()
			let entryDate = Calendar.current.date(byAdding: .hour, value: 0, to: currentDate)!
			let entry = StatusEntry(date: entryDate, configuration: intent)
			entries.append(entry)
		}
		catch {
		}

		return Timeline(entries: entries, policy: .atEnd)
	}
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
	fileprivate static var co2: ConfigurationAppIntent {
		let intent = ConfigurationAppIntent()
		intent.attribute = "CO2"
		intent.value = 800;
		intent.units = "ppm"
		return intent
	}

	fileprivate static var temp: ConfigurationAppIntent {
		let intent = ConfigurationAppIntent()
		intent.attribute = "Temp"
		intent.value = 35;
		intent.units = "C"
		return intent
	}
}

#Preview(as: .systemSmall) {
	DashboardWidget()
} timeline: {
	StatusEntry(date: .now, configuration: .co2)
	StatusEntry(date: .now, configuration: .temp)
}
