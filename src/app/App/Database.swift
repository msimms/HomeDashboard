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

	/// Attempts to connect to the database with the given URL and select the database with the given name.
	func connect(url: String, db_name: String) async {
		do {
			let eventLoopGroup = MultiThreadedEventLoopGroup(numberOfThreads: 1)

			// Connect to the database.
			self.client = try MongoClient(url, using: eventLoopGroup)
			if let client = self.client {

				// Select the database.
				self.database = client.db(db_name)
			}
		} catch {
			print("Failed to connect to MongoDB: \(error)")
		}
	}

	func listCollections() async -> [String] {
		do {
			if let db = self.database {
				let collections = try await db.listCollectionNames()
				return collections
			}
		}
		catch {
			NSLog(error.localizedDescription)
		}
		return []
	}
	
	func fetchDocument(collection: MongoCollection<BSONDocument>, query: BSONDocument) async {
		do {
			if let foundDocument = try await collection.findOne(query) {
				print("Found document: \(foundDocument)")
			} else {
				print("No document found")
			}
		} catch {
			print("Error finding document: \(error)")
		}
	}
}
