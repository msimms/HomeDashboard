//
//  DashboardWidget.swift
//  Created by Michael Simms on 2/7/26.
//

import WidgetKit
import SwiftUI

struct Provider: AppIntentTimelineProvider {

	// Static preview data.
	func placeholder(in context: Context) -> StatusEntry {
		let configuration = ConfigurationAppIntent()
		configuration.attribute1 = "Temp 25 C"
		return StatusEntry(date: Date(), configuration: configuration)
	}

	// Quick snapshot for the gallery.
	func snapshot(for configuration: ConfigurationAppIntent, in context: Context) async -> StatusEntry {
		StatusEntry(date: Date(), configuration: configuration)
	}

	// Timeline with entries and a refresh policy.
	func timeline(for configuration: ConfigurationAppIntent, in context: Context) async -> Timeline<StatusEntry> {
		var entries: [StatusEntry] = []

		do {
			let item = try await APIClient.fetchIndoorStatus()
			let currentDate = Date(timeIntervalSince1970: TimeInterval(item.ts))
			let entryDate = currentDate

			let intent = ConfigurationAppIntent()
			intent.attribute1 = String(format: "CO\u{00B2} %u ppm", item.co2_ppm)
			intent.attribute2 = String(format: "%.2f \u{00B0}C (%.2f \u{00B0}F)", item.temp_c, item.temp_c * 1.8 + 32.0)

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
			HStack {
				Image(systemName: "house")
				Text(self.entry.date, style: .time)
			}
			VStack {
				Text(self.entry.configuration.attribute1)
				Text(self.entry.configuration.attribute2)
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
		intent.attribute1 = String(format: "CO\u{00B2} 800 ppm")
		return intent
	}
}

#Preview(as: .systemSmall) {
	DashboardWidget()
} timeline: {
	StatusEntry(date: .now, configuration: .co2)
}

