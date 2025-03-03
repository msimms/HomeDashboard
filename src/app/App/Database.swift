//
//  Database.swift
//  Created by Michael Simms on 2/22/25.
//

import Foundation
import MongoSwift
import NIO

class Database {
	private var client: MongoClient?
	private var database: MongoDatabase?
	
	func connect(url: String) async {
		do {
			let eventLoopGroup = MultiThreadedEventLoopGroup(numberOfThreads: 1)
			self.client = try await MongoClient(url, using: eventLoopGroup)

			// Select the database and collection
			self.database = self.client?.db("statusdb")
		} catch {
			print("Failed to connect to MongoDB: \(error)")
		}
	}

	func listCollections() async {
		do {
			if let db = self.database {
				let collections = try await db.listCollectionNames()
			}
		}
		catch {
			NSLog(error.localizedDescription)
		}
	}
}
