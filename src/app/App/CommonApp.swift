//
//  CommonApp.swift
//  Created by Michael Simms on 3/1/25.
//

import Foundation
#if os(watchOS)
import SwiftUI // for WKInterfaceDevice
#endif

class CommonApp : ObservableObject {
	static let shared = CommonApp()
	private var database = Database()
	
	/// Singleton constructor
	private init() {
		Task {
			await self.database.connect(url: "")
		}
	}
}
