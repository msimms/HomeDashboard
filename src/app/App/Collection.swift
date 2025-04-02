//
//  Collection.swift
//  Created by Michael Simms on 3/4/25.
//

import Foundation

class Collection : Codable, Identifiable, Hashable, Equatable, ObservableObject {
	@Published var data: [String: Array<(UInt64, Double)>] = [:]

	enum CodingKeys: CodingKey {
		case name
	}

	var name: String = ""
	var displayName: String = ""

	/// Constructor
	init() {
		self.name = ""
		self.displayName = ""
	}
	init(name: String, displayName: String) {
		self.name = name
		self.displayName = displayName
	}

	/// @brief Hashable overrides
	func hash(into hasher: inout Hasher) {
		hasher.combine(self.name)
	}

	/// @brief Equatable overrides
	static func == (lhs: Collection, rhs: Collection) -> Bool {
		return lhs.name == rhs.name
	}

	/// @brief Populates the data array.
	func retrieve(database: Database) {
	}
	
	/// @brief 
	func update(data_key: String, ts: UInt64, value: Double) {
		DispatchQueue.main.async {
			if self.data[data_key] == nil {
				self.data[data_key] = []
			}
			self.data[data_key]?.append((ts, value))
		}
	}
}
