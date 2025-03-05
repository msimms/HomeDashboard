//
//  CommonApp.swift
//  Created by Michael Simms on 3/1/25.
//

import Foundation
#if os(watchOS)
import SwiftUI // for WKInterfaceDevice
#endif

let DATABASE_URL = ""
let DATABASE_NAME = "statusdb"

class CommonApp : ObservableObject {
	static let shared = CommonApp()
	private var database = Database()
	
	/// Singleton constructor
	private init() {
		Task {
			// Connect to the database.
			await self.database.connect(url: DATABASE_URL, db_name: DATABASE_NAME)

			// List all collections and instantiate any modules we have that match the collection name.
			let collections = await self.database.listCollections()
			for collection in collections {
				
			}
		}
	}
}
